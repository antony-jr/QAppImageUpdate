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

ZsyncWriterPrivate::ZsyncWriterPrivate(void)
	: QObject()
{
	emit statusChanged(INITIALIZING);
#ifndef LOGGING_DISABLED
	_pLogger.reset(new QDebug(&_sLogBuffer));
#endif // LOGGING_DISABLED
	_pResults.reset(new QVector<QPair<QPair<zs_blockid , zs_blockid>, QVector<QByteArray>>>);
	_pWatcher = new QFutureWatcher<ZsyncCoreJobPrivate::Result>(this);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::started,
		this , &ZsyncWriterPrivate::started);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::canceled,
		this , &ZsyncWriterPrivate::canceled);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::paused,
		this , &ZsyncWriterPrivate::paused);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::resumed,
		this , &ZsyncWriterPrivate::resumed);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::finished,
		this , &ZsyncWriterPrivate::handleFinished);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::progressValueChanged,
		this , &ZsyncWriterPrivate::progress , Qt::DirectConnection);
	emit statusChanged(IDLE);
	return;
}

ZsyncWriterPrivate::~ZsyncWriterPrivate()
{
	if(!_pWatcher){
		return;
	}

	if(!_pWatcher->isCanceled() && !_pWatcher->isFinished()){
		if(_pWatcher->isPaused()){
			_pWatcher->resume();
		}
		if(_pWatcher->isRunning() || _pWatcher->isStarted()){
			_pWatcher->cancel();
			_pWatcher->waitForFinished();
		}
	}
	return;
}

void ZsyncWriterPrivate::clear(void)
{
	if(!_pWatcher->isCanceled() && !_pWatcher->isFinished()){
		if(_pWatcher->isRunning() || _pWatcher->isStarted() || _pWatcher->isPaused()){
			return;
		}
	}else if(isEmpty){
		return;
	}
	 isEmpty = true;
	_nBlockSize = _nBlocks = 0;
	_nBlockShift = _nWeakCheckSumBytes = _nStrongCheckSumBytes = _nSeqMatches = 0;
	_pResults->clear();
	_sSourceFilePath.clear();
    _pZsyncCoreJobInfos.clear();
	_sTargetFileName.clear();
	_sOutputDirectory.clear();

	INFO_START " clear : flushed everything." INFO_END;
	return;
}

#ifndef LOGGING_DISABLED
void ZsyncWriterPrivate::setLoggerName(const QString &name)
{
	_sLoggerName = QString(name);
	return;
}

void ZsyncWriterPrivate::setShowLog(bool logNeeded)
{
    if(logNeeded) {
        disconnect(this, &ZsyncWriterPrivate::logger, this, &ZsyncWriterPrivate::handleLogMessage);
        connect(this, &ZsyncWriterPrivate::logger, this, &ZsyncWriterPrivate::handleLogMessage);
        INFO_START  " setShowLog : true  , started logging." INFO_END;

    } else {
        INFO_START  " setShowLog : false , finishing logging." INFO_END;
        disconnect(this, &ZsyncWriterPrivate::logger, this, &ZsyncWriterPrivate::handleLogMessage);
    }
    return;
}

void ZsyncWriterPrivate::handleLogMessage(QString msg , QString path)
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

void ZsyncWriterPrivate::getBlockRanges(void)
{
	if(!_pResults || !_nBlockShift){
		return;
	}

	INFO_START " getBlockRanges : emitting required block ranges." INFO_END;
	emit statusChanged(EMITTING_REQUIRED_BLOCK_RANGES);
	for(auto iter = _pResults->constBegin(),
		 end = _pResults->constEnd();
		 iter != end ; 
		 iter++)
	{
		QPair<qint32 , qint32> range;
		range.first = (*iter).first.first << _nBlockShift;
		range.second = (*iter).first.second << _nBlockShift;
		emit blockRange(range);
		QCoreApplication::processEvents();
	}

	emit statusChanged(IDLE);
	emit endOfBlockRanges();
	INFO_START " getBlockRanges : emitted required block ranges." INFO_END;
	return;
}

