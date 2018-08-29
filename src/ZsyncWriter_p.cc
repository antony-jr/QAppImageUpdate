/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018, Antony jr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @filename    : ZsyncWriter_p.cc
 * @description : This is where the main zsync algorithm is implemented.
*/
#include <ZsyncWriter_p.hpp>

/*
 * Prints to the log.
 * LOGS,LOGE  -> Prints normal log messages.
 * INFO_START,INFO_END -> Prints info messages to log.
 * WARNING_START,WARNING_END -> Prints warning messages to log.
 * FATAL_START,FATAL_END -> Prints fatal messages to log.
 *
 * Example:
 * 	LOGS "This is a log message." LOGE
 *
 *
*/
#ifndef LOGGING_DISABLED
#define LOGS *(_pLogger.data()) <<
#define LOGR <<
#define LOGE ; \
	     emit(logger(_sLogBuffer , _sSourceFilePath)); \
	     _sLogBuffer.clear();
#else
#define LOGS (void)
#define LOGR ;(void)
#define LOGE ;
#endif // LOGGING_DISABLED

#define INFO_START LOGS "   INFO: " LOGR
#define INFO_END LOGE

#define WARNING_START LOGS "WARNING: " LOGR
#define WARNING_END LOGE

#define FATAL_START LOGS "  FATAL: " LOGR
#define FATAL_END LOGE

using namespace AppImageUpdaterBridge;

/*
 * The rolling checksum is very similar to Adler32 rolling checksum
 * but not the same , So please don't replace this with the Adler32.
 * Compared to Adler32 , This rolling checksum is weak but very fast.
 * The weakness is balanced by the use of a strong checksum , In this
 * case Md4. Md4 checksum length is reduced using the zsync algorithm
 * mentioned in the technical paper , This is solely done for performance.
*/

/* Update a already calculated block , This is why a rolling checksum is needed. */
#define UPDATE_RSUM(a, b, oldc, newc, bshift) do { (a) += ((unsigned char)(newc)) - ((unsigned char)(oldc)); (b) += (a) - ((oldc) << (bshift)); } while (0)

/* Calculate the rsum for a single block of data. */
static rsum __attribute__ ((pure)) calc_rsum_block(const unsigned char *data, size_t len)
{
    register unsigned short a = 0;
    register unsigned short b = 0;

    while (len) {
        unsigned char c = *data++;
        a += c;
        b += len * c;
        len--;
    }
    {
        struct rsum r = { a, b };
        return r;
    }
}

/*
 * The main class which provides the qt zsync api.
 * This class is responsible to do the delta writing and only that,
 * This will not download anything but expects a downloader to submit data to
 * it inorder to check it and then write it in the correct location of the
 * temporary target file.
 *
 * Example:
 * 	ZsyncWriterPrivate writer;
 *
*/
ZsyncWriterPrivate::ZsyncWriterPrivate(void)
    : QObject()
{
    emit statusChanged(INITIALIZING);
    _pMd4Ctx.reset(new QCryptographicHash(QCryptographicHash::Md4));
#ifndef LOGGING_DISABLED
    _pLogger.reset(new QDebug(&_sLogBuffer));
#endif // LOGGING_DISABLED	

    connect(this, &ZsyncWriterPrivate::initStart, this, &ZsyncWriterPrivate::doStart, Qt::QueuedConnection);
    connect(this, &ZsyncWriterPrivate::error, this, &ZsyncWriterPrivate::resetConnections);
    connect(this, &ZsyncWriterPrivate::canceled, this, &ZsyncWriterPrivate::resetConnections);
    connect(this, &ZsyncWriterPrivate::finished, this, &ZsyncWriterPrivate::resetConnections);

    emit statusChanged(IDLE);
    return;
}

ZsyncWriterPrivate::~ZsyncWriterPrivate()
{
    /* Free all c allocator allocated memory */
    if(_pRsumHash)
        free(_pRsumHash);
    if(_pRanges)
        free(_pRanges);
    if(_pBlockHashes)
        free(_pBlockHashes);
    if(_pBitHash)
        free(_pBitHash);
    return;
}

/* Sets the output directory for the target file. */
void ZsyncWriterPrivate::setOutputDirectory(const QString &dir)
{
    _sOutputDirectory = QString(dir);
    return;
}

/* Sets the logger name. */
void ZsyncWriterPrivate::setLoggerName(const QString &name)
{
#ifndef LOGGING_DISABLED
    _sLoggerName = QString(name);
#else
    (void)name;
#endif
    return;
}

/* Turns on or off the internal logger. */
void ZsyncWriterPrivate::setShowLog(bool logNeeded)
{
#ifndef LOGGING_DISABLED
    if(logNeeded) {
        disconnect(this, &ZsyncWriterPrivate::logger, this, &ZsyncWriterPrivate::handleLogMessage);
        connect(this, &ZsyncWriterPrivate::logger, this, &ZsyncWriterPrivate::handleLogMessage);
    } else {
        disconnect(this, &ZsyncWriterPrivate::logger, this, &ZsyncWriterPrivate::handleLogMessage);
    }
#else
    (void)logNeeded;
#endif
    return;
}

#ifndef LOGGING_DISABLED
void ZsyncWriterPrivate::handleLogMessage(QString msg, QString path)
{
    qInfo().noquote()  << "["
                       <<  QDateTime::currentDateTime().toString(Qt::ISODate)
                       << "] "
                       << _sLoggerName
                       << "("
                       <<  QFileInfo(path).fileName() << ")::" << msg;
    return;
}
#endif // LOGGING_DISABLED

