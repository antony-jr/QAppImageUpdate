#include <ZsyncRemoteControlFileParser_p.hpp>

using namespace AppImageUpdaterBridgePrivate;

/*
 * Prints to the log.
 * LOGS,LOGE  -> Prints normal log messages.
 * INFO_START,INFO_END -> Prints info messages to log.
 * WARNING_START,WARNING_END -> Prints warning messages to log.
 * FATAL_START,FATAL_END -> Prints fatal messages to log.
 *
 * Example:
 *      LOGS "This is a log message." LOGE
 *
 *
*/
#ifdef LOGGING_ENABLED
#define LOGS *(_pLogger.data()) LOGR
#define LOGR <<
#define LOGE ; \
             emit(logger(_sLogBuffer)); \
             _sLogBuffer.clear();
#else
#define LOGS (void)
#define LOGR ;(void)
#define LOGE ;
#endif // LOGGING_ENABLED
#define INFO_START LOGS "   INFO: "
#define INFO_END LOGE

#define WARNING_START LOGS "WARNING: "
#define WARNING_END LOGE

#define FATAL_START LOGS "  FATAL: "
#define FATAL_END LOGE

/*
 * Since this code is repeated in the constructed
 * we can use this macro to reduce lines of code
 * in the source and thus avoid repeated code.
 * 
 * Warning:
 * 	Hard coded macro , do not use this outside
 * 	ZsyncRemoteControlFileParserPrivate.
 *
 * Example:
 * 	CONSTRUCT();
*/
#ifdef LOGGING_ENABLED
#define CONSTRUCT() _pLogger = QSharedPointer<QDebug>(new QDebug(&_sLogBuffer));\
    		    _pNManager = (NetworkManager == nullptr) ? \
                    QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager, doDeleteNetworkManager): \
                    QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager, doNotDeleteNetworkManager); \
    		    connect(this, SIGNAL(error(short)), this, SLOT(handleErrorSignal(short)));
#else
#define CONSTRUCT() _pNManager = (NetworkManager == nullptr) ? \
                    QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager, doDeleteNetworkManager): \
                    QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager, doNotDeleteNetworkManager); \
    		    connect(this, SIGNAL(error(short)), this, SLOT(handleErrorSignal(short)));
#endif // LOGGING_ENABLED

/*
 * Static functions to deallocate QNetworkAccessManager ,
 * Made to be used for smart pointers.
*/
static void doDeleteNetworkManager(QNetworkAccessManager *NetworkManager)
{
    NetworkManager->deleteLater();
    return;
}

/*
 * Dummy deallocator for QNetworkAccessManager , 
 * Made to be used for smart pointers where we don't want to delete
 * the QNetworkAccessManager.
*/
static void doNotDeleteNetworkManager(QNetworkAccessManager *NetworkManager)
{
    (void)NetworkManager;
    return;
}


/*
 * ZsyncRemoteControlFileParserPrivate is the private class to handle all things
 * related to Zsync Control File. This class must be used privately.
 * This class caches the Zsync Control File in a QBuffer , So when its needed ,We 
 * can quickly seek and start feeding in the checksum blocks to ZsyncCore.
 * This class automatically parses the zsync headers on the way to buffer the 
 * control file.
 *
 * Example:
 *
 * 	ZsyncRemoteControlFileParserPrivate rcfp;
 *
 *	// Overloaded Constructor.
 * 	ZsyncRemoteControlFileParserPrivate rcfp(QUrl("https://example.com/yourapp.zsync"));
 * 	ZsyncRemoteControlFileParserPrivate rcfp(QUrl("file:///home/user/yourapp.zsync"));
 * 	
*/
ZsyncRemoteControlFileParserPrivate::ZsyncRemoteControlFileParserPrivate(QNetworkAccessManager *NetworkManager)
    : QObject(NetworkManager)
{
    CONSTRUCT();
    return;
}

ZsyncRemoteControlFileParserPrivate::ZsyncRemoteControlFileParserPrivate
(const QUrl &controlFileUrl, QNetworkAccessManager *NetworkManager)
    : QObject(NetworkManager),
      _uControlFileUrl(controlFileUrl)
{
    CONSTRUCT();
    return;
}

ZsyncRemoteControlFileParserPrivate::~ZsyncRemoteControlFileParserPrivate()
{
    _pNManager.clear();
#ifdef LOGGING_ENABLED
    _pLogger.clear();
#endif // LOGGING_ENABLED
    _pControlFile.clear();
    return;
}


