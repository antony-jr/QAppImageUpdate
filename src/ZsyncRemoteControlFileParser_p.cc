#include <ZsyncRemoteControlFileParser_p.hpp>

using namespace AppImageUpdaterBridgePrivate;

static void doDeleteNetworkManager(QNetworkAccessManager *NetworkManager)
{
	if(NetworkManager != nullptr)
	{
		NetworkManager->deleteLater();
	}
	return;
}

static void doNotDeleteNetworkManager(QNetworkAccessManager *NetworkManager)
{
	(void)NetworkManager;
	return;
}

/*
 * Constructor and Destructor.
*/

ZsyncRemoteControlFileParserPrivate::ZsyncRemoteControlFileParserPrivate(QNetworkAccessManager *NetworkManager)
	: QObject(NetworkManager)
{
	_pLogger = QSharedPointer<QDebug>(new QDebug(&_sLogBuffer));;
	_pNManager = (NetworkManager == nullptr) ? 
		     QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager , doDeleteNetworkManager) :
		     QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager , doNotDeleteNetworkManager);
	connect(_pNManager.data(), SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), 
			this, SLOT(checkNetworkConnection(QNetworkAccessManager::NetworkAccessibility)));	
    connect(this , SIGNAL(error(short)) , this , SLOT(handleError(short)));
	return;
}

ZsyncRemoteControlFileParserPrivate::ZsyncRemoteControlFileParserPrivate
		(const QUrl &controlFileUrl , QNetworkAccessManager *NetworkManager)
	: QObject(NetworkManager)
{
	_pLogger = QSharedPointer<QDebug>(new QDebug(&_sLogBuffer));
	_pNManager = (NetworkManager == nullptr) ? 
		     QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager , doDeleteNetworkManager) :
		     QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager , doNotDeleteNetworkManager);
	connect(_pNManager.data(), SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), 
			this, SLOT(checkNetworkConnection(QNetworkAccessManager::NetworkAccessibility)));	
    connect(this , SIGNAL(error(short)) , this , SLOT(handleError(short)));
    setControlFileUrl(controlFileUrl);
	return;
}

ZsyncRemoteControlFileParserPrivate::~ZsyncRemoteControlFileParserPrivate()
{
	return;
}


/*
 * Public Methods.
*/
void ZsyncRemoteControlFileParserPrivate::setControlFileUrl(const QUrl &controlFileUrl)
{
    _pMutex.lock();
    if(!controlFileUrl.isValid()){
        return;
    }
    _pZsyncHeader.clear();
    _uControlFileUrl = controlFileUrl;
    QNetworkRequest request;
	request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute , true);
	request.setUrl(_uControlFileUrl);
	
	_pTimeoutTimer.setInterval(_nTimeoutTime);
	_pTimeoutTimer.setSingleShot(true);
	connect(&_pTimeoutTimer , SIGNAL(timeout(void)) , 
			this , SLOT(handleTimeout(void)));  

	_pCurrentRequest = request;
	_pCurrentReply = _pNManager->get(_pCurrentRequest);

	_pTimeoutTimer.start();
    connect(_pCurrentReply, SIGNAL(downloadProgress(qint64 , qint64)), 
			this, SLOT(handleControlFileHeader(qint64 , qint64)));

	connect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)) , 
			this ,SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    
    return;
}

void ZsyncRemoteControlFileParserPrivate::setShowLog(bool choose)
{
    QMutexLocker locker(&_pMutex);
    if(choose){
    disconnect(this , SIGNAL(logger(QString)) , 
            this , SLOT(logPrinter(QString)));
    connect(this , SIGNAL(logger(QString)) , 
            this , SLOT(logPrinter(QString)));
    }else{
    disconnect(this , SIGNAL(logger(QString)) , 
            this , SLOT(logPrinter(QString)));
    }
    return;
}

void ZsyncRemoteControlFileParserPrivate::setTimeoutTime(qint64 timeInSeconds)
{
    QMutexLocker locker(&_pMutex);
    _nTimeoutTime = timeInSeconds;
    return;
}