void ZsyncWriterPrivate::writeBlockRanges(const QPair<qint32 , qint32> &range , QByteArray *data)
{
	if(!_pResults || !_nBlockShift || !_pTargetFile){
		return;
	}

	INFO_START " writeBlockRanges : writting given downloaded blocks." INFO_END;
	emit statusChanged(WRITTING_DOWNLOADED_BLOCK_RANGES);
	QCryptographicHash ctx(QCryptographicHash::Md4);
	QBuffer buffer(data);
	buffer.open(QIODevice::ReadOnly);

	bool fullWrite = true;
	auto from = range.first >> _nBlockShift;
	auto to = range.second >> _nBlockShift;
	auto endb = to - from;
		
	emit statusChanged(SEARCHING_FOR_RANGE_CHECKSUMS);
	for(auto iter = _pResults->begin(),
		 end = _pResults->end();
		 iter != end ; 
		 ++iter)
	{
		if((*iter).first.first == from && (*iter).first.second == to){
			emit statusChanged(CHECKING_CHECKSUMS_FOR_DOWNLOADED_BLOCK_RANGES);
			auto Md4Vector = (*iter).second;
			INFO_START " writeBlockRanges : comparing MD4 Checksums for range(" LOGR range LOGR ")." INFO_END;
			for(auto x = 0; x < endb; ++x){
				QByteArray currentBlock = buffer.read(_nBlockSize),
					   currentBlockMd4Sum;
				ctx.addData(currentBlock);
			        currentBlockMd4Sum = ctx.result();
				currentBlockMd4Sum.resize(Md4Vector.at(x).size());
				ctx.reset();
				qDebug() << "currentBlockMd4Sum:: " << currentBlockMd4Sum.toHex() <<
					 " , Md4Vector:: " << Md4Vector.at(x).toHex();
		       		if(currentBlockMd4Sum != Md4Vector.at(x)){
				WARNING_START " writeBlockRanges : MD4 Checksum mismatch , only writting good blocks." WARNING_END;
				emit statusChanged(WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE);
				fullWrite = false;
				if(x){
				_pTargetFile->seek(range.first);
				_pTargetFile->write(data->constData() , x);
				emit blockRangesWritten(range , false);
				}
				break;
				}
				QCoreApplication::processEvents();
			}
			if(fullWrite){
				INFO_START " writeBlockRanges : MD4 Checksum matched! writting all blocks." WARNING_END;
				emit statusChanged(WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE);
				_pTargetFile->seek(range.first);
				_pTargetFile->write(*data);
				_pResults->erase(iter);
				emit blockRangesWritten(range , true);
				/*
				 * If we have no remaining block ranges to download then try constructing
				 * the target file.
				*/
				if(_pResults->isEmpty()){
					INFO_START " writeBlockRanges : no more ranges to download , trying to construct file." INFO_END;
					bool constructed = verifyAndConstructTargetFile();
					if(constructed){
						emit finished(!constructed);
					}
				}

			}
			emit statusChanged(IDLE);
			return;
		}
		QCoreApplication::processEvents();
	}
	emit statusChanged(IDLE);
	return;
}

void ZsyncWriterPrivate::setConfiguration(size_t blocksize , size_t nblocks , qint32 weakCheckSumBytes , qint32 strongCheckSumBytes,
                                          qint32 seqMatches , qint32 targetFileLength , const QString &seedFilePath ,
					  const QString &targetFileName, const QString &targetFileSHA1,
                                          const QVector<ZsyncCoreJobPrivate::Information> &infos)
{
	if(!_pWatcher->isCanceled() && !_pWatcher->isFinished()){
		if(_pWatcher->isRunning() || _pWatcher->isStarted() || _pWatcher->isPaused()){
			return;
		}
	}

	clear();
	isEmpty = false;
	_nBlockSize = blocksize;
	_nBlocks = nblocks;
	_nBlockShift = (blocksize == 1024) ? 10 : (blocksize == 2048) ? 11 : log2(blocksize);
	_nWeakCheckSumBytes = weakCheckSumBytes;
	_nStrongCheckSumBytes = strongCheckSumBytes;
	_nSeqMatches = seqMatches;
	_nTargetFileLength = targetFileLength;
	_sSourceFilePath = seedFilePath;
	_sTargetFileSHA1 = targetFileSHA1;
	_sTargetFileName = targetFileName;
    	_pZsyncCoreJobInfos = infos;
   
    INFO_START " setConfiguration : creating temporary file." INFO_END;	
    auto path = (_sOutputDirectory.isEmpty()) ? QFileInfo(_sSourceFilePath).path() : _sOutputDirectory;
    path = (path == "." ) ? QDir::currentPath() : path;
    auto targetFilePath = path + "/XXXXXXXXXX.AppImage.part";

    QFileInfo perm(path);
    if(!perm.isWritable() || !perm.isReadable()){
	emit error(NO_PERMISSION_TO_READ_WRITE_TARGET_FILE);
	return;
    }
    
    _pTargetFile = new QTemporaryFile(targetFilePath , this);
    if(!_pTargetFile->open()){
	emit error(CANNOT_OPEN_TARGET_FILE);
	return;
    }
    /*
     * To open the target file we have to 
     * request fileName() from the temporary file.
     */
    (void)_pTargetFile->fileName();
    INFO_START " setConfiguration : temporary file will temporarily reside at " LOGR _pTargetFile->fileName() LOGR "." INFO_END; 
    for(auto begin = _pZsyncCoreJobInfos.begin(),
             end = _pZsyncCoreJobInfos.end();
             begin != end;
             ++begin)
    {
            (*begin).targetFile = (QFile*)_pTargetFile;
	    QCoreApplication::processEvents();
    }

    emit finishedConfiguring();
   return;
}

