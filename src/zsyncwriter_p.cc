/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018-2019, Antony jr
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
 * @filename    : zsyncwriter_p.cc
 * @description : This is where the main zsync algorithm is implemented.
*/
#include <cstdlib>

#include "zsyncwriter_p.hpp"
#include "qappimageupdateenums.hpp"
#include "helpers_p.hpp"

/*
 * An efficient logging system specially tailored
 * for this source file.
 *
 * Example:
 * 	LOGS "This is a log message." LOGE
 *
 *
*/
#ifndef LOGGING_DISABLED
#define LOGS *(p_Logger.data()) <<
#define LOGR <<
#define LOGE ; \
	     emit(logger(s_LogBuffer , s_SourceFilePath)); \
	     s_LogBuffer.clear();
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

/* Update a already calculated block ,
 * This is why a rolling checksum is needed. */
#define UPDATE_RSUM(a, b, oldc, newc, bshift) do { \
						(a) += ((unsigned char)(newc)) - ((unsigned char)(oldc));\
       						(b) += (a) - ((oldc) << (bshift)); \
					      } while (0)



/*
 * Zsync uses the same modified version of the Adler32 checksum
 * as in rsync as the rolling checksum , here after denoted by rsum.
 * Calculate the rsum for a single block of data. */
static rsum __attribute__ ((pure)) calc_rsum_block(const unsigned char *data, size_t len) {
    unsigned short a = 0;
    unsigned short b = 0;

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
 * it in order to check it and then write it in the correct location of the
 * temporary target file.
 * Needs a block range downloader in order to construct.
 *
*/
ZsyncWriterPrivate::ZsyncWriterPrivate(QNetworkAccessManager *manager)
    : QObject() {
    m_Manager = manager;
    p_Md4Ctx.reset(new QCryptographicHash(QCryptographicHash::Md4));
#ifndef LOGGING_DISABLED
    p_Logger.reset(new QDebug(&s_LogBuffer));
#endif // LOGGING_DISABLED	
    return;
}

ZsyncWriterPrivate::~ZsyncWriterPrivate() {
    /* Free all c allocator allocated memory */
    if(p_RsumHash)
        free(p_RsumHash);
    if(p_Ranges)
        free(p_Ranges);
    if(p_BlockHashes)
        free(p_BlockHashes);
    if(p_BitHash)
        free(p_BitHash);
    return;
}

/* Sets the output directory for the target file. */
void ZsyncWriterPrivate::setOutputDirectory(const QString &dir) {
    if(b_Started)
        return;
    s_OutputDirectory = QString(dir);
    return;
}

/* Sets the logger name. */
void ZsyncWriterPrivate::setLoggerName(const QString &name) {
    if(b_Started)
        return;
#ifndef LOGGING_DISABLED
    s_LoggerName = QString(name);
#else
    (void)name;
#endif
    return;
}

/* Turns on or off the internal logger. */
void ZsyncWriterPrivate::setShowLog(bool logNeeded) {
#ifndef LOGGING_DISABLED
    if(logNeeded) {
        connect(this, SIGNAL(logger(QString, QString)),
                this, SLOT(handleLogMessage(QString, QString)),
                Qt::UniqueConnection);
        return;
    }
    disconnect(this, SIGNAL(logger(QString, QString)),
               this, SLOT(handleLogMessage(QString, QString)));
#else
    (void)logNeeded;
#endif
    return;
}

#ifndef LOGGING_DISABLED
void ZsyncWriterPrivate::handleLogMessage(QString msg, QString path) {
    qInfo().noquote()  << "["
                       <<  QDateTime::currentDateTime().toString(Qt::ISODate)
                       << "] "
                       << s_LoggerName
                       << "("
                       <<  QFileInfo(path).fileName() << ")::" << msg;
    return;
}
#endif // LOGGING_DISABLED

// Returns the required ranges
bool ZsyncWriterPrivate::getBlockRanges() {
    if(!p_Ranges || !n_Ranges || b_AcceptRange == false) {
        return false;
    }

    INFO_START " getBlockRanges : getting required block ranges." INFO_END;

    int i, n;
    int alloc_n = 100;
    zs_blockid from = 0, to = n_Blocks;
    zs_blockid *r = (zs_blockid*)malloc(2 * alloc_n * sizeof(zs_blockid));

    if (!r)
        return NULL;

    r[0] = from;
    r[1] = to;
    n = 1;
    /* Note r[2*n-1] is the last range in our prospective list */

    for (i = 0; i < n_Ranges; i++) {
        if (p_Ranges[2 * i] > r[2 * n - 1])
            continue;
        if (p_Ranges[2 * i + 1] < from)
            continue;

        /* Okay, they intersect */
        if (n == 1 && p_Ranges[2 * i] <= from) {       /* Overlaps the start of our window */
            r[0] = p_Ranges[2 * i + 1] + 1;
        } else {
            /* If the last block that we still (which is the last window end -1, due
             * to half-openness) then this range just cuts the end of our window */
            if (p_Ranges[2 * i + 1] >= r[2 * n - 1] - 1) {
                r[2 * n - 1] = p_Ranges[2 * i];
            } else {
                /* In the middle of our range, split it */
                r[2 * n] = p_Ranges[2 * i + 1] + 1;
                r[2 * n + 1] = r[2 * n - 1];
                r[2 * n - 1] = p_Ranges[2 * i];
                n++;
                if (n == alloc_n) {
                    zs_blockid *r2;
                    alloc_n += 100;
                    r2 = (zs_blockid*)realloc(r, 2 * alloc_n * sizeof *r);
                    if (!r2) {
                        free(r);
                        return NULL;
                    }
                    r = r2;
                }
            }
        }
    }
    r = (zs_blockid*)realloc(r, 2 * n * sizeof *r);
    if (n == 1 && r[0] >= r[1]) {
        n = 0;
    }

    for(i = 0; i < n; ++i) {
        // Note: to = to * blocksize - 1; As given by author.
        auto to = r[2*i + 1];
        auto from = r[2*i];


        INFO_START " getBlockRanges : (" LOGR from LOGR " , " LOGR to LOGR ")." INFO_END;

        m_RangeDownloader->appendRange(from, to);
        QCoreApplication::processEvents();
    }

    INFO_START " getBlockRanges : requesting " LOGR n LOGR " requests to server." INFO_END;

    free((void*)r);
    return true;
}

/* Simply writes whatever in downloadedData to the working target file ,
 * Used only if the downloader is downloading the entire file.
 * This automatically manages the memory of the given pointer to
 * QByteArray.
*/
void ZsyncWriterPrivate::writeDataSequential(QByteArray *dataFragment, bool isLast) {
    QScopedPointer<QByteArray> data(dataFragment);
    if(!p_TargetFile->isOpen()) {
        /*
         * If the target file is not opened then it most likely means
         * that the file is constructed successfully and so we have
         * no business in writting any further data.
        */
        return;
    }

    // Not to be confused with writeBlocks method
    // which updates n_BytesWritten by itself.
    n_BytesWritten += p_TargetFile->write(*(data.data()));
    if(isLast) {
        QTimer::singleShot(2500, this, &ZsyncWriterPrivate::verifyAndConstructTargetFile);
    }
    return;
}

/*
 * Writes the range in the correct block location.
 * Compares all blocks with the rolling checksum parsed
 * from the zsync control file.
 * Incase there is a mismatch , Only verified blocks are written the working target file.
*/
void ZsyncWriterPrivate::writeBlockRanges(qint32 fromBlock, qint32 toBlock, QByteArray *downloadedData, bool isLast) {
    unsigned char md4sum[CHECKSUM_SIZE];
    /* Build checksum hash tables if we don't have them yet */
    if (!p_RsumHash) {
        if (!buildHash()) {
            emit error(QAppImageUpdateEnums::Error::CannotConstructHashTable);
            return;
        }
    }

    bool Md4ChecksumsMatched = true;
    QScopedPointer<QByteArray> downloaded(downloadedData);
    QScopedPointer<QBuffer> buffer(new QBuffer(downloadedData));
    buffer->open(QIODevice::ReadOnly);



    // In the original code the author uses the similar with the following equation,
    // bfrom = rangeFrom / blocksize
    // bto = bfrom + blocks - 1 .. (1)
    //
    // Such that blocks = bto - bfrom .. (2)
    //
    // If we substitute (2) in (1) we get,
    // bto = bfrom + bto - bfrom - 1
    //     = bto - 1 .. (3)
    //
    // Hence I've used bto = <original to block value> - 1;
    //
    //
    // Reason for using this is, the writeBlock function which writes the
    // given data to the actual working block uses the following equation as
    // length
    //
    // length = bto - brom + 1 .. (4)
    //
    // Equating (3) in (4) we get,
    // length = bfrom + bto - bfrom - bfrom - 1 + 1
    //        = bto - bfrom
    //        = actual no. of blocks got.
    zs_blockid bfrom = fromBlock,
               bto = toBlock - 1;

    for (zs_blockid x = bfrom; x <= bto; ++x) {
        QByteArray blockData = buffer->read(n_BlockSize);
        //// Fill with zeros if the block size is less than the required blocksize.
        if(blockData.size() != n_BlockSize) {
            INFO_START " writeBlockRanges : padding block(" LOGR bfrom LOGR "," LOGR bto LOGR ")." INFO_END;
            QByteArray newBlockData;
            newBlockData.fill('\0', (n_BlockSize - blockData.size()));
            blockData.append(newBlockData);
        }
        calcMd4Checksum(&md4sum[0], (const unsigned char*)blockData.constData(), n_BlockSize);
        if(memcmp(&md4sum, &(p_BlockHashes[x].checksum[0]), n_StrongCheckSumBytes)) {
            Md4ChecksumsMatched = false;
            WARNING_START " writeBlockRanges : block(" LOGR bfrom LOGR "," LOGR bto LOGR ")." WARNING_END;
            WARNING_START " writeBlockRanges : MD4 checksums mismatch." WARNING_END;
            WARNING_START " writeBlockRanges : MD4 Sum of Data : " LOGR
            QByteArray((const char *)(&md4sum[0])).toHex() WARNING_END;
            WARNING_START " writeBlockRanges : MD4 Sum of Required :  " LOGR
            QByteArray((const char *)(&(p_BlockHashes[x].checksum[0]))).toHex() WARNING_END;
            if (x > bfrom) {    /* Write any good blocks we did get */
                INFO_START " writeBlockRanges : only writting good blocks. " INFO_END;
                writeBlocks((const unsigned char*)downloaded->constData(), bfrom, x - 1);
            }
            break;
        }
        QCoreApplication::processEvents();
    }


    if(Md4ChecksumsMatched) {
        writeBlocks((const unsigned char*)downloaded->constData(), bfrom, bto);
    }


    if(isLast) {
        QTimer::singleShot(2500, this, &ZsyncWriterPrivate::verifyAndConstructTargetFile);
    }

    return;
}

/*
 * Sets the configuration for the current ZsyncWriterPrivate.
 * This can be seen as somewhat like init.
 * Emits finishedConfiguring when finished.
 *
 * **IMPORTANT**: You should call this always before calling start.
 * Even if canceled. If you are going to call start again, You are instructed
 * to setConfiguration first and then start after getting finishedConfiguration signal.
 *
 * This is because setConfiguration acts as clear and new config setter. Also
 * b_Configured is set to True in setConfiguration and set to False in start.
 *
 * So start only works once after setConfiguration.
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
        QUrl targetFileUrl,
        QBuffer *targetFileCheckSumBlocks,
        bool rangeSupported,
        QUrl torrentFileUrl) {
    p_CurrentWeakCheckSums = qMakePair(rsum({ 0, 0 }), rsum({ 0, 0 }));
    n_Blocks = nblocks,
    n_BlockSize = blocksize,
    n_BlockShift = (blocksize == 1024) ? 10 : (blocksize == 2048) ? 11 : log2(blocksize);
    n_BytesWritten = 0;
    n_Context = blocksize * seqMatches;
    n_WeakCheckSumBytes = weakChecksumBytes;
    p_WeakCheckSumMask = n_WeakCheckSumBytes < 3 ? 0 : n_WeakCheckSumBytes == 3 ? 0xff : 0xffff;
    n_StrongCheckSumBytes = strongChecksumBytes;
    n_SeqMatches = seqMatches;
    n_TargetFileLength = targetFileLength;
    p_TargetFileCheckSumBlocks.reset(targetFileCheckSumBlocks);
    n_Skip = n_NextKnown =p_HashMask = p_BitHashMask = 0;
    p_Rover = p_NextMatch = nullptr;
    b_AcceptRange = rangeSupported;
    b_TorrentAvail = torrentFileUrl.isValid();
    u_TorrentFileUrl = torrentFileUrl;

    // Since Zsync Writer is only finished officially when all the data is sent and SHA-1 hashes match.
    // But sometimes the block range downloader can have a error or could be canceled and the Zsync Writer
    // will still be in the started state and ignore any further start calls which makes the entire object
    // unusable.
    // So the we have to default the started flag to false. Trivially the start call will be initiated on
    // the control file parser side and the control file parser initiats the zsync writer(this class)
    // after it has parsed the control file.
    // Without the below line, the zsync writer will not recover from a error or cancel.
    b_Started = b_CancelRequested = false;

    u_TargetFileUrl = targetFileUrl;
    if(p_BlockHashes) {
        free(p_BlockHashes);
        p_BlockHashes = nullptr;
    }
    p_BlockHashes = (hash_entry*)calloc(n_Blocks + n_SeqMatches, sizeof(p_BlockHashes[0]));

    if(p_Ranges) {
        free(p_Ranges);
        p_Ranges = nullptr;
        n_Ranges = 0;
    }
    p_Md4Ctx->reset();

    s_SourceFilePath = sourceFilePath;
    s_TargetFileName = targetFileName;
    s_TargetFileSHA1 = targetFileSHA1;

    short errorCode = 0;
    if((errorCode = parseTargetFileCheckSumBlocks()) > 0) {
        emit error(errorCode);
        return;
    }


    INFO_START " setConfiguration : creating temporary file." INFO_END;
    auto path = (s_OutputDirectory.isEmpty()) ? QFileInfo(s_SourceFilePath).path() : s_OutputDirectory;
    path = (path == "." ) ? QDir::currentPath() : path;
    auto targetFilePath = path + "/" + s_TargetFileName + ".XXXXXXXXXX.part";

    QFileInfo perm(path);
    if(!perm.isWritable() || !perm.isReadable()) {
        emit error(QAppImageUpdateEnums::Error::NoPermissionToReadWriteTargetFile);
        return;
    }

    p_TargetFile.reset(new QTemporaryFile(targetFilePath));
    if(!p_TargetFile->open()) {
        emit error(QAppImageUpdateEnums::Error::CannotOpenTargetFile);
        return;
    }
    /*
     * To open the target file we have to
     * request fileName() from the temporary file.
    */
    (void)p_TargetFile->fileName();
    INFO_START " setConfiguration : temporary file will temporarily reside at " LOGR p_TargetFile->fileName() LOGR "." INFO_END;

    /// Create a range downloader or a Torrent Client to download the update
    /// in a decentralized way. Saves bandwidth for the server.

#if defined(DECENTRALIZED_UPDATE_ENABLED) && LIBTORRENT_VERSION_NUM >= 10208
    if(b_TorrentAvail && b_AcceptRange) {
        INFO_START " setConfiguration : candidate suitable for decentralized update" INFO_END;
        m_TorrentDownloader.reset(new TorrentDownloader(m_Manager));
    } else {
        WARNING_START " setConfiguration : candidate not suitable for decentralized update" WARNING_END;
        m_RangeDownloader.reset(new RangeDownloader(m_Manager));
    }
#else
    if(b_TorrentAvail && b_AcceptRange) {
        WARNING_START " setConfiguration : candidate suitable for decentralized update but this build has no capabilities"
        WARNING_END;
    }
    m_RangeDownloader.reset(new RangeDownloader(m_Manager));
#endif // DECENTRALIZED_UPDATE_ENABLED

    b_Configured = true;
    emit finishedConfiguring();
    return;
}