/* 
 * Public Slots.
*/
void ZsyncRemoteControlFileParserPrivate::getTargetFileBlocks(void)
{
	QMutexLocker locker(&_pMutex);
	if(_uControlFileUrl.isEmpty() || _uTargetFileUrl.isEmpty()){
									
		return;
	}

	QNetworkRequest request;
	if(_bSupportForRangeRequests){
	    QByteArray rangeHeaderValue = "bytes="+ QByteArray::number(_nTargetFileBlocksRangeStart) + "-";
       	    rangeHeaderValue += QByteArray::number(_nTargetFileBlocksRangeEnd);
	    request.setRawHeader("Range" , rangeHeaderValue);
	}
	request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute , true);
	request.setUrl(_uControlFileUrl);
	
	_pTimeoutTimer.setInterval(_nTimeoutTime);
	_pTimeoutTimer.setSingleShot(true);

	connect(&_pTimeoutTimer , SIGNAL(timeout(void)) , 
			this , SLOT(handleTimeout(void)));  

	_pCurrentRequest = request;
	_pCurrentReply = _pNManager->get(_pCurrentRequest);

	_pTimeoutTimer.start();
	connect(_pCurrentReply, SIGNAL(finished(void)), 
			this, SLOT(sentAllTargetFileBlocks(void)));

    connect(_pCurrentReply, SIGNAL(downloadProgress(qint64 , qint64)), 
			this, SLOT(sendTargetFileBlocks(qint64 , qint64)));

	connect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)) , 
			this ,SLOT(handleNetworkError(QNetworkReply::NetworkError)));
	return;
}

const size_t &ZsyncRemoteControlFileParserPrivate::getTargetFileBlocksCount(void)
{
	QMutexLocker locker(&_pMutex);
	return _nTargetFileBlocks;
}

const QUrl &ZsyncRemoteControlFileParserPrivate::getControlFileUrl(void)
{
	QMutexLocker locker(&_pMutex);
	return _uControlFileUrl;
}

const QString &ZsyncRemoteControlFileParserPrivate::getZsyncMakeVersion(void)
{
	QMutexLocker locker(&_pMutex);
	return _sZsyncMakeVersion;
}

const QString &ZsyncRemoteControlFileParserPrivate::getTargetFileName(void)
{
	QMutexLocker locker(&_pMutex);
	return _sTargetFileName;
}

const QUrl &ZsyncRemoteControlFileParserPrivate::getTargetFileUrl(void)
{
	QMutexLocker locker(&_pMutex);
	return _uTargetFileUrl;
}

const QString &ZsyncRemoteControlFileParserPrivate::getTargetFileSHA1(void)
{
	QMutexLocker locker(&_pMutex);
	return _sTargetFileSHA1;
}

const QDateTime &ZsyncRemoteControlFileParserPrivate::getMTime(void)
{
	QMutexLocker locker(&_pMutex);
	return _pMTime;
}

const size_t &ZsyncRemoteControlFileParserPrivate::getTargetFileBlockSize(void)
{
	QMutexLocker locker(&_pMutex);
	return _nTargetFileBlockSize;
}

const size_t &ZsyncRemoteControlFileParserPrivate::getTargetFileLength(void)
{
	QMutexLocker locker(&_pMutex);
	return _nTargetFileLength;
}

const qint32 &ZsyncRemoteControlFileParserPrivate::getWeakCheckSumBytes(void)
{
	QMutexLocker locker(&_pMutex);
	return _nWeakCheckSumBytes;
}

const qint32 &ZsyncRemoteControlFileParserPrivate::getStrongCheckSumBytes(void)
{
	QMutexLocker locker(&_pMutex);
	return _nStrongCheckSumBytes;
}

const qint32 &ZsyncRemoteControlFileParserPrivate::getConsecutiveMatchNeeded(void)
{
	QMutexLocker locker(&_pMutex);
	return _nConsecutiveMatchNeeded;
}

/*
 * Private slots.
*/

void ZsyncRemoteControlFileParserPrivate::logPrinter(QString msg)
{
    qDebug().noquote() << "[ " 
                       << QDateTime::currentDateTime().toString(Qt::ISODate)
                       << " ]" 
                       << msg;
    return;
}

void ZsyncRemoteControlFileParserPrivate::handleError(short errorCode)
{
    (void)errorCode;
    _pZsyncHeader.clear();
    if(_pCurrentReply != nullptr){
        _pCurrentReply->abort();
        _pCurrentReply->deleteLater();
        _pCurrentReply = nullptr;
    }
    if(_pMutex.tryLock()){
        _pMutex.unlock();
    }else{
        _pMutex.unlock();
    }
    _bSafeToProceed = false;
    _pTimeoutTimer.stop();
    return;
}