bool ZsyncWriterPrivate::isPaused  (void) const
{
	if(_pWatcher->isCanceled()){
		return false;
	}
	return _pWatcher->isPaused();
}

bool ZsyncWriterPrivate::isStarted (void) const
{
	if(_pWatcher->isCanceled()){
		return false;
	}
	return _pWatcher->isStarted();
}

bool ZsyncWriterPrivate::isRunning (void) const
{
	if(_pWatcher->isCanceled()){
		return false;
	}
	return _pWatcher->isRunning();
}

bool ZsyncWriterPrivate::isCanceled(void) const
{
	return _pWatcher->isCanceled();
}

bool ZsyncWriterPrivate::isFinished(void) const
{
	if(_pWatcher->isCanceled()){
		return false;
	}
	return _pWatcher->isFinished();
}

ZsyncCoreJobPrivate::Result ZsyncWriterPrivate::startJob(const ZsyncCoreJobPrivate::Information &info)
{
    ZsyncCoreJobPrivate z(info);
    return z();
}

void ZsyncWriterPrivate::start(void)
{
	INFO_START " start : starting delta writer." INFO_END;
	_pWatcher->setFuture(QtConcurrent::mapped(_pZsyncCoreJobInfos, &ZsyncWriterPrivate::startJob));
	return;
}

void ZsyncWriterPrivate::cancel(void)
{
	if(_pWatcher->isCanceled()){
		return;
	}else if(_pWatcher->isFinished()){
		return;
	}else{
	INFO_START " cancel : requesting cancel on delta writer." INFO_END;
	_pWatcher->cancel();
	}
	return;
}

void ZsyncWriterPrivate::pause(void)
{
	if(_pWatcher->isCanceled()){
		return;
	}else if(_pWatcher->isFinished()){
		return;
	}else if(_pWatcher->isPaused()){
		return;
	}else{
	INFO_START " pause : requesting pause on delta writer." INFO_END;
	_pWatcher->pause();
	}	
	return;
}

void ZsyncWriterPrivate::resume(void)
{
	if(_pWatcher->isCanceled()){
		return;
	}else if(_pWatcher->isFinished()){
		return;
	}else if(_pWatcher->isStarted() || _pWatcher->isRunning() || !_pWatcher->isPaused()){
		return;
	}else{
	INFO_START " resume : requesting resume on delta writer." INFO_END;
	_pWatcher->resume();
	}
	return;
}


void ZsyncWriterPrivate::handleFinished(void)
{
	emit statusChanged(ANALYZING_RESULTS_FOR_CONSTRUCTING_TARGET_FILE);
	auto results = (_pWatcher->future()).results();
	qint32 gotBlocks = 0;
	for(auto iter = results.begin() ,
		 end = results.end();
		 iter != end;
		 ++iter){
		if((*iter).errorCode > 0){
			emit error((*iter).errorCode);
			return;
		}
		gotBlocks += (*iter).gotBlocks;
		if((*iter).requiredRanges != nullptr)
		{
			_pResults->append(*((*iter).requiredRanges));
			delete (*iter).requiredRanges;
		}

	}
	emit statusChanged(IDLE);

	if(gotBlocks >= _nBlocks){
		INFO_START " handleFinished : got all blocks from delta writer." INFO_END;
		INFO_START " handleFinished : trying to construct target file." INFO_END;
		bool constructed = verifyAndConstructTargetFile();
		emit finished(!constructed);
		return;
	}

	INFO_START " handleFinished : did not get all the blocks from delta writer." INFO_END;
	INFO_START " handleFinished : halting delta writer , waiting for remaining blocks to be submitted." INFO_END;
	emit finished(true);
	return;
}