/* Emits the required block ranges via blockRange signal.
 *
 * If the entire file is to be downloaded , This will emit
 * a invalid block range which should be interpreted as to
 * download the entire file.
*/
void ZsyncWriterPrivate::getBlockRanges(void)
{
    if(!_pRanges || !_nRanges || _bAcceptRange == false) {
        /* Emitting an empty blockRange implies the downloader to download the
        * entire file sequentially.
         * In case we have no usable data and have to download the entire file , Also
         * if range requests are not support , We need to download the entire file.
         * The final SHA1 checksum will ensure that the target file is retrived from
         * a trusted source.
        */
        emit blockRange( 0, 0);
        emit endOfBlockRanges();
        emit statusChanged(IDLE);
        return;
    }

    INFO_START " getBlockRanges : emitting required block ranges." INFO_END;
    emit statusChanged(EMITTING_REQUIRED_BLOCK_RANGES);


    if(_pRequiredRanges.isEmpty()) {
        zs_blockid from = 0, to = _nBlocks;
        _pRequiredRanges.append(qMakePair(from, to));
        _pRequiredRanges.append(qMakePair(0, 0));

        for(qint32 i = 0, n = 1; i < _nRanges ; ++i) {
            _pRequiredRanges.append(qMakePair(0, 0));
            if (_pRanges[2 * i] > _pRequiredRanges.at(n - 1).second)
                continue;
            if (_pRanges[2 * i + 1] < from)
                continue;

            /* Okay, they intersect */
            if (n == 1 && _pRanges[2 * i] <= from) {       /* Overlaps the start of our window */
                _pRequiredRanges[0].first = _pRanges[2 * i + 1] + 1;
            } else {
                /* If the last block that we still (which is the last window end -1, due
                 * to half-openness) then this range just cuts the end of our window */
                if (_pRanges[2 * i + 1] >= _pRequiredRanges.at(n - 1).second - 1) {
                    _pRequiredRanges[n - 1].second = _pRanges[2 * i];
                } else {
                    /* In the middle of our range, split it */
                    _pRequiredRanges[n].first = _pRanges[2 * i + 1] + 1;
                    _pRequiredRanges[n].second = _pRequiredRanges.at(n-1).second;
                    _pRequiredRanges[n-1].second = _pRanges[2 * i];
                    n++;
                }
            }
            QCoreApplication::processEvents();
        }
        if (_pRequiredRanges.at(0).first >= _pRequiredRanges.at(0).second)
            _pRequiredRanges.clear();

        _pRequiredRanges.removeAll(qMakePair(0, 0));
    }

    for(auto iter = _pRequiredRanges.constBegin(), end  = _pRequiredRanges.constEnd(); iter != end; ++iter) {
        auto to = ((*iter).second >= _nBlocks) ? _nTargetFileLength :
                  ((*iter).second << _nBlockShift) + _nBlockSize;
        auto from = (*iter).first << _nBlockShift;

        INFO_START " getBlockRanges : (" LOGR from LOGR " , " LOGR to LOGR ")." INFO_END;

        emit blockRange(from, to);
        QCoreApplication::processEvents();
    }
    emit endOfBlockRanges();
    emit statusChanged(IDLE);
    INFO_START " getBlockRanges : emitted required block ranges." INFO_END;
    return;
}

/* Returns the total number of bytes written to the working target file. */
qint32 ZsyncWriterPrivate::getBytesWritten(void)
{
    return _nBytesWritten;
}

void ZsyncWriterPrivate::downloadFinished(void)
{
    if(!_pTargetFile->isOpen() || !_pTargetFile->autoRemove())
        return;

    verifyAndConstructTargetFile();
    return;
}

/* Simply writes whatever in downloadedData to the working target file ,
 * Used only if the downloader is downloading the entire file.
 * This automatically manages the memory of the given pointer to
 * QByteArray.
*/
void ZsyncWriterPrivate::rawSeqWrite(QByteArray *downloadedData)
{
    QScopedPointer<QByteArray> data(downloadedData);
    if(!_pTargetFile->isOpen()) {
        /*
         * If the target file is not opened then it most likely means
         * that the file is constructed successfully and so we have
         * no business in writting any further data.
        */
        return;
    }

    _nBytesWritten += _pTargetFile->write(*(data.data()));
    return;
}

/*
 * Writes the range in the correct block location.
 * Compares all blocks with the rolling checksum parsed
 * from the zsync control file.
 * Incase there is a mismatch , Only verified blocks are written the working target file.
*/
void ZsyncWriterPrivate::writeBlockRanges(qint32 fromRange, qint32 toRange, QByteArray *downloadedData)
{


    unsigned char md4sum[CHECKSUM_SIZE];
    /* Build checksum hash tables if we don't have them yet */
    if (!_pRsumHash) {
        if (!buildHash()) {
            emit error(CANNOT_CONSTRUCT_HASH_TABLE);
            return;
        }
    }

    /* Check if transfer speed time already started , if not start it. */
    if(_pTransferSpeed->isNull()) {
        _pTransferSpeed->start();
    }

    bool Md4ChecksumsMatched = true;
    QScopedPointer<QByteArray> downloaded(downloadedData);
    QScopedPointer<QBuffer> buffer(new QBuffer(downloadedData));
    buffer->open(QIODevice::ReadOnly);

    zs_blockid bfrom = fromRange >> _nBlockShift,
               bto   = (toRange == _nTargetFileLength) ? _nBlocks : (toRange - _nBlockSize) >> _nBlockShift;

    emit statusChanged(WRITTING_DOWNLOADED_BLOCK_RANGES);

    /*
     * Only check if the to blockid is not the end blockid ,
     * If we are writting the end blockid then simply write it to file ,
     * Later the final checksum will verify everything anyways.
     * This is because the end blockid not always accurate to the target file
     * length , Therefore the md4 checks fail on the end block which makes it
     * impossible to finish the delta update eventhough everything is authentic.
    */
    if(bto != _nBlocks) {
        for (zs_blockid x = bfrom; x <= bto; ++x) {
            QByteArray blockData = buffer->read(_nBlockSize);
            calcMd4Checksum(&md4sum[0], (const unsigned char*)blockData.constData(), _nBlockSize);
            if(memcmp(&md4sum, &(_pBlockHashes[x].checksum[0]), _nStrongCheckSumBytes)) {
                Md4ChecksumsMatched = false;
                WARNING_START " writeBlockRanges : md4 checksums mismatch." WARNING_END;
                if (x > bfrom) {    /* Write any good blocks we did get */
                    INFO_START " writeBlockRanges : only writting good blocks. " INFO_END;
                    writeBlocks((const unsigned char*)downloaded->constData(), bfrom, x - 1);

                    /*
                     * Remove the entire range eventhough we did'nt write the
                     * entire range , This is because in some cases only unwanted
                     * data is marked as mismatched (like trailing zeros).
                     * So to tolerate this , We remove the entire range only
                     * if we write something , Thus when all ranges are finished
                     * verifyAndConstructTargetFile() will automatically check for
                     * integrity.
                     *
                     * If integrity check failed , When we request required again
                     * , The _pRequiredRanges vector gets filled with needed blocks.
                     */
                    if(!_pRequiredRanges.isEmpty())
                        _pRequiredRanges.removeAll(qMakePair(bfrom, bto));

                }
                break;
            }
            QCoreApplication::processEvents();
        }
    }

    if(Md4ChecksumsMatched) {
        /* All blocks are valid; write them and update our state */
        writeBlocks((const unsigned char*)downloadedData->constData(), bfrom, bto );

        /* Remove the blocks we written successfully. */
        if(!_pRequiredRanges.isEmpty())
            _pRequiredRanges.removeAll(qMakePair(bfrom, bto));
    }
    emit blockRangesWritten(fromRange, toRange, /*all blocks were written with no md4 mismatch=*/Md4ChecksumsMatched);

    /* Calculate our progress. */
    {
        qint64 bytesReceived = _nBytesWritten, bytesTotal = _nTargetFileLength;
        QString sUnit;
        int nPercentage = static_cast<int>(
                              (static_cast<float>
                               (bytesReceived) * 100.0
                              ) / static_cast<float>
                              (bytesTotal)
                          );
        double nSpeed =  bytesReceived * 1000.0 / _pTransferSpeed->elapsed();
        if (nSpeed < 1024) {
            sUnit = "bytes/sec";
        } else if (nSpeed < 1024 * 1024) {
            nSpeed /= 1024;
            sUnit = "kB/s";
        } else {
            nSpeed /= 1024 * 1024;
            sUnit = "MB/s";
        }
        emit progress(nPercentage, bytesReceived, bytesTotal, nSpeed, sUnit);
    }
    emit statusChanged(IDLE);
    return;
}