void ZsyncRemoteControlFileParserPrivate::handleNetworkError(QNetworkReply::NetworkError error)
{
    if(error == QNetworkReply::OperationCanceledError){
        return;
    }
     _bSafeToProceed = false;
      disconnect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)) , 
			this ,SLOT(handleNetworkError(QNetworkReply::NetworkError)));
      disconnect(&_pTimeoutTimer , SIGNAL(timeout(void)) , 
			this , SLOT(handleTimeout(void)));
    // emit(error(ERROR_RESPONSE_CODE));
    return;
}

void ZsyncRemoteControlFileParserPrivate::handleTimeout(void)
{
    _pTimeoutTimer.stop();
    disconnect(&_pTimeoutTimer , SIGNAL(timeout(void)) , 
			this , SLOT(handleTimeout(void)));
    emit(error(NETWORK_TIMEOUT));
    return;
}

void ZsyncRemoteControlFileParserPrivate::checkNetworkConnection(QNetworkAccessManager::NetworkAccessibility access)
{
     if (access == QNetworkAccessManager::NotAccessible || access == QNetworkAccessManager::UnknownAccessibility) {
         emit(error(NO_INTERNET_CONNECTION));
    }
    return;
}

void ZsyncRemoteControlFileParserPrivate::handleControlFileHeader(qint64 bytesReceived, qint64 bytesTotal)
{
    _pTimeoutTimer.stop();
    if(!_bSafeToProceed){
        _bSupportForRangeRequests = false;
        if (_pCurrentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 400) {
            _bSafeToProceed = false;
            disconnect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)) , 
			this ,SLOT(handleNetworkError(QNetworkReply::NetworkError)));
            disconnect(&_pTimeoutTimer , SIGNAL(timeout(void)) , 
			this , SLOT(handleTimeout(void)));
            disconnect(_pCurrentReply, SIGNAL(downloadProgress(qint64 , qint64)), 
			this, SLOT(handleControlFileHeader(qint64 , qint64)));
            emit(error(CONTROL_FILE_NOT_FOUND));
            return;
        }else if(_pCurrentReply->hasRawHeader("Accept-Ranges")) {
        QString qstrAcceptRanges = _pCurrentReply->rawHeader("Accept-Ranges");
        _bSupportForRangeRequests = (qstrAcceptRanges.compare("bytes", Qt::CaseInsensitive) == 0);
        }else{
         _bSupportForRangeRequests = false;   
        }
        _bSafeToProceed = true;
        _nTargetFileBlocksRangeEnd = bytesTotal;
    }else{
         int nPercentage =
        static_cast<int>(
            (static_cast<float>
             (
                bytesReceived
             ) * 100.0
            ) / static_cast<float>
            (
                bytesTotal
            )
        );
        emit(progress(nPercentage));
    }
    
    _pZsyncHeader += _pCurrentReply->readAll();
    
    if(_pZsyncHeader.contains("\n\n")){
        if(_pMutex.tryLock()){
            _pMutex.unlock();
        }else{
            _pMutex.unlock();
        }
        
        QMutexLocker locker(&_pMutex);
        
        disconnect(_pCurrentReply, SIGNAL(downloadProgress(qint64 , qint64)), 
			this, SLOT(handleControlFileHeader(qint64 , qint64)));
        _pCurrentReply->abort();
        _pCurrentReply->deleteLater();
        _pCurrentReply = nullptr;
       
        QString ZsyncHeader = QString::fromLatin1(_pZsyncHeader);	
        ZsyncHeader = ZsyncHeader.split("\n\n")[0];
        
        if(_bSupportForRangeRequests){
            _nTargetFileBlocksRangeStart = _pZsyncHeader.size() + 2;
        }
        
        QStringList ZsyncHeaderList = QString(ZsyncHeader).split("\n"); 
        if(ZsyncHeaderList.size() < 8){
            /* error */
        }
        
        _sZsyncMakeVersion = ZsyncHeaderList.at(0).split("zsync: ")[1];
        
        if(_sZsyncMakeVersion == ZsyncHeaderList.at(0)){
            /*
             * error , invalid version. 
            */
            return;
        }
        
        _sTargetFileName = ZsyncHeaderList.at(1).split("Filename: ")[1];
        
        if(_sTargetFileName == ZsyncHeaderList.at(1)){
            /*
             * error , invalid target file name.
            */
            return;
        }
       
        _pMTime = QDateTime::fromString(ZsyncHeaderList.at(2).split("MTime: ")[1] , "ddd, dd MMM yyyy HH:mm:ss +zzz0");
        
        if(!_pMTime.isValid()){
            qDebug() << "Invalid MTime.";
		return;
        }
        
        _nTargetFileBlockSize = (size_t)ZsyncHeaderList.at(3).split("Blocksize: ")[1].toInt();
        
        if(_nTargetFileBlockSize < 1024){
            return;
        }
        
        _nTargetFileLength =  (size_t)ZsyncHeaderList.at(4).split("Length: ")[1].toInt();
        
        if(_nTargetFileLength == 0){
            return;
        }
        
        QString HashLength = ZsyncHeaderList.at(5).split("Hash-Lengths: ")[1];
        QStringList HashLengths = HashLength.split(',');
        if(HashLengths.size() != 3){
            /* invalid hash length line. */
            return;
        }
        
        _nConsecutiveMatchNeeded = HashLengths.at(0).toInt();
        _nWeakCheckSumBytes = HashLengths.at(1).toInt();
        _nStrongCheckSumBytes = HashLengths.at(2).toInt();
        
        if(_nWeakCheckSumBytes < 1 || _nWeakCheckSumBytes > 4
            || _nStrongCheckSumBytes < 3 || _nStrongCheckSumBytes > 16
            || _nConsecutiveMatchNeeded > 2 || _nConsecutiveMatchNeeded < 1)
        {
            return;
        }
        
        _uTargetFileUrl = QUrl(ZsyncHeaderList.at(6).split("URL: ")[1]);
        if(!_uTargetFileUrl.isValid()){
            return;
        }
        _sTargetFileSHA1 = ZsyncHeaderList.at(7).split("SHA-1: ")[1];
        if(_sTargetFileSHA1 == ZsyncHeaderList.at(7)){
            return;
        }
        
    }
    
    _pTimeoutTimer.setInterval(_nTimeoutTime);
    _pTimeoutTimer.start();
    return;
}