/* cancels the started process. */
void ZsyncWriterPrivate::cancel() {
    INFO_START " cancel : cancel requested." INFO_END;
    if(!b_Started) {
        INFO_START " cancel : called before started!" INFO_END;
        return;
    }
    b_CancelRequested = true;
#if defined(DECENTRALIZED_UPDATE_ENABLED) && LIBTORRENT_VERSION_NUM >= 10208
    if(b_TorrentAvail && b_AcceptRange) {
        if(!m_TorrentDownloader.isNull()) {
            m_TorrentDownloader->cancel();
        }
    } else {
        if(!m_RangeDownloader.isNull()) {
            INFO_START " cancel : cancel requested to Range Downloader." INFO_END;
	    m_RangeDownloader->cancel();
        }
    }
#else
    if(!m_RangeDownloader.isNull()) {
        m_RangeDownloader->cancel();
    }
#endif // DECENTRALIZED_UPDATE_ENABLED
    INFO_START " cancel : cancel requested " LOGR b_CancelRequested LOGR "." INFO_END;
    return;
}

/// You should only start after getting finishedConfiguring signal.
//  If not, start does not work.
/* start the zsync algorithm. */
void ZsyncWriterPrivate::start() {
    if(b_Started || !b_Configured)
        return;
    b_Configured = false;
    b_CancelRequested = false;
    b_Started = true;
    emit started();

    INFO_START " start : starting delta writer." INFO_END;
    short errorCode = 0;

    /*
     * Check if we have some incomplete downloads.
     * if so then add them as a seed file then delete them.
    */
    QStringList foundGarbageFiles;
    {
        QStringList filters;
        filters << s_TargetFileName + ".*.part";

        QDir dir(QFileInfo(p_TargetFile->fileName()).path());
        auto foundGarbageFilesInfo = dir.entryInfoList(filters);
        QDir seedFileDir(QFileInfo(s_SourceFilePath).path());
        foundGarbageFilesInfo << seedFileDir.entryInfoList(filters);


        for(auto iter = foundGarbageFilesInfo.constBegin(),
                end = foundGarbageFilesInfo.constEnd();
                iter != end;
                ++iter
           ) {
            foundGarbageFiles << (*iter).absoluteFilePath();
            QCoreApplication::processEvents();
        }
        foundGarbageFiles.removeAll(QFileInfo(p_TargetFile->fileName()).absoluteFilePath());
        foundGarbageFiles.removeDuplicates();
    }

    if(b_AcceptRange == true) {
        /*
         * Check if we have the target file already downloaded
         * in the output of the target file directory.
        */
        {
            QString alreadyDownloadedTargetFile = QFileInfo(p_TargetFile->fileName()).path() + "/" + s_TargetFileName;
            QFileInfo info(alreadyDownloadedTargetFile);
            if(info.exists() && info.isReadable()) {
                QFile *targetFile = nullptr;
                if((errorCode = tryOpenSourceFile(alreadyDownloadedTargetFile, &targetFile)) > 0) {
                    emit error(errorCode);
                    return;
                }


                int r = 0;
                if((r = submitSourceFile(targetFile)) < 0) {
                    delete targetFile;
                    if(r == -2) {
                        /// Cannot construst hash table.
                        b_Started = b_CancelRequested = false;
                        emit error(QAppImageUpdateEnums::Error::HashTableNotAllocated);
                    } else if(r == -3) {
                        /// Canceled the update
                        b_Started = false;
                    }

                    if(r != -1) {
                        /// -1 cannot allocated memory.
                        return;
                    }
                }
                delete targetFile;
            }
        }

        if(n_BytesWritten < n_TargetFileLength) {
            for(auto iter = foundGarbageFiles.constBegin(),
                    end  = foundGarbageFiles.constEnd();
                    iter != end && n_BytesWritten < n_TargetFileLength;
                    ++iter) {
                QFile *sourceFile = nullptr;
                if((errorCode = tryOpenSourceFile(*iter, &sourceFile)) > 0) {
                    emit error(errorCode);
                    return;
                }

                int r = 0;
                if((r = submitSourceFile(sourceFile)) < 0) {
                    delete sourceFile;
                    if(r == -2) {
                        /// Cannot construst hash table.
                        b_Started = b_CancelRequested = false;
                        emit error(QAppImageUpdateEnums::Error::HashTableNotAllocated);
                    } else if(r == -3) {
                        /// Canceled the update
                        b_Started = false;
                    }
                    if(r != -1) {
                        /// -1 cannot allocated memory.
                        return;
                    }
                }
                delete sourceFile;
                QFile::remove((*iter));
            }
        }


        if(n_BytesWritten < n_TargetFileLength) {
            QFile *sourceFile = nullptr;
            if((errorCode = tryOpenSourceFile(s_SourceFilePath, &sourceFile)) > 0) {
                emit error(errorCode);
                return;
            }

            int r = 0;
            if((r = submitSourceFile(sourceFile)) < 0) {
                delete sourceFile;
                if(r == -1) {
                    /// Cannot allocate buffer memory
                    b_Started = b_CancelRequested = false;
                    emit error(QAppImageUpdateEnums::Error::NotEnoughMemory);
                } else if(r == -2) {
                    /// Cannot construst hash table.
                    b_Started = b_CancelRequested = false;
                    emit error(QAppImageUpdateEnums::Error::HashTableNotAllocated);
                } else if(r == -3) {
                    /// Canceled the update
                    b_Started = false;
                }
                b_Started = b_CancelRequested = false;
                return;
            }
            delete sourceFile;
        }
    }

    p_TransferSpeed.reset(new QElapsedTimer); // Refresh timer.
    p_TransferSpeed->start();

    if(n_BytesWritten >= n_TargetFileLength) {
        QCoreApplication::processEvents(); // Check if cancel requested.
        if(b_CancelRequested) {
            b_Started = b_CancelRequested = false;
            emit canceled();
            return;
        }
        verifyAndConstructTargetFile();
    }
#if defined(DECENTRALIZED_UPDATE_ENABLED) && LIBTORRENT_VERSION_NUM >= 10208
    /// See BEP 17 and BEP 19, Web seeds or url seeds needs a server which accepts
    /// range requests, It is possible for the torrent client to update if there are
    /// ample amounts of peers seeding it but there is no assurance that the AppImage
    /// is being seeded. So if we try torrenting with a unsupported url seed then
    /// the update will just be quietly waiting for seeds forever.
    /// So the best way is to just do a dumb http download.
    else if(b_TorrentAvail && b_AcceptRange) {
        m_TorrentDownloader->setTargetFileDone(n_BytesWritten);
        m_TorrentDownloader->setTargetFileLength(n_TargetFileLength);
        m_TorrentDownloader->setTorrentFileUrl(u_TorrentFileUrl);
        m_TorrentDownloader->setTargetFile(p_TargetFile.data());
        m_TorrentDownloader->setTargetFileUrl(u_TargetFileUrl);

        connect(m_TorrentDownloader.data(), &TorrentDownloader::finished,
                this, &ZsyncWriterPrivate::verifyAndConstructTargetFile, Qt::QueuedConnection);
        connect(m_TorrentDownloader.data(), &TorrentDownloader::error,
                this, &ZsyncWriterPrivate::handleTorrentError, Qt::QueuedConnection);
        connect(m_TorrentDownloader.data(), &TorrentDownloader::canceled,
                this, &ZsyncWriterPrivate::handleCancel, Qt::QueuedConnection);
        connect(m_TorrentDownloader.data(), &TorrentDownloader::logger,
                this, &ZsyncWriterPrivate::handleTorrentLogger);

        connect(m_TorrentDownloader.data(), &TorrentDownloader::progress,
                this, &ZsyncWriterPrivate::progress, Qt::DirectConnection);

        m_TorrentDownloader->start();
    }
#endif // DECENTRALIZED_UPDATE_ENABLED
    else {
        m_RangeDownloader->setTargetFileLength(n_TargetFileLength);
        m_RangeDownloader->setBytesWritten(n_BytesWritten);

        if(!p_Ranges || !n_Ranges || b_AcceptRange == false) {
            m_RangeDownloader->setFullDownload(true);
            // Full Download
            connect(m_RangeDownloader.data(), &RangeDownloader::data,
                    this, &ZsyncWriterPrivate::writeDataSequential, Qt::QueuedConnection);
        } else {
            // Partial Download
            auto partial = getBlockRanges();
            m_RangeDownloader->setFullDownload(!partial);

            if(partial) {
                connect(m_RangeDownloader.data(), &RangeDownloader::rangeData,
                        this, &ZsyncWriterPrivate::writeBlockRanges, Qt::QueuedConnection);

            } else {
                connect(m_RangeDownloader.data(), &RangeDownloader::data,
                        this, &ZsyncWriterPrivate::writeDataSequential, Qt::QueuedConnection);

            }
        }

        connect(m_RangeDownloader.data(), &RangeDownloader::canceled,
                this, &ZsyncWriterPrivate::handleCancel, Qt::QueuedConnection);

        connect(m_RangeDownloader.data(), &RangeDownloader::progress,
                this, &ZsyncWriterPrivate::progress, Qt::DirectConnection);

        connect(m_RangeDownloader.data(), &RangeDownloader::error,
                this, &ZsyncWriterPrivate::handleNetworkError, Qt::QueuedConnection);

        m_RangeDownloader->setBlockSize(n_BlockSize);
        m_RangeDownloader->setTargetFileUrl(u_TargetFileUrl);
        m_RangeDownloader->start();
    }
    return;
}