/*
 * Sets the configuration for the current ZsyncWriterPrivate.
 * This can be seen as somewhat like init.
 * Emits finishedConfiguring when finished.
*/
void ZsyncWriterPrivate::setConfiguration(qint32 blocksize,
        qint32 nblocks,
        qint32 weakChecksumBytes,
        qint32 strongChecksumBytes,
        qint32 seqMatches,
        qint32 targetFileLength,
        const QString &sourceFilePath,
        const QString &targetFileName,
        const QString &targetFileSHA1,
        QBuffer *targetFileCheckSumBlocks,
        bool rangeSupported)
{
    _pCurrentWeakCheckSums = qMakePair(rsum({ 0, 0 }), rsum({ 0, 0 }));
    _nBlocks = nblocks,
    _nBlockSize = blocksize,
    _nBlockShift = (blocksize == 1024) ? 10 : (blocksize == 2048) ? 11 : log2(blocksize);
    _nBytesWritten = 0;
    _nContext = blocksize * seqMatches;
    _nWeakCheckSumBytes = weakChecksumBytes;
    _pWeakCheckSumMask = _nWeakCheckSumBytes < 3 ? 0 : _nWeakCheckSumBytes == 3 ? 0xff : 0xffff;
    _nStrongCheckSumBytes = strongChecksumBytes;
    _nSeqMatches = seqMatches;
    _nTargetFileLength = targetFileLength;
    _pTargetFileCheckSumBlocks.reset(targetFileCheckSumBlocks);
    _nSkip = _nNextKnown =_pHashMask = _pBitHashMask = 0;
    _pRover = _pNextMatch = nullptr;
    _bAcceptRange = rangeSupported;
    if(_pBlockHashes) {
        free(_pBlockHashes);
        _pBlockHashes = nullptr;
    }
    _pBlockHashes = (hash_entry*)calloc(_nBlocks + _nSeqMatches, sizeof(_pBlockHashes[0]));

    if(_pRanges) {
        free(_pRanges);
        _pRanges = nullptr;
        _nRanges = 0;
    }
    _pRequiredRanges.clear();
    _pMd4Ctx->reset();

    _sSourceFilePath = sourceFilePath;
    _sTargetFileName = targetFileName;
    _sTargetFileSHA1 = targetFileSHA1;

    short errorCode = 0;
    if((errorCode = parseTargetFileCheckSumBlocks()) > 0) {
        emit error(errorCode);
        return;
    }


    INFO_START " setConfiguration : creating temporary file." INFO_END;
    auto path = (_sOutputDirectory.isEmpty()) ? QFileInfo(_sSourceFilePath).path() : _sOutputDirectory;
    path = (path == "." ) ? QDir::currentPath() : path;
    auto targetFilePath = path + "/" + _sTargetFileName + ".XXXXXXXXXX.part";

    QFileInfo perm(path);
    if(!perm.isWritable() || !perm.isReadable()) {
        emit error(NO_PERMISSION_TO_READ_WRITE_TARGET_FILE);
        return;
    }

    _pTargetFile.reset(new QTemporaryFile(targetFilePath));
    if(!_pTargetFile->open()) {
        emit error(CANNOT_OPEN_TARGET_FILE);
        return;
    }
    /*
     * To open the target file we have to
     * request fileName() from the temporary file.
    */
    (void)_pTargetFile->fileName();
    INFO_START " setConfiguration : temporary file will temporarily reside at " LOGR _pTargetFile->fileName() LOGR "." INFO_END;
    emit finishedConfiguring();
    return;
}

/* Starts the zsync algorithm. */
void ZsyncWriterPrivate::start(void)
{
    QTimer::singleShot(1000, this, SIGNAL(initStart()));
    return;
}

/* Cancels the started process.
 * This actually emits a signal which gets queued and
 * when the event loop is not busy , doCancel private slot
 * will be executed cleanly.
*/
void ZsyncWriterPrivate::cancel(void)
{
    emit initCancel();
    return;
}

/* Private slot to actually cancel the started process. */
void ZsyncWriterPrivate::doCancel(void)
{
    _bCancelRequested = true;
    return;
}