/* This public method safely sets the zsync control file url. */
void ZsyncRemoteControlFileParserPrivate::setControlFileUrl(const QUrl &controlFileUrl)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " setControlFileUrl : mutex is locked , operation ignored." WARNING_END;
        return;
    }
    INFO_START LOGR " setControlFileUrl : using " LOGR controlFileUrl LOGR " for zsync control file." INFO_END;
    _uControlFileUrl = controlFileUrl;
    _pMutex.unlock();
    return;
}

#ifdef LOGGING_ENABLED
/* This public method safely turns on and off the internal logger. */
void ZsyncRemoteControlFileParserPrivate::setShowLog(bool choose)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " setShowLog : mutex is locked , operation ignored." WARNING_END;
        return;
    } else if(choose) {
        disconnect(this, SIGNAL(logger(QString)),
                   this, SLOT(handleLogMessage(QString)));
        connect(this, SIGNAL(logger(QString)),
                this, SLOT(handleLogMessage(QString)));
	INFO_START LOGR " setShowLog : started logging." INFO_END;
    } else {
	INFO_START LOGR " setShowLog : stopping logging." INFO_END;
        disconnect(this, SIGNAL(logger(QString)),
                   this, SLOT(handleLogMessage(QString)));
    }
    _pMutex.unlock();
    return;
}
#endif // LOGGING_ENABLED

/* clears all internal cache in the class. */
void ZsyncRemoteControlFileParserPrivate::clear(void)
{
    if(!_pMutex.tryLock()) {
        WARNING_START LOGR " clear : mutex is locked , operation ignored." WARNING_END;
	return;
    }

    INFO_START LOGR " clear : flushed everything." INFO_END;

    _sZsyncMakeVersion.clear();
    _sTargetFileName.clear();
    _sTargetFileSHA1.clear();
    _sControlFileName.clear();
#ifdef LOGGING_ENABLED
    _sLogBuffer.clear();
#endif // LOGGING_ENABLED
    _pMTime = QDateTime();
    _nTargetFileBlockSize = _nTargetFileLength = _nTargetFileBlocks =
    _nWeakCheckSumBytes = _nStrongCheckSumBytes = _nConsecutiveMatchNeeded =
    _nCheckSumBlocksOffset = 0;
    _uTargetFileUrl.clear();
    _uControlFileUrl.clear();
    _pControlFile.clear();
    _pControlFile = nullptr;
    _pMutex.unlock();
    return;
}

/* Starts an async request to the given zsync control file. */
void ZsyncRemoteControlFileParserPrivate::getControlFile(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getControlFile : mutex is locked , operation ignored." WARNING_END;
        return;
    }

    if(_uControlFileUrl.isEmpty() || !_uControlFileUrl.isValid()) {
	WARNING_START LOGR " getControlFile : no zsync control file url(" LOGR _uControlFileUrl LOGR ") is given or valid." WARNING_END;
        _pMutex.unlock();
        return;
    }

    INFO_START LOGR " getControlFile : sending get request to " LOGR _uControlFileUrl LOGR "." INFO_END;

    QNetworkRequest request;
    request.setUrl(_uControlFileUrl);
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkReply *reply = _pNManager->get(request);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished(void)), this, SLOT(handleControlFile(void)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(handleDownloadProgress(qint64, qint64)));
    return;
}

