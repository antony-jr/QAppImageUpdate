#include <ZsyncWriter_p.hpp>

using namespace AppImageUpdaterBridge;

ZsyncWriterPrivate::ZsyncWriterPrivate(void)
	: QObject()
{
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
	_pResults->clear();
	_sSourceFilePath.clear();
    _pZsyncCoreJobInfos.clear();
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

void ZsyncWriterPrivate::setConfiguration(size_t blocksize , size_t nblocks , qint32 weakCheckSumBytes , qint32 strongCheckSumBytes,
                                          qint32 seqMatches , qint32 targetFileLength , const QString &seedFilePath ,
					  const QString &targetFileName,
                                          const QVector<ZsyncCoreJobPrivate::Information> &infos)
{
	if(!_pWatcher->isCanceled() && !_pWatcher->isFinished()){
		if(_pWatcher->isRunning() || _pWatcher->isStarted() || _pWatcher->isPaused()){
			return;
		}
	}
	isEmpty = false;
	_nBlockSize = blocksize;
	_nBlocks = nblocks;
	_nBlockShift = (blocksize == 1024) ? 10 : (blocksize == 2048) ? 11 : log2(blocksize);
	_nWeakCheckSumBytes = weakCheckSumBytes;
	_nStrongCheckSumBytes = strongCheckSumBytes;
	_nSeqMatches = seqMatches;
	_nTargetFileLength = targetFileLength;
	_sSourceFilePath = seedFilePath;
	_sTargetFileName = targetFileName;
    	_pZsyncCoreJobInfos = infos;
    
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

ZsyncCoreJobPrivate::Result ZsyncWriterPrivate::startJob(const ZsyncCoreJobPrivate::Information &info)
{
	auto z = ZsyncCoreJobPrivate(info);
    return z();
}

void ZsyncWriterPrivate::start(void)
{
	_pWatcher->setFuture(QtConcurrent::mapped(_pZsyncCoreJobInfos, &ZsyncWriterPrivate::startJob));
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
		//_pTargetFile->rename(_sTargetFileName);
		_pTargetFile->close();
		emit finished(false);
		return;
	}
	emit finished(true);
	return;
}