/* Private slot to actually start the zsync algorithm. */
void ZsyncWriterPrivate::doStart(void)
{
    INFO_START " start : starting delta writer." INFO_END;

    /*
     * Disconnect this slot to prevent consecutive calls.
     * Kind of like mutex but better than that.
    */
    disconnect(this, &ZsyncWriterPrivate::initStart, this, &ZsyncWriterPrivate::doStart);
    connect(this, &ZsyncWriterPrivate::initCancel, this,  &ZsyncWriterPrivate::doCancel, Qt::QueuedConnection);


    short errorCode = 0;
    _bCancelRequested = false;

    /*
     * Check if we have some incomplete downloads.
     * If so delete them since they are garbage and can
     * make the delta writer very slow.
    */
    QStringList foundGarbageFiles;

    {
        QStringList filters;
        filters << _sTargetFileName + ".*.part";

        QDir dir(QFileInfo(_pTargetFile->fileName()).path());
        auto foundGarbageFilesInfo = dir.entryInfoList(filters);
        QDir seedFileDir(QFileInfo(_sSourceFilePath).path());
        foundGarbageFilesInfo << seedFileDir.entryInfoList(filters);


        for(auto iter = foundGarbageFilesInfo.constBegin(),
            end = foundGarbageFilesInfo.constEnd();
            iter != end;
            ++iter
           ) {
            foundGarbageFiles << (*iter).absoluteFilePath();
            QCoreApplication::processEvents();
        }
        foundGarbageFiles.removeAll(QFileInfo(_pTargetFile->fileName()).absoluteFilePath());
        foundGarbageFiles.removeDuplicates();
    }

    for(auto iter = foundGarbageFiles.constBegin(),
        end  = foundGarbageFiles.constEnd();
        iter != end && _nBytesWritten < _nTargetFileLength;
        ++iter) {
        QFile::remove((*iter));
        QCoreApplication::processEvents();
    }

    emit started();
    if(_bAcceptRange == true) {
        /*
         * Check if we have the target file already downloaded
         * in the output of the target file directory.
        */
        {
            QString alreadyDownloadedTargetFile = QFileInfo(_pTargetFile->fileName()).path() + "/" + _sTargetFileName;
            QFileInfo info(alreadyDownloadedTargetFile);
            if(info.exists() && info.isReadable()) {
                QFile *targetFile = nullptr;
                if((errorCode = tryOpenSourceFile(alreadyDownloadedTargetFile, &targetFile)) > 0) {
                    emit error(errorCode);
                    qDebug() << errorCode;
                    return;
                }

                if(submitSourceFile(targetFile) < 0) {
                    _bCancelRequested = false;
                    connect(this, &ZsyncWriterPrivate::initStart,
                            this, &ZsyncWriterPrivate::doStart, Qt::QueuedConnection);
                    disconnect(this, &ZsyncWriterPrivate::initCancel,
                               this,  &ZsyncWriterPrivate::doCancel);
                    qDebug() << "Submit source file returned non zero.";
                    return;
                }

            }
        }

        if(_nBytesWritten < _nTargetFileLength && _bAcceptRange == true) {
            QFile *sourceFile = nullptr;
            if((errorCode = tryOpenSourceFile(_sSourceFilePath, &sourceFile)) > 0) {
                emit error(errorCode);
                return;
            }

            if(submitSourceFile(sourceFile) < 0) {
                _bCancelRequested = false;
                connect(this, &ZsyncWriterPrivate::initStart, this, &ZsyncWriterPrivate::doStart, Qt::QueuedConnection);
                disconnect(this, &ZsyncWriterPrivate::initCancel, this,  &ZsyncWriterPrivate::doCancel);
                qDebug() << "Submit source file returned 2 non zero";
                return;
            }
            delete sourceFile;
        }
    }

    if(_nBytesWritten >= _nTargetFileLength) {
        verifyAndConstructTargetFile();
    } else {
        emit download();
    }
    return;
}

/*
 * Incase of an error , This private slot sets the connections to
 * default.
*/
void ZsyncWriterPrivate::resetConnections(void)
{
    _bCancelRequested = false;
    connect(this, &ZsyncWriterPrivate::initStart, this, &ZsyncWriterPrivate::doStart, Qt::QueuedConnection);
    disconnect(this, &ZsyncWriterPrivate::initCancel, this,  &ZsyncWriterPrivate::doCancel);
    return;
}

/*
 * This private method parses the raw checksum blocks from the zsync control file
 * and then constructs the hash table , If some error is detected , this returns
 * a non zero value with respect to the error code intrinsic to this class.
 *
 * Note:
 * 	This has to be called before any other methods , Because without the
 * 	hash table we cannot compare anything.
 *
 * Example:
 * 	short errorCode = parseTargetFileCheckSumBlocks();
 * 	if(errorCode > 0)
 * 		// Handle error.
*/
short ZsyncWriterPrivate::parseTargetFileCheckSumBlocks(void)
{
    if(!_pBlockHashes) {
        return HASH_TABLE_NOT_ALLOCATED;
    } else if(!_pTargetFileCheckSumBlocks ||
              _pTargetFileCheckSumBlocks->size() < (_nWeakCheckSumBytes + _nStrongCheckSumBytes)) {
        return INVALID_TARGET_FILE_CHECKSUM_BLOCKS;
    } else {
        if(!_pTargetFileCheckSumBlocks->open(QIODevice::ReadOnly))
            return CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS;
    }

    _pTargetFileCheckSumBlocks->seek(0);

    for(zs_blockid id = 0; id < _nBlocks && !_pTargetFileCheckSumBlocks->atEnd(); ++id) {
        rsum r = { 0, 0 };
        unsigned char checksum[16];

        /* Read on. */
        if (_pTargetFileCheckSumBlocks->read(((char *)&r) + 4 - _nWeakCheckSumBytes, _nWeakCheckSumBytes) < 1
            || _pTargetFileCheckSumBlocks->read((char *)&checksum, _nStrongCheckSumBytes) < 1) {
            return QBUFFER_IO_READ_ERROR;
        }

        /* Convert to host endian and store.
         * We need to convert from network endian to host endian ,
         * Network endian is nothing but big endian byte order , So if we have little endian byte order ,
         * We need to convert the data but if we have a big endian byte order ,
         * We can simply avoid this conversion to save computation power.
         *
         * But most of the time we will need little endian since intel's microproccessors always follows
         * the little endian byte order.
        */
        if(Q_BYTE_ORDER == Q_LITTLE_ENDIAN) {
            r.a = qFromBigEndian(r.a);
            r.b = qFromBigEndian(r.b);
        }


        /* Get hash entry with checksums for this block */
        hash_entry *e = &(_pBlockHashes[id]);

        /* Enter checksums */
        memcpy(e->checksum, checksum, _nStrongCheckSumBytes);
        e->r.a = r.a & _pWeakCheckSumMask;
        e->r.b = r.b;

        QCoreApplication::processEvents();
    }

    /* New checksums invalidate any existing checksum hash tables */
    if (_pRsumHash) {
        free(_pRsumHash);
        _pRsumHash = NULL;
        free(_pBitHash);
        _pBitHash = NULL;
    }
    return 0;
}

/*
 * This is a private method which tries to open the given seed file
 * in the given path.
 * This method checks for the existence and the read permission of
 * the file.
 * If any of the two condition does not satisfy , This method returns
 * a error code with respect to the intrinsic error codes defined in this
 * class , else returns 0.
*/
short ZsyncWriterPrivate::tryOpenSourceFile(const QString &filePath, QFile **sourceFile)
{
    if(filePath.isEmpty()) {
        return 0;
    }

    auto seedFile = new QFile(filePath);
    /* Check if the file actually exists. */
    if(!seedFile->exists()) {
        delete seedFile;
        return SOURCE_FILE_NOT_FOUND;
    }
    /* Check if we have the permission to read it. */
    auto perm = seedFile->permissions();
    if(
        !(perm & QFileDevice::ReadUser) &&
        !(perm & QFileDevice::ReadGroup) &&
        !(perm & QFileDevice::ReadOther)
    ) {
        delete seedFile;
        return NO_PERMISSION_TO_READ_SOURCE_FILE;
    }
    /*
     * Finally open the file.
     */
    if(!seedFile->open(QIODevice::ReadOnly)) {
        delete seedFile;
        return CANNOT_OPEN_SOURCE_FILE;
    }
    *sourceFile = seedFile;
    return 0;
}


