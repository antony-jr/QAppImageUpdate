#include <ZsyncRemoteControlFileParser_p.hpp>

using namespace AppImageUpdaterBridgePrivate;

static void doDeleteNetworkManager(QNetworkAccessManager *NetworkManager)
{
    NetworkManager->deleteLater();
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
                 QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager, doDeleteNetworkManager) :
                 QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager, doNotDeleteNetworkManager);
    connect(this, SIGNAL(error(short)), this, SLOT(handleErrorSignal(short)));
    return;
}

ZsyncRemoteControlFileParserPrivate::ZsyncRemoteControlFileParserPrivate
(const QUrl &controlFileUrl, QNetworkAccessManager *NetworkManager)
    : QObject(NetworkManager),
      _uControlFileUrl(controlFileUrl)
{
    _pLogger = QSharedPointer<QDebug>(new QDebug(&_sLogBuffer));
    _pNManager = (NetworkManager == nullptr) ?
                 QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager, doDeleteNetworkManager) :
                 QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager, doNotDeleteNetworkManager);
    connect(this, SIGNAL(error(short)), this, SLOT(handleErrorSignal(short)));
    return;
}

ZsyncRemoteControlFileParserPrivate::~ZsyncRemoteControlFileParserPrivate()
{
    _pNManager.clear();
    _pLogger.clear();
    _pControlFile.clear();
    return;
}


/*
 * Public Methods.
*/
void ZsyncRemoteControlFileParserPrivate::setControlFileUrl(const QUrl &controlFileUrl)
{
    if(!_pMutex.tryLock()) {
        return;
    }
    _uControlFileUrl = controlFileUrl;
    _pMutex.unlock();
    return;
}

void ZsyncRemoteControlFileParserPrivate::setShowLog(bool choose)
{
    if(!_pMutex.tryLock()) {
        return;
    } else if(choose) {
        disconnect(this, SIGNAL(logger(QString)),
                   this, SLOT(logPrinter(QString)));
        connect(this, SIGNAL(logger(QString)),
                this, SLOT(logPrinter(QString)));
    } else {
        disconnect(this, SIGNAL(logger(QString)),
                   this, SLOT(logPrinter(QString)));
    }
    _pMutex.unlock();
    return;
}