/*
 * Starts to send zsync control file's checksum blocks through 
 * Qt signals , One has to connect this signal to the ZsyncCore
 * class to fill in the checksum blocks.
 * Does nothing if a async request is beign processed.
*/
void ZsyncRemoteControlFileParserPrivate::getTargetFileBlocks(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getTargetFileBlocks : mutex is locked , operation ignored." WARNING_END;
        return;
    }

    if(!_pControlFile ||
       !_pControlFile->isOpen() ||
       _pControlFile->size() - _nCheckSumBlocksOffset < (_nWeakCheckSumBytes + _nStrongCheckSumBytes) ||
       !_nCheckSumBlocksOffset) {
	FATAL_START LOGR " getTargetFileBlocks : zsync control file is not valid." FATAL_END;
        _pMutex.unlock();
        return;
    }

    INFO_START LOGR " getTargetFileBlocks : zsync control file checksum blocks offset(" 
	       LOGR _nCheckSumBlocksOffset LOGR ")." INFO_END;

    INFO_START LOGR " getTargetFileBlocks : This system uses " LOGR Q_BYTE_ORDER LOGR "." INFO_END;
    
    /*
     * We can make this parallel but I thought that was kind of
     * a over pull , Because this is just a read and store ,
     * Also sequential reads are faster than random reads.
     * To make this parallel we need to use this simple algorithm to
     * get the offset of the checksum block.
     *
     * CheckSumBlockOffset(blockid , controlFileOffset)
     * 	 	= ((_nWeakCheckSumBytes + _nStrongCheckSumBytes) * blockid)
     * 	 	  + controlFileOffset
    */
    _pControlFile->seek(_nCheckSumBlocksOffset); /* Seek to the offset of the checksum block. */
    INFO_START LOGR " getTargetFileBlocks : starting to send target file checksum blocks." INFO_END;
    for(zs_blockid id = 0; id < _nTargetFileBlocks ; ++id) {
        rsum r = { 0, 0 };
        unsigned char checksum[16];

        /* Read on. */
        if (_pControlFile->read(((char *)&r) + 4 - _nWeakCheckSumBytes, _nWeakCheckSumBytes) < 1
            || _pControlFile->read((char *)&checksum, _nStrongCheckSumBytes) < 1) {
            emit error(IO_READ_ERROR);
            return;
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

        emit receiveTargetFileBlocks(id, r, checksum);
    }
    _pMutex.unlock();
    INFO_START LOGR " getTargetFileBlocks : finished sending target file blocks." INFO_END;
    emit endOfTargetFileBlocks(); // Tell the user to start adding seed files.
    return;
}

/*
 * Returns the number blocks in the target file.
 * If an async request is processed currently , This will return a 0.
*/
size_t ZsyncRemoteControlFileParserPrivate::getTargetFileBlocksCount(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getTargetFileBlocksCount : mutex is locked , operation ignored." WARNING_END;
        return 0;
    }
    auto ret = _nTargetFileBlocks;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the url of the zsync control file.
 * If an async request is processed currently , This will return an
 * empty url.
*/ 
QUrl ZsyncRemoteControlFileParserPrivate::getControlFileUrl(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getControlFileUrl : mutex is locked , operation ignored." WARNING_END;;
        return QUrl();
    }
    auto ret = _uControlFileUrl;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the Zsync make version that is used to create the parsed
 * zsync control file.
 * This returns an empty string if a process is busy or the control
 * file was never parsed.
*/
QString ZsyncRemoteControlFileParserPrivate::getZsyncMakeVersion(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getZsyncMakeVersion : mutex is locked , operation ignored." WARNING_END;
        return QString();
    }
    auto ret = _sZsyncMakeVersion;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the target file's name.
 * This returns an emtpy string if a process is busy or the control
 * file was never parsed.
*/
QString ZsyncRemoteControlFileParserPrivate::getTargetFileName(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getTargetFileName : mutex is locked , operation ignored." WARNING_END;
        return QString();
    }
    auto ret = _sTargetFileName;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the target file's url.
 * Returns an empty url incase a process is busy or the control 
 * file was never parsed.
*/
QUrl ZsyncRemoteControlFileParserPrivate::getTargetFileUrl(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getTargetFileUrl : mutex is locked , operation ignored." WARNING_END;
        return QUrl();
    }
    auto ret = _uTargetFileUrl;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the SHA1 Hash of the target file.
 * This returns an empty string if a process is busy or the control
 * file was never parsed.
*/
QString ZsyncRemoteControlFileParserPrivate::getTargetFileSHA1(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getTargetFileSHA1 : mutex is locked , operation ignored." WARNING_END;
        return QString();
    }
    auto ret = _sTargetFileSHA1;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the Date and Time of Target file's MTime.
 * In case a process is busy or the control file was never parsed
 * then this would return an invalid datetime class.
*/
QDateTime ZsyncRemoteControlFileParserPrivate::getMTime(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getMTime : mutex is locked , operation ignored." WARNING_END;
        return QDateTime();
    }
    auto ret = _pMTime;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the BlockSize of the target file.
 * This would return 0 if any process is busy or the control file 
 * was never parsed.
*/
size_t ZsyncRemoteControlFileParserPrivate::getTargetFileBlockSize(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getTargetFileBlockSize : mutex is locked , operation ignored." WARNING_END;
        return 0;
    }
    auto ret = _nTargetFileBlockSize;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the target file's length.
 * This would return 0 if any process is busy or the control file
 * was never parsed.