/*
 * This private slot verifies if the current working target file matches
 * the final SHA1 Hash of the actual target file which resides in a remote
 * server.
 * Returns true if successfully constructed the target file.
*/
bool ZsyncWriterPrivate::verifyAndConstructTargetFile(void)
{
    if(!_pTargetFile->isOpen() || !_pTargetFile->autoRemove()) {
        return true;
    }

    bool constructed = false;
    QString UnderConstructionFileSHA1;
    qint64 bufferSize = 0;
    QScopedPointer<QCryptographicHash> SHA1Hasher(new QCryptographicHash(QCryptographicHash::Sha1));

    /*
     * Truncate and Seek.
     **/
    _pTargetFile->resize(_nTargetFileLength);
    _pTargetFile->seek(0);

    INFO_START " verifyAndConstructTargetFile : calculating sha1 hash on temporary target file. " INFO_END;
    emit statusChanged(CALCULATING_TARGET_FILE_SHA1_HASH);
    if(_nTargetFileLength >= 1073741824) { // 1 GiB and more.
        bufferSize = 104857600; // copy per 100 MiB.
    } else if(_nTargetFileLength >= 1048576 ) { // 1 MiB and more.
        bufferSize = 1048576; // copy per 1 MiB.
    } else if(_nTargetFileLength  >= 1024) { // 1 KiB and more.
        bufferSize = 4096; // copy per 4 KiB.
    } else { // less than 1 KiB
        bufferSize = 1024; // copy per 1 KiB.
    }

    while(!_pTargetFile->atEnd()) {
        SHA1Hasher->addData(_pTargetFile->read(bufferSize));
        QCoreApplication::processEvents();
    }
    UnderConstructionFileSHA1 = QString(SHA1Hasher->result().toHex().toUpper());

    INFO_START " verifyAndConstructTargetFile : comparing temporary target file sha1 hash(" LOGR UnderConstructionFileSHA1
    LOGR ") and remote target file sha1 hash(" LOGR _sTargetFileSHA1 INFO_END;

    if(UnderConstructionFileSHA1 == _sTargetFileSHA1) {
        INFO_START " verifyAndConstructTargetFile : sha1 hash matches!" INFO_END;
        emit statusChanged(CONSTRUCTING_TARGET_FILE);
        QString newTargetFileName;
        _pTargetFile->setAutoRemove(!(constructed = true));
        /*
        * Rename the new version with current time stamp.
         * Do not touch anything else.
               * Note: Since we checked for permissions earlier
               * , We don't need to verify it again.
               */
        {
            QFileInfo sameFile(QFileInfo(_pTargetFile->fileName()).path() + "/" + _sTargetFileName);
            if(sameFile.exists()) {
                newTargetFileName = sameFile.baseName() +
                                    QString("-revised-on-") +
                                    QDateTime::currentDateTime().toString(Qt::ISODate)
                                    .replace(":", "-")
                                    .replace(" ", "-") +
                                    QString(".") +
                                    sameFile.completeSuffix();

                INFO_START " verifyAndConstructTargetFile : file with target file name exists." INFO_END;
                INFO_START " verifyAndConstructTargetFile : renaming new version as " LOGR newTargetFileName LOGR "." INFO_END;
            } else {
                newTargetFileName = _sTargetFileName;
            }
        }
        _pTargetFile->rename(QFileInfo(_pTargetFile->fileName()).path() + "/" + newTargetFileName);
        _pTargetFile->setPermissions(QFileInfo(_sSourceFilePath).permissions()); // Set the same permissions as the old version.
        _pTargetFile->close();
    } else {
        FATAL_START " verifyAndConstructTargetFile : sha1 hash mismatch." FATAL_END;
        emit statusChanged(IDLE);
        emit error(TARGET_FILE_SHA1_HASH_MISMATCH);
        return constructed;
    }

    /*
     * Emit finished signal.
    */
    QJsonObject newVersionDetails {
        {"AbsolutePath", QFileInfo(_pTargetFile->fileName()).absoluteFilePath() },
        {"Sha1Hash", UnderConstructionFileSHA1}
    };
    emit finished(newVersionDetails, _sSourceFilePath);
    emit statusChanged(IDLE);
    return constructed;
}

/* Given a hash table entry, check the data in this block against every entry
 * in the linked list for this hash entry, checking the checksums for this
 * block against those recorded in the hash entries.
 *
 * If we get a hit (checksums match a desired block), write the data to that
 * block in the target file and update our state accordingly to indicate that
 * we have got that block successfully.
 *
 * Return the number of blocks successfully obtained.
 */
qint32 ZsyncWriterPrivate::checkCheckSumsOnHashChain(const struct hash_entry *e, const unsigned char *data,int onlyone)
{
    unsigned char md4sum[2][CHECKSUM_SIZE];
    signed int done_md4 = -1;
    qint32 got_blocks = 0;
    register rsum rs = _pCurrentWeakCheckSums.first;

    /* This is a hint to the caller that they should try matching the next
     * block against a particular hash entry (because at least _nSeqMatches
     * prior blocks to it matched in sequence). Clear it here and set it below
     * if and when we get such a set of matches. */
    _pNextMatch = NULL;

    /* This is essentially a for (;e;e=e->next), but we want to remove links from
     * the list as we find matches, without keeping too many temp variables.
     */
    _pRover = e;
    while (_pRover) {
        zs_blockid id;

        e = _pRover;
        _pRover = onlyone ? NULL : e->next;

        /* Check weak checksum first */

        // HashHit++
        if (e->r.a != (rs.a & _pWeakCheckSumMask) || e->r.b != rs.b) {
            continue;
        }

        id = getHashEntryBlockId( e);

        if (!onlyone && _nSeqMatches > 1
            && (_pBlockHashes[id + 1].r.a != (_pCurrentWeakCheckSums.second.a & _pWeakCheckSumMask)
                || _pBlockHashes[id + 1].r.b != _pCurrentWeakCheckSums.second.b))
            continue;

        // WeakHit++

        {
            int ok = 1;
            signed int check_md4 = 0;
            zs_blockid next_known = -1;

            /* This block at least must match; we must match at least
             * _nSeqMatches-1 others, which could either be trailing stuff,
             * or these could be preceding blocks that we have verified
             * already. */
            do {
                /* We only calculate the MD4 once we need it; but need not do so twice */
                if (check_md4 > done_md4) {
                    calcMd4Checksum(&md4sum[check_md4][0],
                                    data + _nBlockSize * check_md4,
                                    _nBlockSize);
                    done_md4 = check_md4;
                    // Checksummed++
                }

                /* Now check the strong checksum for this block */
                if (memcmp(&md4sum[check_md4],
                           &_pBlockHashes[id + check_md4].checksum[0],
                           _nStrongCheckSumBytes)) {
                    ok = 0;
                } else if (next_known == -1) {
                }
                check_md4++;
                QCoreApplication::processEvents();
            } while (ok && !onlyone && check_md4 < _nSeqMatches);

            if (ok) {
                qint32 num_write_blocks;

                /* Find the next block that we already have data for. If this
                 * is part of a run of matches then we have this stored already
                 * as ->next_known. */
                zs_blockid next_known = onlyone ? _nNextKnown : nextKnownBlock( id);

                // stronghit++

                if (next_known > id + check_md4) {
                    num_write_blocks = check_md4;

                    /* Save state for this run of matches */
                    _pNextMatch = &(_pBlockHashes[id + check_md4]);
                    if (!onlyone) _nNextKnown = next_known;
                } else {
                    /* We've reached the EOF, or data we already know. Just
                     * write out the blocks we don't know, and that's the end
                     * of this run of matches. */
                    num_write_blocks = next_known - id;
                }

                /* Write out the matched blocks that we don't yet know */
                writeBlocks( data, id, id + num_write_blocks - 1);
                got_blocks += num_write_blocks;
            }
        }
    }
    return got_blocks;
}