void ZsyncWriterPrivate::handleNetworkError(QNetworkReply::NetworkError code) {
    b_Started = false;
    FATAL_START " handleNetworkError : " LOGR code FATAL_END;
    emit error(translateQNetworkReplyError(code));
}

#if defined(DECENTRALIZED_UPDATE_ENABLED) && LIBTORRENT_VERSION_NUM >= 10208
void ZsyncWriterPrivate::handleTorrentError(QNetworkReply::NetworkError code) {
    Q_UNUSED(code);
    FATAL_START " handleTorrentError : " LOGR code FATAL_END;
    INFO_START " handleTorrentError : Falling back to range download" INFO_END;

    m_TorrentDownloader->disconnect();
    m_TorrentDownloader.reset(nullptr); // Delete the torrent downloader completely.

    /// Reset everything to default.
    b_TorrentAvail = false;

    m_RangeDownloader.reset(new RangeDownloader(m_Manager));
    m_RangeDownloader->setTargetFileLength(n_TargetFileLength);
    m_RangeDownloader->setBytesWritten(n_BytesWritten);

    if(!p_Ranges || !n_Ranges || b_AcceptRange == false) {
        m_RangeDownloader->setFullDownload(true);
        // Full Download
        connect(m_RangeDownloader.data(), &RangeDownloader::data,
                this, &ZsyncWriterPrivate::writeDataSequential, Qt::QueuedConnection);
    } else {
        // Partial Download
        auto partial = getBlockRanges();
        m_RangeDownloader->setFullDownload(!partial);

        if(partial) {
            connect(m_RangeDownloader.data(), &RangeDownloader::rangeData,
                    this, &ZsyncWriterPrivate::writeBlockRanges, Qt::QueuedConnection);

        } else {
            connect(m_RangeDownloader.data(), &RangeDownloader::data,
                    this, &ZsyncWriterPrivate::writeDataSequential, Qt::QueuedConnection);
        }
    }

    connect(m_RangeDownloader.data(), &RangeDownloader::canceled,
            this, &ZsyncWriterPrivate::handleCancel, Qt::QueuedConnection);

    connect(m_RangeDownloader.data(), &RangeDownloader::progress,
            this, &ZsyncWriterPrivate::progress, Qt::DirectConnection);

    connect(m_RangeDownloader.data(), &RangeDownloader::error,
            this, &ZsyncWriterPrivate::handleNetworkError, Qt::QueuedConnection);

    m_RangeDownloader->setBlockSize(n_BlockSize);
    m_RangeDownloader->setTargetFileUrl(u_TargetFileUrl);
    m_RangeDownloader->start();
}