*/
size_t ZsyncRemoteControlFileParserPrivate::getTargetFileLength(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getTargetFileLength : mutex is locked , operation ignored." WARNING_END;
        return 0;
    }
    auto ret = _nTargetFileLength;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the number of weak checksum bytes available in the 
 * zsync control file.
*/
qint32 ZsyncRemoteControlFileParserPrivate::getWeakCheckSumBytes(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getWeakCheckSumBytes : mutex is locked , operation ignored." WARNING_END;
        return 0;
    }
    auto ret = _nWeakCheckSumBytes;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the number of strong checksum bytes available in the 
 * zsync control file.
*/
qint32 ZsyncRemoteControlFileParserPrivate::getStrongCheckSumBytes(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getStrongCheckSumBytes : mutex is locked , operation ignored." WARNING_END;
        return 0;
    }
    auto ret = _nStrongCheckSumBytes;
    _pMutex.unlock();
    return ret;
}

/*
 * Returns the number of sequence matches needed for the zsync algorithm.
*/
qint32 ZsyncRemoteControlFileParserPrivate::getConsecutiveMatchNeeded(void)
{
    if(!_pMutex.tryLock()) {
	WARNING_START LOGR " getConsecutiveMatchNeeded : mutex is locked , operation ignored." WARNING_END;
        return 0;
    }
    auto ret = _nConsecutiveMatchNeeded;
    _pMutex.unlock();
    return ret;
}

/*
 * Calculates the download progress of the control file in percentage 
 * and emits it to the user.
*/
void ZsyncRemoteControlFileParserPrivate::handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
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
    emit progress(nPercentage);
    return;
}