/* Reads the supplied data (length datalen) and identifies any contained blocks
 * of data that can be used to make up the target file.
 *
 * offset should be 0 for a new data stream (or if our position in the data
 * stream has been changed and does not match the last call) or should be the
 * offset in the whole source stream otherwise.
 *
 * Returns the number of blocks in the target file that we obtained as a result
 * of reading this buffer.
 *
 * IMPLEMENTATION:
 * We maintain the following state:
 * _nSkip - the number of bytes to skip next time we enter ZsyncWriterPrivate::submitSourceData
 *        e.g. because we've just matched a block and the forward jump takes
 *        us past the end of the buffer
 * _pCurrentWeakCheckSums.first - rolling checksum of the first blocksize bytes of the buffer
 * _pCurrentWeakCheckSums.second - rolling checksum of the next blocksize bytes of the buffer (if _nSeqMatches > 1)
 */
qint32 ZsyncWriterPrivate::submitSourceData(unsigned char *data,size_t len, off_t offset)
{
    /* The window in data[] currently being considered is
     * [x, x+bs)
     */
    qint32 x = 0;
    register qint32 bs = _nBlockSize;
    qint32 got_blocks = 0;

    if (offset) {
        x = _nSkip;
    } else {
        _pNextMatch = NULL;
    }

    if (x || !offset) {
        _pCurrentWeakCheckSums.first = calc_rsum_block(data + x, bs);
        if (_nSeqMatches > 1)
            _pCurrentWeakCheckSums.second = calc_rsum_block(data + x + bs, bs);
    }
    _nSkip = 0;

    /* Work through the block until the current blocksize bytes being
     * considered, starting at x, is at the end of the buffer */
    for (;;) {
        if ((size_t)(x + _nContext) == len) {
            return got_blocks;
        }
        {
            /* # of blocks of the output file we got from this data */
            qint32 thismatch = 0;
            /* # of blocks to advance if thismatch > 0. Can be less than
             * thismatch as thismatch could be N*blocks_matched, if a block was
             * duplicated to multiple locations in the output file. */
            qint32 blocks_matched = 0;

            /* If the previous block was a match, but we're looking for
             * sequential matches, then test this block against the block in
             * the target immediately after our previous hit. */
            if (_pNextMatch && _nSeqMatches > 1) {
                if (0 != (thismatch = checkCheckSumsOnHashChain( _pNextMatch, data + x, 1))) {
                    blocks_matched = 1;
                }
            }
            if (!thismatch) {
                const struct hash_entry *e;

                /* Do a hash table lookup - first in the _pBitHash (fast negative
                 * check) and then in the rsum hash */
                unsigned hash = _pCurrentWeakCheckSums.first.b;
                hash ^= ((_nSeqMatches > 1) ? _pCurrentWeakCheckSums.second.b
                         : _pCurrentWeakCheckSums.first.a & _pWeakCheckSumMask) << BITHASHBITS;
                if ((_pBitHash[(hash & _pBitHashMask) >> 3] & (1 << (hash & 7))) != 0
                    && (e = _pRsumHash[hash & _pHashMask]) != NULL) {

                    /* Okay, we have a hash hit. Follow the hash chain and
                     * check our block against all the entries. */
                    thismatch = checkCheckSumsOnHashChain( e, data + x, 0);
                    if (thismatch)
                        blocks_matched = _nSeqMatches;
                }
            }
            got_blocks += thismatch;

            /* If we got a hit, skip forward (if a block in the target matches
             * at x, it's highly unlikely to get a hit at x+1 as all the
             * target's blocks are multiples of the blocksize apart. */
            if (blocks_matched) {
                x += bs + (blocks_matched > 1 ? bs : 0);

                if ((size_t)(x + _nContext) > len) {
                    /* can't calculate rsum for block after this one, because
                     * it's not in the buffer. So leave a hint for next time so
                     * we know we need to recalculate */
                    _nSkip = x + _nContext - len;
                    return got_blocks;
                }

                /* If we are moving forward just 1 block, we already have the
                 * following block rsum. If we are skipping both, then
                 * recalculate both */
                if (_nSeqMatches > 1 && blocks_matched == 1)
                    _pCurrentWeakCheckSums.first = _pCurrentWeakCheckSums.second;
                else
                    _pCurrentWeakCheckSums.first = calc_rsum_block(data + x, bs);
                if (_nSeqMatches > 1)
                    _pCurrentWeakCheckSums.second = calc_rsum_block(data + x + bs, bs);
                continue;
            }
        }

        /* Else - advance the window by 1 byte - update the rolling checksum
         * and our offset in the buffer */
        {
            unsigned char Nc = data[x + bs * 2];
            unsigned char nc = data[x + bs];
            unsigned char oc = data[x];
            UPDATE_RSUM(_pCurrentWeakCheckSums.first.a, _pCurrentWeakCheckSums.first.b, oc, nc, _nBlockShift);
            if (_nSeqMatches > 1)
                UPDATE_RSUM(_pCurrentWeakCheckSums.second.a, _pCurrentWeakCheckSums.second.b, nc, Nc, _nBlockShift);
        }
        x++;
    }
}

/* Read the given stream, applying the rsync rolling checksum algorithm to
 * identify any blocks of data in common with the target file. Blocks found are
 * written to our working target output.
 */