void ZsyncWriterPrivate::handleTorrentLogger(QString msg) {
    INFO_START msg INFO_END;
}
#endif // DECENTRALIZED_UPDATE_ENABLED

void ZsyncWriterPrivate::handleCancel() {
    b_CancelRequested = false;
    b_Started = false;
    INFO_START " handleCancel : canceled." INFO_END;
    emit canceled();
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
short ZsyncWriterPrivate::parseTargetFileCheckSumBlocks() {
    if(!p_BlockHashes) {
        return QAppImageUpdateEnums::Error::HashTableNotAllocated;
    } else if(!p_TargetFileCheckSumBlocks ||
              p_TargetFileCheckSumBlocks->size() < (n_WeakCheckSumBytes + n_StrongCheckSumBytes)) {
        return QAppImageUpdateEnums::Error::InvalidTargetFileChecksumBlocks;
    } else {
        if(!p_TargetFileCheckSumBlocks->open(QIODevice::ReadOnly))
            return QAppImageUpdateEnums::Error::CannotOpenTargetFileChecksumBlocks;
    }

    p_TargetFileCheckSumBlocks->seek(0);

    for(zs_blockid id = 0; id < n_Blocks && !p_TargetFileCheckSumBlocks->atEnd(); ++id) {
        rsum r = { 0, 0 };
        unsigned char checksum[16];

        /* Read on. */
        if (p_TargetFileCheckSumBlocks->read(((char *)&r) + 4 - n_WeakCheckSumBytes, n_WeakCheckSumBytes) < 1
                || p_TargetFileCheckSumBlocks->read((char *)&checksum, n_StrongCheckSumBytes) < 1) {
            return QAppImageUpdateEnums::Error::QbufferIoReadError;
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
        hash_entry *e = &(p_BlockHashes[id]);

        /* Enter checksums */
        memcpy(e->checksum, checksum, n_StrongCheckSumBytes);
        e->r.a = r.a & p_WeakCheckSumMask;
        e->r.b = r.b;

        QCoreApplication::processEvents();
    }

    /* New checksums invalidate any existing checksum hash tables */
    if (p_RsumHash) {
        free(p_RsumHash);
        p_RsumHash = NULL;
        free(p_BitHash);
        p_BitHash = NULL;
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
short ZsyncWriterPrivate::tryOpenSourceFile(const QString &filePath, QFile **sourceFile) {
    if(filePath.isEmpty()) {
        return 0;
    }

    auto seedFile = new QFile(filePath);
    /* Check if the file actually exists. */
    if(!seedFile->exists()) {
        delete seedFile;
        return QAppImageUpdateEnums::Error::SourceFileNotFound;
    }
    /* Check if we have the permission to read it. */
    auto perm = seedFile->permissions();
    if(
        !(perm & QFileDevice::ReadUser) &&
        !(perm & QFileDevice::ReadGroup) &&
        !(perm & QFileDevice::ReadOther)
    ) {
        delete seedFile;
        return QAppImageUpdateEnums::Error::NoPermissionToReadSourceFile;
    }
    /*
     * Finally open the file.
     */
    if(!seedFile->open(QIODevice::ReadOnly)) {
        delete seedFile;
        return QAppImageUpdateEnums::Error::CannotOpenSourceFile;
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
bool ZsyncWriterPrivate::verifyAndConstructTargetFile() {
    if(!p_TargetFile->isOpen() || !p_TargetFile->autoRemove()) {
        return true;
    }

    bool constructed = false;
    QString UnderConstructionFileSHA1;
    qint64 bufferSize = 0;
    QScopedPointer<QCryptographicHash> SHA1Hasher(new QCryptographicHash(QCryptographicHash::Sha1));

    /*
     * Truncate and Seek.
     **/
    p_TargetFile->resize(n_TargetFileLength);
    p_TargetFile->seek(0);

    INFO_START " verifyAndConstructTargetFile : calculating sha1 hash on temporary target file. " INFO_END;
    if(n_TargetFileLength >= 1073741824) { // 1 GiB and more.
        bufferSize = 104857600; // copy per 100 MiB.
    } else if(n_TargetFileLength >= 1048576 ) { // 1 MiB and more.
        bufferSize = 1048576; // copy per 1 MiB.
    } else if(n_TargetFileLength  >= 1024) { // 1 KiB and more.
        bufferSize = 4096; // copy per 4 KiB.
    } else { // less than 1 KiB
        bufferSize = 1024; // copy per 1 KiB.
    }

    while(!p_TargetFile->atEnd()) {
        SHA1Hasher->addData(p_TargetFile->read(bufferSize));
        QCoreApplication::processEvents();
    }
    UnderConstructionFileSHA1 = QString(SHA1Hasher->result().toHex().toUpper());

    INFO_START " verifyAndConstructTargetFile : comparing temporary target file sha1 hash(" LOGR UnderConstructionFileSHA1
    LOGR ") and remote target file sha1 hash(" LOGR s_TargetFileSHA1 INFO_END;

    if(UnderConstructionFileSHA1 == s_TargetFileSHA1) {
        INFO_START " verifyAndConstructTargetFile : sha1 hash matches!" INFO_END;
        QString newTargetFileName;
        p_TargetFile->setAutoRemove(!(constructed = true));
        /*
             * Rename the new version with current time stamp.
             * Do not touch anything else.
             * Note: Since we checked for permissions earlier
             * , We don't need to verify it again.
             */
        {
            QFileInfo sameFile(QFileInfo(p_TargetFile->fileName()).path() + "/" + s_TargetFileName);
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
                newTargetFileName = s_TargetFileName;
            }
        }
        p_TargetFile->rename(QFileInfo(p_TargetFile->fileName()).path() + "/" + newTargetFileName);

        /*Set the same permission as the old version and close. */
        p_TargetFile->setPermissions(QFileInfo(s_SourceFilePath).permissions());
        p_TargetFile->close();
    } else {
        b_Started = b_CancelRequested = false;
        FATAL_START " verifyAndConstructTargetFile : sha1 hash mismatch." FATAL_END;
        emit error(QAppImageUpdateEnums::Error::TargetFileSha1HashMismatch);
        return constructed;
    }

    /*
     * Emit finished signal.
    */

    QJsonObject newVersionDetails {
        {"AbsolutePath", QFileInfo(p_TargetFile->fileName()).absoluteFilePath() },
        {"Sha1Hash", UnderConstructionFileSHA1},
        {"UsedTorrent", b_TorrentAvail && b_AcceptRange}
    };
    b_Started = b_CancelRequested = false;
    emit finished(newVersionDetails, s_SourceFilePath);
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
qint32 ZsyncWriterPrivate::checkCheckSumsOnHashChain(const struct hash_entry *e, const unsigned char *data,int onlyone) {
    unsigned char md4sum[2][CHECKSUM_SIZE];
    signed int done_md4 = -1;
    qint32 got_blocks = 0;
    rsum rs = p_CurrentWeakCheckSums.first;

    /* This is a hint to the caller that they should try matching the next
     * block against a particular hash entry (because at least n_SeqMatches
     * prior blocks to it matched in sequence). Clear it here and set it below
     * if and when we get such a set of matches. */
    p_NextMatch = NULL;

    /* This is essentially a for (;e;e=e->next), but we want to remove links from
     * the list as we find matches, without keeping too many temp variables.
     */
    p_Rover = e;
    while (p_Rover) {
        zs_blockid id;

        e = p_Rover;
        p_Rover = onlyone ? NULL : e->next;

        /* Check weak checksum first */

        // HashHit++
        if (e->r.a != (rs.a & p_WeakCheckSumMask) || e->r.b != rs.b) {
            continue;
        }

        id = getHashEntryBlockId( e);

        if (!onlyone && n_SeqMatches > 1
                && (p_BlockHashes[id + 1].r.a != (p_CurrentWeakCheckSums.second.a & p_WeakCheckSumMask)
                    || p_BlockHashes[id + 1].r.b != p_CurrentWeakCheckSums.second.b))
            continue;

        // WeakHit++

        {
            int ok = 1;
            signed int check_md4 = 0;
            zs_blockid next_known = -1;

            /* This block at least must match; we must match at least
             * n_SeqMatches-1 others, which could either be trailing stuff,
             * or these could be preceding blocks that we have verified
             * already. */
            do {
                /* We only calculate the MD4 once we need it; but need not do so twice */
                if (check_md4 > done_md4) {
                    calcMd4Checksum(&md4sum[check_md4][0],
                                    data + n_BlockSize * check_md4,
                                    n_BlockSize);
                    done_md4 = check_md4;
                    // Checksummed++
                }

                /* Now check the strong checksum for this block */
                if (memcmp(&md4sum[check_md4],
                           &p_BlockHashes[id + check_md4].checksum[0],
                           n_StrongCheckSumBytes)) {
                    ok = 0;
                } else if (next_known == -1) {
                }
                check_md4++;
                QCoreApplication::processEvents();
            } while (ok && !onlyone && check_md4 < n_SeqMatches);

            if (ok) {
                qint32 num_write_blocks;

                /* Find the next block that we already have data for. If this
                 * is part of a run of matches then we have this stored already
                 * as ->next_known. */
                zs_blockid next_known = onlyone ? n_NextKnown : nextKnownBlock( id);

                // stronghit++

                if (next_known > id + check_md4) {
                    num_write_blocks = check_md4;

                    /* Save state for this run of matches */
                    p_NextMatch = &(p_BlockHashes[id + check_md4]);
                    if (!onlyone) n_NextKnown = next_known;
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
 * n_Skip - the number of bytes to skip next time we enter ZsyncWriterPrivate::submitSourceData
 *        e.g. because we've just matched a block and the forward jump takes
 *        us past the end of the buffer
 * p_CurrentWeakCheckSums.first - rolling checksum of the first blocksize bytes of the buffer
 * p_CurrentWeakCheckSums.second - rolling checksum of the next blocksize bytes of the buffer (if n_SeqMatches > 1)
 */
qint32 ZsyncWriterPrivate::submitSourceData(unsigned char *data,size_t len, off_t offset) {
    /* The window in data[] currently being considered is
     * [x, x+bs)
     */
    qint32 x = 0;
    qint32 bs = n_BlockSize;
    qint32 got_blocks = 0;

    if (offset) {
        x = n_Skip;
    } else {
        p_NextMatch = NULL;
    }

    if (x || !offset) {
        p_CurrentWeakCheckSums.first = calc_rsum_block(data + x, bs);
        if (n_SeqMatches > 1)
            p_CurrentWeakCheckSums.second = calc_rsum_block(data + x + bs, bs);
    }
    n_Skip = 0;

    /* Work through the block until the current blocksize bytes being
     * considered, starting at x, is at the end of the buffer */
    for (;;) {
        if ((size_t)(x + n_Context) == len) {
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
            if (p_NextMatch && n_SeqMatches > 1) {
                if (0 != (thismatch = checkCheckSumsOnHashChain( p_NextMatch, data + x, 1))) {
                    blocks_matched = 1;
                }
            }
            if (!thismatch) {
                const struct hash_entry *e;

                /* Do a hash table lookup - first in the p_BitHash (fast negative
                 * check) and then in the rsum hash */
                unsigned hash = p_CurrentWeakCheckSums.first.b;
                hash ^= ((n_SeqMatches > 1) ? p_CurrentWeakCheckSums.second.b
                         : p_CurrentWeakCheckSums.first.a & p_WeakCheckSumMask) << BITHASHBITS;
                if ((p_BitHash[(hash & p_BitHashMask) >> 3] & (1 << (hash & 7))) != 0
                        && (e = p_RsumHash[hash & p_HashMask]) != NULL) {

                    /* Okay, we have a hash hit. Follow the hash chain and
                     * check our block against all the entries. */
                    thismatch = checkCheckSumsOnHashChain( e, data + x, 0);
                    if (thismatch)
                        blocks_matched = n_SeqMatches;
                }
            }
            got_blocks += thismatch;

            /* If we got a hit, skip forward (if a block in the target matches
             * at x, it's highly unlikely to get a hit at x+1 as all the
             * target's blocks are multiples of the blocksize apart. */
            if (blocks_matched) {
                x += bs + (blocks_matched > 1 ? bs : 0);

                if ((size_t)(x + n_Context) > len) {
                    /* can't calculate rsum for block after this one, because
                     * it's not in the buffer. So leave a hint for next time so
                     * we know we need to recalculate */
                    n_Skip = x + n_Context - len;
                    return got_blocks;
                }

                /* If we are moving forward just 1 block, we already have the
                 * following block rsum. If we are skipping both, then
                 * recalculate both */
                if (n_SeqMatches > 1 && blocks_matched == 1)
                    p_CurrentWeakCheckSums.first = p_CurrentWeakCheckSums.second;
                else
                    p_CurrentWeakCheckSums.first = calc_rsum_block(data + x, bs);
                if (n_SeqMatches > 1)
                    p_CurrentWeakCheckSums.second = calc_rsum_block(data + x + bs, bs);
                continue;
            }
        }

        /* Else - advance the window by 1 byte - update the rolling checksum
         * and our offset in the buffer */
        {
            unsigned char Nc = data[x + bs * 2];
            unsigned char nc = data[x + bs];
            unsigned char oc = data[x];
            UPDATE_RSUM(p_CurrentWeakCheckSums.first.a, p_CurrentWeakCheckSums.first.b, oc, nc, n_BlockShift);
            if (n_SeqMatches > 1)
                UPDATE_RSUM(p_CurrentWeakCheckSums.second.a, p_CurrentWeakCheckSums.second.b, nc, Nc, n_BlockShift);
        }
        x++;
    }
}

/* Read the given stream, applying the rsync rolling checksum algorithm to
 * identify any blocks of data in common with the target file. Blocks found are
 * written to our working target output.
 */
qint32 ZsyncWriterPrivate::submitSourceFile(QFile *file) {
    if(!file) {
        return 0;
    }

    qint32 error = 0;
    off_t in = 0;
    /* Allocate buffer of 16 blocks */
    qint32 bufsize = n_BlockSize * 16;
    unsigned char *buf = (unsigned char*)malloc(bufsize + n_Context);
    if (!buf)
        return (error = -1);

    /* Build checksum hash tables ready to analyse the blocks we find */
    if (!p_RsumHash) {
        if (!buildHash()) {
            free(buf);
            return (error = -2);
        }
    }


    p_TransferSpeed.reset(new QElapsedTimer);
    p_TransferSpeed->start();
    while (!file->atEnd()) {
        size_t len;
        off_t start_in = in;

        /* If this is the start, fill the buffer for the first time */
        if (!in) {
            len = file->read((char*)buf, bufsize);
            in += len;
        }

        /* Else, move the last n_Context bytes from the end of the buffer to the
         * start, and refill the rest of the buffer from the stream. */
        else {
            memcpy(buf, buf + (bufsize - n_Context), n_Context);
            in += bufsize - n_Context;
            len = n_Context + file->read((char*)(buf + n_Context), (bufsize - n_Context));
        }

        if (file->atEnd()) {          /* 0 pad to complete a block */
            memset(buf + len, 0, n_Context);
            len += n_Context;
        }

        /* Process the data in the buffer, and report progress */
        submitSourceData( buf, len, start_in);
        {
            qint64 bytesReceived = n_BytesWritten,
                   bytesTotal = n_TargetFileLength;

            int nPercentage = static_cast<int>(
                                  (static_cast<float>
                                   ( bytesReceived ) * 100.0
                                  ) / static_cast<float>
                                  (
                                      bytesTotal
                                  )
                              );

            double nSpeed =  bytesReceived * 1000.0 / p_TransferSpeed->elapsed();
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
        if(b_CancelRequested == true) {
            error = -3;
            b_CancelRequested = false;
            emit canceled();
            break;
        }
    }
    p_TransferSpeed.reset(new QElapsedTimer);
    file->close();
    free(buf);
    return error;
}



/* Build hash tables to quickly lookup a block based on its rsum value.
 * Returns non-zero if successful.
 */
qint32 ZsyncWriterPrivate::buildHash() {
    zs_blockid id;
    qint32 i = 16;

    /* Try hash size of 2^i; step down the value of i until we find a good size
     */
    while ((2 << (i - 1)) > n_Blocks && i > 4) {
        i--;
        QCoreApplication::processEvents();
    }

    /* Allocate hash based on rsum */
    p_HashMask = (2 << i) - 1;
    p_RsumHash = (hash_entry**)calloc(p_HashMask + 1, sizeof *(p_RsumHash));
    if (!p_RsumHash)
        return 0;

    /* Allocate bit-table based on rsum */
    p_BitHashMask = (2 << (i + BITHASHBITS)) - 1;
    p_BitHash = (unsigned char*)calloc(p_BitHashMask + 1, 1);
    if (!p_BitHash) {
        free(p_RsumHash);
        p_RsumHash = NULL;
        return 0;
    }

    /* Now fill in the hash tables.
     * Minor point: We do this in reverse order, because we're adding entries
     * to the hash chains by prepending, so if we iterate over the data in
     * reverse then the resulting hash chains have the blocks in normal order.
     * That's improves our pattern of I/O when writing out identical blocks
     * once we are processing data; we will write them in order. */
    for (id = n_Blocks; id > 0;) {
        /* Decrement the loop variable here, and get the hash entry. */
        hash_entry *e = p_BlockHashes + (--id);

        /* Prepend to linked list for this hash entry */
        unsigned h = calcRHash( e);
        e->next = p_RsumHash[h & p_HashMask];
        p_RsumHash[h & p_HashMask] = e;

        /* And set relevant bit in the p_BitHash to 1 */
        p_BitHash[(h & p_BitHashMask) >> 3] |= 1 << (h & 7);

        QCoreApplication::processEvents();
    }
    return 1;
}

/* Remove the given data block from the rsum hash table, so it won't be
 * returned in a hash lookup again (e.g. because we now have the data)
 */
void ZsyncWriterPrivate::removeBlockFromHash(zs_blockid id) {
    hash_entry *t = &(p_BlockHashes[id]);

    hash_entry **p = &(p_RsumHash[calcRHash(t) & p_HashMask]);

    while (*p != NULL) {
        if (*p == t) {
            if (t == p_Rover) {
                p_Rover = t->next;
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
 * n_Ranges if it is after the last range
 */
qint32 ZsyncWriterPrivate::rangeBeforeBlock(zs_blockid x) {
    /* Lowest number and highest number block that it could be inside (0 based) */
    qint32 min = 0, max = n_Ranges-1;

    /* By bisection */
    for (; min<=max;) {
        /* Range number to compare against */
        qint32 r = (max+min)/2;

        if (x > p_Ranges[2*r+1]) min = r+1;  /* After range r */
        else if (x < p_Ranges[2*r]) max = r-1;/* Before range r */
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
void ZsyncWriterPrivate::addToRanges(zs_blockid x) {
    qint32 r = rangeBeforeBlock(x);

    if (r == -1) {
        /* Already have this block */
    } else {
        /* If between two ranges and exactly filling the hole between them,
         * merge them */
        if (r > 0 && r < n_Ranges
                && p_Ranges[2 * (r - 1) + 1] == x - 1
                && p_Ranges[2 * r] == x + 1) {

            // This block fills the gap between two areas that we have got completely. Merge the adjacent ranges
            p_Ranges[2 * (r - 1) + 1] = p_Ranges[2 * r + 1];
            memmove(&p_Ranges[2 * r], &p_Ranges[2 * r + 2],
                    (n_Ranges - r - 1) * sizeof(p_Ranges[0]) * 2);
            n_Ranges--;
        }

        /* If adjoining a range below, add to it */
        else if (r > 0 && n_Ranges && p_Ranges[2 * (r - 1) + 1] == x - 1) {
            p_Ranges[2 * (r - 1) + 1] = x;
        }

        /* If adjoining a range above, add to it */
        else if (r < n_Ranges && p_Ranges[2 * r] == x + 1) {
            p_Ranges[2 * r] = x;
        }

        else { /* New range for this block alone */
            void *guard = realloc(p_Ranges, (n_Ranges + 1) * 2 * sizeof(p_Ranges[0]));
            if(!guard) {
                return;
            }
            p_Ranges = (zs_blockid*) guard;
            memmove(&p_Ranges[2 * r + 2], &p_Ranges[2 * r],
                    (n_Ranges - r) * 2 * sizeof(p_Ranges[0]));
            p_Ranges[2 * r] = p_Ranges[2 * r + 1] = x;
            n_Ranges++;
        }
    }
}

/* Return true if blockid x of the target file is already known */
qint32 ZsyncWriterPrivate::alreadyGotBlock(zs_blockid x) {
    return (rangeBeforeBlock(x) == -1);
}

/* Returns the blockid of the next block which we already have data for.
 * If we know the requested block, it returns the blockid given; otherwise it
 * will return a later blockid.
 * If no later blocks are known, it returns numblocks (i.e. the block after
 * the end of the file).
 */
zs_blockid ZsyncWriterPrivate::nextKnownBlock(zs_blockid x) {
    qint32 r = rangeBeforeBlock(x);
    if (r == -1)
        return x;
    if (r == n_Ranges) {
        return n_Blocks;
    }
    /* Else return first block of next known range. */
    return p_Ranges[2*r];
}

/* Calculates the rsum hash table hash for the given hash entry. */
unsigned ZsyncWriterPrivate::calcRHash(const hash_entry *const e) {
    unsigned h = e[0].r.b;

    h ^= ((n_SeqMatches > 1) ? e[1].r.b
          : e[0].r.a & p_WeakCheckSumMask) << BITHASHBITS;

    return h;
}

/* Returns the hash entry's blockid. */
zs_blockid ZsyncWriterPrivate::getHashEntryBlockId(const hash_entry *e) {
    return e - p_BlockHashes;
}


/* Writes the block range (inclusive) from the supplied buffer to the given
 * under-construction output file */
void ZsyncWriterPrivate::writeBlocks(const unsigned char *data, zs_blockid bfrom, zs_blockid bto) {
    if(!p_TargetFile->isOpen() || !p_TargetFile->autoRemove())
        return;

    // Equation of length with
    // bto = <original bto> - 1;
    // bfrom = <original bfrom>;
    //
    // len = <original bto> - 1 - bfrom + 1
    //     = <original bto> - bfrom
    //     = actual no. of blocks got.

    off_t len = ((off_t) (bto - bfrom + 1)) << n_BlockShift;
    off_t offset = ((off_t)bfrom) << n_BlockShift;

    auto pos = p_TargetFile->pos();
    p_TargetFile->seek(offset);
    n_BytesWritten += p_TargetFile->write((char*)data, len);
    p_TargetFile->seek(pos);

    {
        /* Having written those blocks, discard them from the rsum hashes (as
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
void ZsyncWriterPrivate::calcMd4Checksum(unsigned char *c, const unsigned char *data, size_t len) {
    p_Md4Ctx->reset();
    p_Md4Ctx->addData((const char*)data, len);
    auto result = p_Md4Ctx->result();
    memmove(c, result.constData(), sizeof(const char) * result.size());
    return;
}