/*
 * This private slot parses the control file.
*/
void ZsyncRemoteControlFileParserPrivate::handleControlFile(void)
{
    INFO_START LOGR " handleControlFile : starting to parse zsync control file." INFO_END;
    QNetworkReply *senderReply = (QNetworkReply*)QObject::sender();
    int responseCode = senderReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    INFO_START LOGR " handleControlFile : http response code(" LOGR responseCode LOGR ")." INFO_END;
    if(responseCode > 400) {
	emit error(ERROR_RESPONSE_CODE);
        return;
    }

    /*
     * Disconnect all ties before casting it as QIODevice to read.
    */
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, SIGNAL(finished(void)), this, SLOT(handleControlFile(void)));
    disconnect(senderReply, SIGNAL(downloadProgress(qint64, qint64)),
               this, SLOT(handleDownloadProgress(qint64, qint64)));

    _pControlFile = QSharedPointer<QBuffer>(new QBuffer);
    _pControlFile->open(QIODevice::ReadWrite);

    /*
     * Since QNetworkReply is sequential QIODevice , We cannot seek but we
     * need to seek on command for future operation so we copy everything
     * to our newly allocated QIODevice and Also find the offset on the way.
     * Since this is sequential , We cannot use *->pos() to get the current position
     * of the file pointer , Therefore we will use a temporary variable.
     */
    {
	INFO_START LOGR " handleControlFile : search for checksum blocks offset in the zsync control file." INFO_END;
        qint64 pos = 0;
        int bufferSize = 2; // 2 bytes.
        while(!senderReply->atEnd()) {
            /*
             * Read two bytes at a time , Since the marker for the
             * offset of the checksum blocks is \n\n.
             *
             * Therefore,
             *
             * ZsyncHeaders = (0 , offset - 2)
             * Checksums = (offset , EOF)
            */
            QByteArray data = senderReply->read(bufferSize);
            pos += bufferSize;
            if(bufferSize < 1024 && data.at(0) == 10 && data.at(1) == 10) {
                /*
                 * Set the offset and increase the buffer size
                 * to finish the rest of the copy faster.
                */
		INFO_START LOGR " handleControlFile : found checksum blocks offset(" LOGR pos LOGR ") in zsync control file." INFO_END;
                _nCheckSumBlocksOffset = pos;
                bufferSize = 1024; /* Use standard size of 1024 bytes. */
            }
            _pControlFile->write(data);
        }
    }
    _pControlFile->seek(0); /* seek to the top again. */
    if(!_nCheckSumBlocksOffset) {
        /* error , we don't know the marker and therefore
        				it must be an invalid control file.*/
        emit error(NO_MARKER_FOUND_IN_CONTROL_FILE );
        return;
    }
    QString ZsyncHeader(_pControlFile->read(_nCheckSumBlocksOffset - 2)); /* avoid reading the marker. */
    QStringList ZsyncHeaderList = QString(ZsyncHeader).split("\n");
    if(ZsyncHeaderList.size() < 8) {
        emit error(INVALID_ZSYNC_HEADERS_NUMBER);
        return;
    }

    _sZsyncMakeVersion = ZsyncHeaderList.at(0).split("zsync: ")[1];
    if(_sZsyncMakeVersion == ZsyncHeaderList.at(0)) {
        emit error(INVALID_ZSYNC_MAKE_VERSION);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync make version confirmed to be " LOGR _sZsyncMakeVersion LOGR "." INFO_END;

    _sTargetFileName = ZsyncHeaderList.at(1).split("Filename: ")[1];
    if(_sTargetFileName == ZsyncHeaderList.at(1)) {
        emit error(INVALID_ZSYNC_TARGET_FILENAME);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file name confirmed to be " LOGR _sTargetFileName LOGR "." INFO_END;

    _pMTime = QDateTime::fromString(ZsyncHeaderList.at(2).split("MTime: ")[1], "ddd, dd MMM yyyy HH:mm:ss +zzz0");
    if(!_pMTime.isValid()) {
        emit error(INVALID_ZSYNC_MTIME);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file MTime confirmed to be " LOGR _pMTime LOGR "." INFO_END;

    _nTargetFileBlockSize = (size_t)ZsyncHeaderList.at(3).split("Blocksize: ")[1].toInt();
    if(_nTargetFileBlockSize < 1024) {
        emit error(INVALID_ZSYNC_BLOCKSIZE);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file blocksize confirmed to be " LOGR _nTargetFileBlockSize LOGR " bytes." INFO_END;

    _nTargetFileLength =  (size_t)ZsyncHeaderList.at(4).split("Length: ")[1].toInt();
    if(_nTargetFileLength == 0) {
        emit error(INVALID_TARGET_FILE_LENGTH);
        return;
    }
    INFO_START LOGR " handleControlFile : zysnc target file length confirmed to be " LOGR _nTargetFileLength LOGR " bytes." INFO_END;

    QString HashLength = ZsyncHeaderList.at(5).split("Hash-Lengths: ")[1];
    QStringList HashLengths = HashLength.split(',');
    if(HashLengths.size() != 3) {
        emit error(INVALID_HASH_LENGTH_LINE);
        return;
    }

    _nConsecutiveMatchNeeded = HashLengths.at(0).toInt();
    _nWeakCheckSumBytes = HashLengths.at(1).toInt();
    _nStrongCheckSumBytes = HashLengths.at(2).toInt();
    if(_nWeakCheckSumBytes < 1 || _nWeakCheckSumBytes > 4
       || _nStrongCheckSumBytes < 3 || _nStrongCheckSumBytes > 16
       || _nConsecutiveMatchNeeded > 2 || _nConsecutiveMatchNeeded < 1) {
        emit error(INVALID_HASH_LENGTHS);
        return;
    }

    INFO_START LOGR " handleControlFile : " LOGR _nWeakCheckSumBytes LOGR " bytes of weak checksum is available." INFO_END;
    INFO_START LOGR " handleControlFile : " LOGR _nStrongCheckSumBytes LOGR " bytes of strong checksum is available." INFO_END;
    INFO_START LOGR " handleControlFile : " LOGR _nConsecutiveMatchNeeded LOGR " consecutive matches is needed." INFO_END;

    _uTargetFileUrl = QUrl(ZsyncHeaderList.at(6).split("URL: ")[1]);
    if(!_uTargetFileUrl.isValid()) {
        emit error(INVALID_TARGET_FILE_URL);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file url is confirmed to be " LOGR _uTargetFileUrl LOGR "." INFO_END;

    _sTargetFileSHA1 = ZsyncHeaderList.at(7).split("SHA-1: ")[1];
    if(_sTargetFileSHA1 == ZsyncHeaderList.at(7)) {
        emit error(INVALID_TARGET_FILE_SHA1);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file sha1 hash is confirmed to be " LOGR _sTargetFileSHA1 LOGR "." INFO_END;

    _nTargetFileBlocks = (_nTargetFileLength + _nTargetFileBlockSize - 1) / _nTargetFileBlockSize;
    INFO_START LOGR " handleControlFile : zsync target file has " LOGR _nTargetFileBlocks LOGR " number of blocks." INFO_END;

    _pMutex.unlock();
    emit receiveControlFile();
    return;
}

void ZsyncRemoteControlFileParserPrivate::handleNetworkError(QNetworkReply::NetworkError errorCode)
{
    if(errorCode == QNetworkReply::OperationCanceledError) {
        return;
    }
    QNetworkReply *senderReply = (QNetworkReply*)QObject::sender();
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this,SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, SIGNAL(finished(void)), this, SLOT(handleControlFile(void)));
    disconnect(senderReply, SIGNAL(downloadProgress(qint64, qint64)),
               this, SLOT(handleDownloadProgress(qint64, qint64)));

    FATAL_START LOGR " handleNetworkError : " LOGR errorCode LOGR "." FATAL_END;

    senderReply->deleteLater();
    emit error(UNKNOWN_NETWORK_ERROR);
    return;
}

/*
 * This slot will be called anytime error signal is emitted.
 * This is to prevent any deadlocks.
*/
void ZsyncRemoteControlFileParserPrivate::handleErrorSignal(short errorCode)
{
    /*
     * Since unlocking a unlocked mutex can cause
     * undefined behaviour , We have to be carefull on
     * this.
    */
    if(_pMutex.tryLock()) { // Check if it is locked.
        _pMutex.unlock(); // Unlock if locked.
    } else {
        /*
        * Unlock even if it is not locked
        * since tryLock() will lock the mutex again.
        */
        _pMutex.unlock();
    }
    
    FATAL_START LOGR " error : " LOGR errorCodeToString(errorCode) LOGR " occured."; 

    clear(); // clear all data to prevent later corrupted data collisions.
    return;
}

#ifdef LOGGING_ENABLED
/*
 * This private slot prints the logger messages in a nice way.
*/
void ZsyncRemoteControlFileParserPrivate::handleLogMessage(QString msg)
{
    qDebug().noquote() << "[ "
                       << QDateTime::currentDateTime().toString(Qt::ISODate)
                       << " ] ZsyncRemoteControlFileParserPrivate(" << _uControlFileUrl
		       << ")::"
                       << msg;
    return;
}
#endif // LOGGING_ENABLED

QString ZsyncRemoteControlFileParserPrivate::errorCodeToString(short errorCode)
{
	QString errorCodeString = "ZsyncRemoteControlFileParserPrivate::error_code(";
	switch(errorCode){
		case UNKNOWN_NETWORK_ERROR:
			errorCodeString.append("UNKNOWN_NETWORK_ERROR");
			break;
		case IO_READ_ERROR:
		        errorCodeString.append("IO_READ_ERROR");
			break;
		case ERROR_RESPONSE_CODE:
			errorCodeString.append("ERROR_RESPONSE_CODE");
			break;
		case NO_MARKER_FOUND_IN_CONTROL_FILE:
			errorCodeString.append("NO_MARKER_FOUND_IN_CONTROL_FILE");
			break;
        case INVALID_ZSYNC_HEADERS_NUMBER:
            errorCodeString.append("INVALID_ZSYNC_HEADERS_NUMBER");
            break;
        case INVALID_ZSYNC_MAKE_VERSION:
			errorCodeString.append("INVALID_ZSYNC_MAKE_VERSION");
			break;
        case INVALID_ZSYNC_TARGET_FILENAME:
			errorCodeString.append("INVALID_ZSYNC_TARGET_FILENAME");
			break;
        case INVALID_ZSYNC_MTIME:
			errorCodeString.append("INVALID_ZSYNC_MTIME");
			break;
        case INVALID_ZSYNC_BLOCKSIZE:
			errorCodeString.append("INVALID_ZSYNC_BLOCKSIZE");
			break;
        case INVALID_TARGET_FILE_LENGTH:
			errorCodeString.append("INVALID_TARGET_FILE_LENGTH");
			break;
        case INVALID_HASH_LENGTH_LINE:
			errorCodeString.append("INVALID_HASH_LENGTH_LINE");
			break;
        case INVALID_HASH_LENGTHS:
			errorCodeString.append("INVALID_HASH_LENGTHS");
			break;
        case INVALID_TARGET_FILE_URL:
			errorCodeString.append("INVALID_TARGET_FILE_URL");
			break;
        case INVALID_TARGET_FILE_SHA1:
			errorCodeString.append("INVALID_TARGET_FILE_SHA1");
			break;
		default:
			errorCodeString.append("UNKNOWN_ERROR_CODE");
			break;
	}

	errorCodeString.append(")");
	return errorCodeString;
}