qint32 ZsyncWriterPrivate::submitSourceFile(QFile *file)
{
    if(!file) {
        return 0;
    }

    qint32 error = 0;
    off_t in = 0;
    /* Allocate buffer of 16 blocks */
    register qint32 bufsize = _nBlockSize * 16;
    unsigned char *buf = (unsigned char*)malloc(bufsize + _nContext);
    if (!buf)
        return (error = -1);

    /* Build checksum hash tables ready to analyse the blocks we find */
    if (!_pRsumHash)
        if (!buildHash()) {
            free(buf);
            return (error = -2);
        }

    if(_pTransferSpeed.isNull()) {
        _pTransferSpeed.reset(new QTime);
    } else if(!_pTransferSpeed->isNull()) {
        _pTransferSpeed.reset(new QTime);
    }

    _pTransferSpeed->start();
    while (!file->atEnd()) {
        size_t len;
        off_t start_in = in;

        /* If this is the start, fill the buffer for the first time */
        if (!in) {
            len = file->read((char*)buf, bufsize);
            in += len;
        }

        /* Else, move the last _nContext bytes from the end of the buffer to the
         * start, and refill the rest of the buffer from the stream. */
        else {
            memcpy(buf, buf + (bufsize - _nContext), _nContext);
            in += bufsize - _nContext;
            len = _nContext + file->read((char*)(buf + _nContext), (bufsize - _nContext));
        }

        if (file->atEnd()) {          /* 0 pad to complete a block */
            memset(buf + len, 0, _nContext);
            len += _nContext;
        }

        /* Process the data in the buffer, and report progress */
        submitSourceData( buf, len, start_in);
        {
            qint64 bytesReceived = _nBytesWritten,
                   bytesTotal = _nTargetFileLength;

            int nPercentage = static_cast<int>(
                                  (static_cast<float>
                                   ( bytesReceived ) * 100.0
                                  ) / static_cast<float>
                                  (
                                      bytesTotal
                                  )
                              );

            double nSpeed =  bytesReceived * 1000.0 / _pTransferSpeed->elapsed();
            QString sUnit;
            if (nSpeed < 1024) {
                sUnit = "bytes/sec";
            } else if (nSpeed < 1024 * 1024) {
                nSpeed /= 1024;
                sUnit = "kB/s";
            } else {
                nSpeed /= 1024 * 1024;
                sUnit = "MB/s";
            }

            emit progress(nPercentage, bytesReceived, bytesTotal, nSpeed, sUnit);
        }
        QCoreApplication::processEvents();
        if(_bCancelRequested == true) {
            error = -3;
            _bCancelRequested = false;
            emit canceled();
            break;
        }
    }
    _pTransferSpeed.reset(new QTime);
    file->close();
    free(buf);
    return error;
}



/* Build hash tables to quickly lookup a block based on its rsum value.
 * Returns non-zero if successful.
 */
qint32 ZsyncWriterPrivate::buildHash(void)
{
    zs_blockid id;
    qint32 i = 16;

    /* Try hash size of 2^i; step down the value of i until we find a good size
     */
    while ((2 << (i - 1)) > _nBlocks && i > 4) {
        i--;
        QCoreApplication::processEvents();
    }

    /* Allocate hash based on rsum */
    _pHashMask = (2 << i) - 1;
    _pRsumHash = (hash_entry**)calloc(_pHashMask + 1, sizeof *(_pRsumHash));
    if (!_pRsumHash)
        return 0;

    /* Allocate bit-table based on rsum */
    _pBitHashMask = (2 << (i + BITHASHBITS)) - 1;
    _pBitHash = (unsigned char*)calloc(_pBitHashMask + 1, 1);
    if (!_pBitHash) {
        free(_pRsumHash);
        _pRsumHash = NULL;
        return 0;
    }

    /* Now fill in the hash tables.
     * Minor point: We do this in reverse order, because we're adding entries
     * to the hash chains by prepending, so if we iterate over the data in
     * reverse then the resulting hash chains have the blocks in normal order.
     * That's improves our pattern of I/O when writing out identical blocks
     * once we are processing data; we will write them in order. */
    for (id = _nBlocks; id > 0;) {
        /* Decrement the loop variable here, and get the hash entry. */
        hash_entry *e = _pBlockHashes + (--id);

        /* Prepend to linked list for this hash entry */
        unsigned h = calcRHash( e);
        e->next = _pRsumHash[h & _pHashMask];
        _pRsumHash[h & _pHashMask] = e;

        /* And set relevant bit in the _pBitHash to 1 */
        _pBitHash[(h & _pBitHashMask) >> 3] |= 1 << (h & 7);

        QCoreApplication::processEvents();
    }
    return 1;
}

/* Remove the given data block from the rsum hash table, so it won't be
 * returned in a hash lookup again (e.g. because we now have the data)
 */
void ZsyncWriterPrivate::removeBlockFromHash(zs_blockid id)
{
    hash_entry *t = &(_pBlockHashes[id]);

    hash_entry **p = &(_pRsumHash[calcRHash(t) & _pHashMask]);

    while (*p != NULL) {
        if (*p == t) {
            if (t == _pRover) {
                _pRover = t->next;
            }
            *p = (*p)->next;
            return;
        } else {
            p = &((*p)->next);
        }
        QCoreApplication::processEvents();
    }
}


/* This determines which of the existing known ranges x falls in.
 * It returns -1 if it is inside an existing range (it doesn't tell you which
 *  one; if you already have it, that usually is enough to know).
 * Or it returns 0 if x is before the 1st range;
 * 1 if it is between ranges 1 and 2 (array indexes 0 and 1)
 * ...
 * _nRanges if it is after the last range
 */
qint32 ZsyncWriterPrivate::rangeBeforeBlock(zs_blockid x)
{
    /* Lowest number and highest number block that it could be inside (0 based) */
    register qint32 min = 0, max = _nRanges-1;

    /* By bisection */
    for (; min<=max;) {
        /* Range number to compare against */
        register qint32 r = (max+min)/2;

        if (x > _pRanges[2*r+1]) min = r+1;  /* After range r */
        else if (x < _pRanges[2*r]) max = r-1;/* Before range r */
        else return -1;                     /* In range r */
    }

    /* If we reach here, we know min = max + 1 and we were below range max+1
     * and above range min-1.
     * So we're between range max and max + 1
     * So we return max + 1  (return value is 1 based)  ( = min )
     */
    return min;
}

/* Mark the given blockid as known, updating the stored known ranges
 * appropriately */