bool ZsyncWriterPrivate::verifyAndConstructTargetFile(void)
{
	bool constructed = false;
	QString UnderConstructionFileSHA1;
	qint64 bufferSize = 0;
	QCryptographicHash *SHA1Hasher = new QCryptographicHash(QCryptographicHash::Sha1);

	_pTargetFile->resize(_nTargetFileLength);
	_pTargetFile->seek(0);

	INFO_START " verifyAndConstructTargetFile : calculating sha1 hash on temporary target file. " INFO_END;
	emit statusChanged(CALCULATING_TARGET_FILE_SHA1_HASH);
    	if(_nTargetFileLength >= 1073741824){ // 1 GiB and more.
			bufferSize = 104857600; // copy per 100 MiB.
    	}
    	else if(_nTargetFileLength >= 1048576 ){ // 1 MiB and more.
			bufferSize = 1048576; // copy per 1 MiB.
    	}else if(_nTargetFileLength  >= 1024){ // 1 KiB and more.
			bufferSize = 4096; // copy per 4 KiB.
    	}else{ // less than 1 KiB
			bufferSize = 1024; // copy per 1 KiB.
    	}

    	while(!_pTargetFile->atEnd()){
		SHA1Hasher->addData(_pTargetFile->read(bufferSize));
		QCoreApplication::processEvents();
    	}
    	UnderConstructionFileSHA1 = QString(SHA1Hasher->result().toHex().toUpper());	
    	delete SHA1Hasher; 

	INFO_START " verifyAndConstructTargetFile : comparing temporary target file sha1 hash(" LOGR UnderConstructionFileSHA1
		   LOGR ") and remote target file sha1 hash(" LOGR _sTargetFileSHA1 INFO_END;

	if(UnderConstructionFileSHA1 == _sTargetFileSHA1)
	{
		INFO_START " verifyAndConstructTargetFile : sha1 hash matches!" INFO_END;
		emit statusChanged(CONSTRUCTING_TARGET_FILE);
		constructed = true;
		/*
		 * Rename old files with the same 
		 * name.
		 *
		 * Note: Since we checked for permissions earlier
		 * , We don't need to verify it again.
		*/
		{
		QFile oldFile(QFileInfo(_pTargetFile->fileName()).path() + "/" + _sTargetFileName);
		if(oldFile.exists()){
			INFO_START " verifyAndConstructTargetFile : file with target file name exists , renaming it." INFO_END;
			oldFile.rename(_sTargetFileName + ".old-version");
		}
		}
		/*
		 * Construct the file.
		*/
		_pTargetFile->setAutoRemove(false);
		_pTargetFile->resize(_nTargetFileLength);
		_pTargetFile->rename(_sTargetFileName);
		_pTargetFile->close();
	}else{
		FATAL_START " verifyAndConstructTargetFile : sha1 hash mismatch." FATAL_END;
		emit statusChanged(IDLE);
		emit error(TARGET_FILE_SHA1_HASH_MISMATCH);
		return false;
	}
	emit statusChanged(IDLE);
	return constructed;
}

QString ZsyncWriterPrivate::errorCodeToString(short errorCode)
{
	QString ret = "AppImageDeltaWriter::errorCode(";
	switch (errorCode) {
		case HASH_TABLE_NOT_ALLOCATED:
			ret += "HASH_TABLE_NOT_ALLOCATED)";
			break;
		case INVALID_TARGET_FILE_CHECKSUM_BLOCKS:
			ret += "INVALID_TARGET_FILE_CHECKSUM_BLOCKS)";
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
	QString ret = "AppImageDeltaWriter::statusCode(";
	switch (statusCode){
		case WRITTING_DOWNLOADED_BLOCK_RANGES:
			ret += "WRITTING_DOWNLOADED_BLOCK_RANGES)";
			break;
		case EMITTING_REQUIRED_BLOCK_RANGES:
			ret += "EMITTING_REQUIRED_BLOCK_RANGES)";
			break;
		case SEARCHING_FOR_RANGE_CHECKSUMS:
			ret += "SEARCHING_FOR_RANGE_CHECKSUMS)";
			break;
		case CHECKING_CHECKSUMS_FOR_DOWNLOADED_BLOCK_RANGES:
			ret += "CHECKING_CHECKSUMS_FOR_DOWNLOADED_BLOCK_RANGES)";
			break;
		case WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE:
			ret += "WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE)";
			break;
		case ANALYZING_RESULTS_FOR_CONSTRUCTING_TARGET_FILE:
			ret += "ANALYZING_RESULTS_FOR_CONSTRUCTING_TARGET_FILE)";
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