void ZsyncRemoteControlFileParserPrivate::clear(void)
{
    if(!_pMutex.tryLock()) {
        return;
    }
    _sZsyncMakeVersion.clear();
    _sTargetFileName.clear();
    _sTargetFileSHA1.clear();
    _sControlFileName.clear();
    _sLogBuffer.clear();
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

/*
 * Public Slots.
*/

void ZsyncRemoteControlFileParserPrivate::getControlFile(void)
{
    if(!_pMutex.tryLock()) {
        return;
    }

    if(_uControlFileUrl.isEmpty() || !_uControlFileUrl.isValid()) {
        _pMutex.unlock();
        return;
    }

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

void ZsyncRemoteControlFileParserPrivate::getTargetFileBlocks(void)
{
    /* Use seperate if control flows since
     * we don't know if a mutex is locked if we put
     * it in the same tag which can lead to deadlock.
    */
    if(!_pMutex.tryLock()) {
        return;
    }

    if(!_pControlFile ||
       !_pControlFile->isOpen() ||
       _pControlFile->size() - _nCheckSumBlocksOffset < (_nWeakCheckSumBytes + _nStrongCheckSumBytes) ||
       !_nCheckSumBlocksOffset) {
        _pMutex.unlock();
        return;
    }

    /*
     * We can make this parallel but I thought that was kind of
     * a over pull , Because this is just a read and sequential reads
     * are faster than random reads.
     * To make this parallel we need to use simple algorithm to
     * get the offset of the checksum block.
     *
     * CheckSumBlockOffset(blockid , controlFileOffset)
     * 	 	= ((_nWeakCheckSumBytes + _nStrongCheckSumBytes) * blockid)
     * 	 	  + controlFileOffset
    */
    _pControlFile->seek(_nCheckSumBlocksOffset); /* Seek to the offset of the checksum block. */
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
         * But most of the time we will need little endian since intel's microproccessors always follow
         * little endian byte order.
        */
        if(Q_BYTE_ORDER == Q_LITTLE_ENDIAN) {
            r.a = qFromBigEndian(r.a);
            r.b = qFromBigEndian(r.b);
        }

        emit receiveTargetFileBlocks(id, r, checksum);
    }
    _pMutex.unlock();
    emit endOfTargetFileBlocks();
    return;
}

size_t ZsyncRemoteControlFileParserPrivate::getTargetFileBlocksCount(void)
{
    if(!_pMutex.tryLock()) {
        return 0;
    }
    auto ret = _nTargetFileBlocks;
    _pMutex.unlock();
    return ret;
}

QUrl ZsyncRemoteControlFileParserPrivate::getControlFileUrl(void)
{
    if(!_pMutex.tryLock()) {
        return QUrl();
    }
    auto ret = _uControlFileUrl;
    _pMutex.unlock();
    return ret;
}

QString ZsyncRemoteControlFileParserPrivate::getZsyncMakeVersion(void)
{
    if(!_pMutex.tryLock()) {
        return QString();
    }
    auto ret = _sZsyncMakeVersion;
    _pMutex.unlock();
    return ret;
}

QString ZsyncRemoteControlFileParserPrivate::getTargetFileName(void)
{
    if(!_pMutex.tryLock()) {
        return QString();
    }
    auto ret = _sTargetFileName;
    _pMutex.unlock();
    return ret;
}

QUrl ZsyncRemoteControlFileParserPrivate::getTargetFileUrl(void)
{
    if(!_pMutex.tryLock()) {
        return QUrl();
    }
    auto ret = _uTargetFileUrl;
    _pMutex.unlock();
    return ret;
}

QString ZsyncRemoteControlFileParserPrivate::getTargetFileSHA1(void)
{
    if(!_pMutex.tryLock()) {
        return QString();
    }
    auto ret = _sTargetFileSHA1;
    _pMutex.unlock();
    return ret;
}

QDateTime ZsyncRemoteControlFileParserPrivate::getMTime(void)
{
    if(!_pMutex.tryLock()) {
        return QDateTime();
    }
    auto ret = _pMTime;
    _pMutex.unlock();
    return ret;
}

size_t ZsyncRemoteControlFileParserPrivate::getTargetFileBlockSize(void)
{
    if(!_pMutex.tryLock()) {
        return 0;
    }
    auto ret = _nTargetFileBlockSize;
    _pMutex.unlock();
    return ret;
}

size_t ZsyncRemoteControlFileParserPrivate::getTargetFileLength(void)
{
    if(!_pMutex.tryLock()) {
        return 0;
    }
    auto ret = _nTargetFileLength;
    _pMutex.unlock();
    return ret;
}

qint32 ZsyncRemoteControlFileParserPrivate::getWeakCheckSumBytes(void)
{
    if(!_pMutex.tryLock()) {
        return 0;
    }
    auto ret = _nWeakCheckSumBytes;
    _pMutex.unlock();
    return ret;
}

qint32 ZsyncRemoteControlFileParserPrivate::getStrongCheckSumBytes(void)
{
    if(!_pMutex.tryLock()) {
        return 0;
    }
    auto ret = _nStrongCheckSumBytes;
    _pMutex.unlock();
    return ret;
}

qint32 ZsyncRemoteControlFileParserPrivate::getConsecutiveMatchNeeded(void)
{
    if(!_pMutex.tryLock()) {
        return 0;
    }
    auto ret = _nConsecutiveMatchNeeded;
    _pMutex.unlock();
    return ret;
}

/*
 * Private slots.
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

void ZsyncRemoteControlFileParserPrivate::handleControlFile(void)
{
    QNetworkReply *senderReply = (QNetworkReply*)QObject::sender();
    int responseCode = senderReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
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

    _sTargetFileName = ZsyncHeaderList.at(1).split("Filename: ")[1];
    if(_sTargetFileName == ZsyncHeaderList.at(1)) {
        emit error(INVALID_ZSYNC_TARGET_FILENAME);
        return;
    }

    _pMTime = QDateTime::fromString(ZsyncHeaderList.at(2).split("MTime: ")[1], "ddd, dd MMM yyyy HH:mm:ss +zzz0");
    if(!_pMTime.isValid()) {
        emit error(INVALID_ZSYNC_MTIME);
        return;
    }

    _nTargetFileBlockSize = (size_t)ZsyncHeaderList.at(3).split("Blocksize: ")[1].toInt();
    if(_nTargetFileBlockSize < 1024) {
        emit error(INVALID_ZSYNC_BLOCKSIZE);
        return;
    }

    _nTargetFileLength =  (size_t)ZsyncHeaderList.at(4).split("Length: ")[1].toInt();
    if(_nTargetFileLength == 0) {
        emit error(INVALID_TARGET_FILE_LENGTH);
        return;
    }

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

    _uTargetFileUrl = QUrl(ZsyncHeaderList.at(6).split("URL: ")[1]);
    if(!_uTargetFileUrl.isValid()) {
        emit error(INVALID_TARGET_FILE_URL);
        return;
    }

    _sTargetFileSHA1 = ZsyncHeaderList.at(7).split("SHA-1: ")[1];
    if(_sTargetFileSHA1 == ZsyncHeaderList.at(7)) {
        emit error(INVALID_TARGET_FILE_SHA1);
        return;
    }

    /*
     * No need to worry about zero devision error since blocksize is checked
     * earlier. Anything lesser than 1024 bytes is an invalid blocksize.
    */
    _nTargetFileBlocks = (_nTargetFileLength + _nTargetFileBlockSize - 1) / _nTargetFileBlockSize;

    _pMutex.unlock();
    emit receiveControlFile(_nTargetFileBlocks, _nTargetFileBlockSize, _nWeakCheckSumBytes,
                            _nStrongCheckSumBytes, _nConsecutiveMatchNeeded, _nTargetFileLength);
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
    clear(); // clear all data to prevent later corrupted data collisions.
    return;
}

void ZsyncRemoteControlFileParserPrivate::handleLogMessage(QString msg)
{
    qDebug().noquote() << "[ "
                       << QDateTime::currentDateTime().toString(Qt::ISODate)
                       << " ]"
                       << msg;
    return;
}