void ZsyncWriterPrivate::addToRanges(zs_blockid x)
{
    qint32 r = rangeBeforeBlock(x);

    if (r == -1) {
        /* Already have this block */
    } else {
        /* If between two ranges and exactly filling the hole between them,
         * merge them */
        if (r > 0 && r < _nRanges
            && _pRanges[2 * (r - 1) + 1] == x - 1
            && _pRanges[2 * r] == x + 1) {

            // This block fills the gap between two areas that we have got completely. Merge the adjacent ranges
            _pRanges[2 * (r - 1) + 1] = _pRanges[2 * r + 1];
            memmove(&_pRanges[2 * r], &_pRanges[2 * r + 2],
                    (_nRanges - r - 1) * sizeof(_pRanges[0]) * 2);
            _nRanges--;
        }

        /* If adjoining a range below, add to it */
        else if (r > 0 && _nRanges && _pRanges[2 * (r - 1) + 1] == x - 1) {
            _pRanges[2 * (r - 1) + 1] = x;
        }

        /* If adjoining a range above, add to it */
        else if (r < _nRanges && _pRanges[2 * r] == x + 1) {
            _pRanges[2 * r] = x;
        }

        else { /* New range for this block alone */
            _pRanges = (zs_blockid*)
                       realloc(_pRanges,
                               (_nRanges + 1) * 2 * sizeof(_pRanges[0]));
            memmove(&_pRanges[2 * r + 2], &_pRanges[2 * r],
                    (_nRanges - r) * 2 * sizeof(_pRanges[0]));
            _pRanges[2 * r] = _pRanges[2 * r + 1] = x;
            _nRanges++;
        }
    }
}

/* Return true if blockid x of the target file is already known */
qint32 ZsyncWriterPrivate::alreadyGotBlock(zs_blockid x)
{
    return (rangeBeforeBlock(x) == -1);
}

/* Returns the blockid of the next block which we already have data for.
 * If we know the requested block, it returns the blockid given; otherwise it
 * will return a later blockid.
 * If no later blocks are known, it returns numblocks (i.e. the block after
 * the end of the file).
 */
zs_blockid ZsyncWriterPrivate::nextKnownBlock(zs_blockid x)
{
    qint32 r = rangeBeforeBlock(x);
    if (r == -1)
        return x;
    if (r == _nRanges) {
        return _nBlocks;
    }
    /* Else return first block of next known range. */
    return _pRanges[2*r];
}

/* Calculates the rsum hash table hash for the given hash entry. */
unsigned ZsyncWriterPrivate::calcRHash(const hash_entry *const e)
{
    unsigned h = e[0].r.b;

    h ^= ((_nSeqMatches > 1) ? e[1].r.b
          : e[0].r.a & _pWeakCheckSumMask) << BITHASHBITS;

    return h;
}

/* Returns the hash entry's blockid. */
zs_blockid ZsyncWriterPrivate::getHashEntryBlockId(const hash_entry *e)
{
    return e - _pBlockHashes;
}


/* Writes the block range (inclusive) from the supplied buffer to the given
 * under-construction output file */
void ZsyncWriterPrivate::writeBlocks(const unsigned char *data, zs_blockid bfrom, zs_blockid bto)
{
    if(!_pTargetFile->isOpen() || !_pTargetFile->autoRemove())
        return;


    off_t len = ((off_t) (bto - bfrom + 1)) << _nBlockShift;
    off_t offset = ((off_t)bfrom) << _nBlockShift;

    auto pos = _pTargetFile->pos();
    _pTargetFile->seek(offset);
    _nBytesWritten += _pTargetFile->write((char*)data, len);
    _pTargetFile->seek(pos);

    {   /* Having written those blocks, discard them from the rsum hashes (as
         * we don't need to identify data for those blocks again, and this may
         * speed up lookups (in particular if there are lots of identical
         * blocks), and add the written blocks to the record of blocks that we
         * have received and stored the data for */
        int id;
        for (id = bfrom; id <= bto; id++) {
            removeBlockFromHash(id);
            addToRanges(id);
            QCoreApplication::processEvents();
        }
    }
    return;
}

/* Calculates the Md4 Checksum of the given data with respect to the given len. */
void ZsyncWriterPrivate::calcMd4Checksum(unsigned char *c, const unsigned char *data, size_t len)
{
    _pMd4Ctx->reset();
    _pMd4Ctx->addData((const char*)data, len);
    auto result = _pMd4Ctx->result();
    memmove(c, result.constData(), sizeof(const char) * result.size());
    return;
}

QString ZsyncWriterPrivate::errorCodeToString(short errorCode)
{
    QString ret = "AppImageUpdaterBridge::errorCode(";
    switch (errorCode) {
    case HASH_TABLE_NOT_ALLOCATED:
        ret += "HASH_TABLE_NOT_ALLOCATED)";
        break;
    case INVALID_TARGET_FILE_CHECKSUM_BLOCKS:
        ret += "INVALID_TARGET_FILE_CHECKSUM_BLOCKS)";
        break;
    case CANNOT_CONSTRUCT_HASH_TABLE:
        ret += "CANNOT_CONSTRUCT_HASH_TABLE)";
        break;
    case CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS:
        ret += "CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS)";
        break;
    case QBUFFER_IO_READ_ERROR:
        ret += "QBUFFER_IO_READ_ERROR)";
        break;
    case SOURCE_FILE_NOT_FOUND:
        ret += "SOURCE_FILE_NOT_FOUND)";
        break;
    case NO_PERMISSION_TO_READ_SOURCE_FILE:
        ret += "NO_PERMISSION_TO_READ_SOURCE_FILE)";
        break;
    case CANNOT_OPEN_SOURCE_FILE:
        ret += "CANNOT_OPEN_SOURCE_FILE)";
        break;
    case NO_PERMISSION_TO_READ_WRITE_TARGET_FILE:
        ret += "NO_PERMISSION_TO_READ_WRITE_TARGET_FILE)";
        break;
    case CANNOT_OPEN_TARGET_FILE:
        ret += "CANNOT_OPEN_TARGET_FILE)";
        break;
    case TARGET_FILE_SHA1_HASH_MISMATCH:
        ret += "TARGET_FILE_SHA1_HASH_MISMATCH)";
        break;
    default:
        ret += "Unknown)";
        break;
    }
    return ret;
}

QString ZsyncWriterPrivate::statusCodeToString(short statusCode)
{
    QString ret = "AppImageUpdaterBridge::statusCode(";
    switch (statusCode) {
    case WRITTING_DOWNLOADED_BLOCK_RANGES:
        ret += "WRITTING_DOWNLOADED_BLOCK_RANGES)";
        break;
    case EMITTING_REQUIRED_BLOCK_RANGES:
        ret += "EMITTING_REQUIRED_BLOCK_RANGES)";
        break;
    case CHECKING_CHECKSUMS_FOR_DOWNLOADED_BLOCK_RANGES:
        ret += "CHECKING_CHECKSUMS_FOR_DOWNLOADED_BLOCK_RANGES)";
        break;
    case WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE:
        ret += "WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE)";
        break;
    case CALCULATING_TARGET_FILE_SHA1_HASH:
        ret += "CALCULATING_TARGET_FILE_SHA1_HASH)";
        break;
    case CONSTRUCTING_TARGET_FILE:
	ret += "CONSTRUCTING_TARGET_FILE)";
	break;
    default:
        ret += "Unknown)";
        break;
    }
    return ret;
}
