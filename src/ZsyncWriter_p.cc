#include <ZsyncWriter_p.hpp>

using namespace AppImageUpdaterBridge;

ZsyncWriterPrivate::ZsyncWriterPrivate(void)
	: QObject()
{
	_pZsyncCoreJobInfos.reset(new QList<ZsyncCoreJobInfo>);
	_pResults.reset(new QVector<QPair<QPair<zs_blockid , zs_blockid>, QVector<QByteArray>>>);
	_pWatcher = new QFutureWatcher<ZsyncCoreJobPrivate::Result>(this);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::started,
		this , &ZsyncWriterPrivate::started);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::paused,
		this , &ZsyncWriterPrivate::paused);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::resumed,
		this , &ZsyncWriterPrivate::resumed);
	connect(_pWatcher , &QFutureWatcher<ZsyncCoreJobPrivate::Result>::finished,
		this , &ZsyncWriterPrivate::handleFinished);
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
	_pZsyncCoreJobInfos->clear();
	_pResults->clear();
	_sSourceFilePath.clear();
	_sTargetFileName.clear();
	_sOutputDirectory.clear();
	return;
}

void ZsyncWriterPrivate::getBlockRanges(void)
{
	if(!_pResults || !_nBlockShift){
		return;
	}

	for(auto iter = _pResults->constBegin(),
		 end = _pResults->constEnd();
		 iter != end ; 
		 iter++)
	{
		QSharedPointer<QPair<qint32 , qint32>> range;
		range->first = (*iter).first.first << _nBlockShift;
		range->second = (*iter).first.second << _nBlockShift;
		emit blockRange(range);
		QCoreApplication::processEvents();
	}
	emit endOfBlockRanges();
	return;
}