void ZsyncRemoteControlFileParserPrivate::sendTargetFileBlocks(qint64 bytesReceived , qint64 bytesTotal)
{
    _pTimeoutTimer.stop();
    if(!_bSafeToProceed){
        if (_pCurrentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 400) {
            _bSafeToProceed = false;
            disconnect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)) , 
			this ,SLOT(handleNetworkError(QNetworkReply::NetworkError)));
            disconnect(&_pTimeoutTimer , SIGNAL(timeout(void)) , 
			this , SLOT(handleTimeout(void)));
            disconnect(_pCurrentReply, SIGNAL(downloadProgress(qint64 , qint64)), 
			this, SLOT(sendTargetFileBlocks(qint64 , qint64)));
            emit(error(CANNOT_PROCESS_TARGET_BLOCKS));
            return;
        }
        _bSafeToProceed = true;
    }else{
         int nPercentage =
        static_cast<int>(
            (static_cast<float>
             (
                _nTargetFileBlocksRangeStart + bytesReceived
             ) * 100.0
            ) / static_cast<float>
            (
                _nTargetFileBlocksRangeEnd
            )
        );
        emit(progress(nPercentage));
    }
    
    if(_pBlockSumBuffer.size() < _nWeakCheckSumBytes + _nStrongCheckSumBytes){
        _pBlockSumBuffer += _pCurrentReply->readAll();
    }else{
        QDataStream stream(_pBlockSumBuffer);
        qint32 gotTargetBlocks = _pBlockSumBuffer.size() / (_nWeakCheckSumBytes + _nStrongCheckSumBytes);
        qint32 cutRight = gotTargetBlocks;
        gotTargetBlocks += _nCurrentBlockId;
        while(_nCurrentBlockId < gotTargetBlocks){
        rsum r = { 0, 0 };
        unsigned char checksum[16];

        /* Read in */
        if (stream.readRawData(((char *)&r) + 4 - _nWeakCheckSumBytes, _nWeakCheckSumBytes) < 1
            || stream.readRawData((char *)&checksum, _nStrongCheckSumBytes) < 1) {
            return;
        }

        /* Convert to host endian and store */
        if(Q_BYTE_ORDER == Q_LITTLE_ENDIAN){
        r.a = qFromBigEndian(r.a);
        r.b = qFromBigEndian(r.b);
        }
        emit(handleTargetFileBlocks(_nCurrentBlockId , r , checksum));
        ++_nCurrentBlockId;
        }
        _pBlockSumBuffer = _pBlockSumBuffer.right(cutRight);
    }
    
    _pTimeoutTimer.setInterval(_nTimeoutTime);
    _pTimeoutTimer.start();
    return;
}

void ZsyncRemoteControlFileParserPrivate::sentAllTargetFileBlocks(void)
{
    _pCurrentReply->deleteLater();
    _pCurrentReply = nullptr;
    
    emit(gotAllTargetFileBlocks());
    return;
}