void ZsyncWriterPrivate::writeBlockRanges(const QPair<qint32 , qint32> &range , QByteArray *data)
{
	if(!_pResults || !_nBlockShift || !_pTargetFile){
		return;
	}

	QCryptographicHash ctx(QCryptographicHash::Md4);
	QBuffer buffer(data);
	buffer.open(QIODevice::ReadOnly);

	bool fullWrite = true;
	auto from = range.first >> _nBlockShift;
	auto to = range.second >> _nBlockShift;
	auto endb = to - from;
		
	for(auto iter = _pResults->begin(),
		 end = _pResults->end();
		 iter != end ; 
		 ++iter)
	{
		if((*iter).first.first == from && (*iter).first.second == to){
			auto Md4Vector = (*iter).second;
			for(auto x = from - from; x <= endb; ++x){	
				QByteArray currentBlock = buffer.read(_nBlockSize),
					   currentBlockMd4Sum;
				ctx.addData(currentBlock);
			        currentBlockMd4Sum = ctx.result();
				currentBlockMd4Sum.resize(Md4Vector.at(x).size());
				ctx.reset();
		       		if(currentBlockMd4Sum != Md4Vector.at(x)){
					fullWrite = false;
					/*
					 * Write any good blocks.
					 */
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
				_pTargetFile->seek(range.first);
				_pTargetFile->write(*data);
				_pResults->erase(iter);
				if(_pResults->isEmpty()){
				_pTargetFile->setAutoRemove(false);
				_pTargetFile->resize(_nTargetFileLength);
				_pTargetFile->rename(_sTargetFileName);
				_pTargetFile->close();
				}
				emit blockRangesWritten(range , true);
			}
			return;
		}
		QCoreApplication::processEvents();
	}
	return;
}

void ZsyncWriterPrivate::setConfiguration(size_t blocksize , size_t nblocks , qint32 blockshift, 
			      qint32 weakCheckSumBytes , qint32 strongCheckSumBytes , qint32 seqMatches, qint32 targetFileLength , 
			      QBuffer *checkSumBlocks , const QString &seedFilePath , const QString&targetFileName ,
			      const QString &targetFileOutputDirectory)
{
	if(!_pWatcher->isCanceled() && !_pWatcher->isFinished()){
		if(_pWatcher->isRunning() || _pWatcher->isStarted() || _pWatcher->isPaused()){
			return;
		}
	}
	int firstThreadBlocksToDo = 0,
	    otherThreadsBlocksToDo = 0,
	    bytesRead = 0;
	auto mod = (int)_nBlocks % QThread::idealThreadCount();
	QBuffer *partition = nullptr; 
	char *readBuffer = new char[(firstThreadBlocksToDo * (_nStrongCheckSumBytes + _nWeakCheckSumBytes)) + 1];
	memset(readBuffer , 0 , (firstThreadBlocksToDo * (_nStrongCheckSumBytes + _nWeakCheckSumBytes))+1);

	isEmpty = false;
	_nBlockSize = blocksize;
	_nBlocks = nblocks;
	_nBlockShift = blockshift;
	_nWeakCheckSumBytes = weakCheckSumBytes;
	_nStrongCheckSumBytes = strongCheckSumBytes;
	_nSeqMatches = seqMatches;
	_nTargetFileLength = targetFileLength;
	_sSourceFilePath = seedFilePath;
	_sTargetFileName = targetFileName;
	_sOutputDirectory = targetFileOutputDirectory;

    if(!checkSumBlocks ||
       checkSumBlocks->size() < (_nWeakCheckSumBytes + _nStrongCheckSumBytes)) {
	   clear();
	   emit error(INVALID_TARGET_FILE_CHECKSUM_BLOCKS);
	   return;
    }else
    if(!checkSumBlocks->isOpen() || !checkSumBlocks->isReadable()){
	    checkSumBlocks->close();
	    checkSumBlocks->open(QIODevice::ReadOnly);
    	    checkSumBlocks->seek(0);
    }else{
	    checkSumBlocks->seek(0);
    }
   
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
 

    if(mod > 0){
	firstThreadBlocksToDo = (int)(((int)_nBlocks - mod) / QThread::idealThreadCount()) + mod;
	otherThreadsBlocksToDo = firstThreadBlocksToDo - mod;
    }else{ // mod == 0
	firstThreadBlocksToDo = (int)((int)_nBlocks / QThread::idealThreadCount());
	otherThreadsBlocksToDo = firstThreadBlocksToDo;
    }
    partition = new QBuffer;
    partition->open(QIODevice::WriteOnly);
    bytesRead = checkSumBlocks->read(readBuffer , firstThreadBlocksToDo * (_nWeakCheckSumBytes + _nStrongCheckSumBytes)); 
    partition->write(readBuffer , bytesRead);
    memset(readBuffer , 0 , bytesRead);
    partition->close();
    
    _pZsyncCoreJobInfos->append(ZsyncCoreJobInfo(_nBlocks , 0 , firstThreadBlocksToDo ,
		    	  	_nWeakCheckSumBytes , _nStrongCheckSumBytes , _nSeqMatches , 
			  	partition , _pTargetFile , _sSourceFilePath));

    if(otherThreadsBlocksToDo && QThread::idealThreadCount() > 1){
	int threadCount = 2;
	while(threadCount <= QThread::idealThreadCount()){
	auto fromId = firstThreadBlocksToDo * (threadCount - 1);
	partition = new QBuffer;
	partition->open(QIODevice::WriteOnly);
    	bytesRead = checkSumBlocks->read(readBuffer , otherThreadsBlocksToDo * (_nWeakCheckSumBytes + _nStrongCheckSumBytes));
	partition->write(readBuffer , bytesRead);
	memset(readBuffer , 0 , bytesRead);
	partition->close();
  	_pZsyncCoreJobInfos->append(ZsyncCoreJobInfo(_nBlocks , fromId , otherThreadsBlocksToDo ,
		    	  	    		     _nWeakCheckSumBytes , _nStrongCheckSumBytes , _nSeqMatches , 
			  			     partition , _pTargetFile , _sSourceFilePath));
	++threadCount;
	QCoreApplication::processEvents();
	}
   }

   checkSumBlocks->close();
   delete checkSumBlocks;
   delete readBuffer;
   return;
}

bool ZsyncWriterPrivate::isPaused  (void) const
{
	return _pWatcher->isPaused();
}

bool ZsyncWriterPrivate::isStarted (void) const
{
	return _pWatcher->isStarted();
}

bool ZsyncWriterPrivate::isRunning (void) const
{
	return _pWatcher->isRunning();
}

bool ZsyncWriterPrivate::isCanceled(void) const
{
	return _pWatcher->isCanceled();
}

bool ZsyncWriterPrivate::isFinished(void) const
{
	return _pWatcher->isFinished();
}

ZsyncCoreJobPrivate::Result ZsyncWriterPrivate::startJob(const ZsyncWriterPrivate::ZsyncCoreJobInfo &info)
{
	return ZsyncCoreJobPrivate(info.blockSize, info.blockIdOffset , info.nblocks , 
				  info.weakCheckSumBytes , info.strongCheckSumBytes ,
				  info.seqMatches , info.checkSumBlocks , info.targetFile ,
				  info.seedFilePath)();
}


void ZsyncWriterPrivate::start(void)
{
	_pWatcher->setFuture(QtConcurrent::mapped(*(_pZsyncCoreJobInfos.data()) , &ZsyncWriterPrivate::startJob));
	return;
}

void ZsyncWriterPrivate::handleFinished(void)
{
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

	if(gotBlocks >= _nBlocks){
		/*
		 * Construct the file.
		*/
		_pTargetFile->setAutoRemove(false);
		_pTargetFile->resize(_nTargetFileLength);
		_pTargetFile->rename(_sTargetFileName);
		_pTargetFile->close();
		emit finished(false);
		return;
	}
	emit finished(true);
	return;
}
