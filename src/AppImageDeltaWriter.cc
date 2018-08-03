#include <AppImageUpdateInformation_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncCoreJob_p.hpp>
#include <AppImageDeltaWriter.hpp>

#define APPIMAGE_DELTA_WRITER QString("AppImageDeltaWriter")

using namespace AppImageUpdaterBridge;

#define CONSTRUCT(x) setObjectName(APPIMAGE_DELTA_WRITER); \
		     _pFutureWatcher.reset(new QFutureWatcher<ZsyncCoreJobPrivate::JobResult>);\
		     if(!singleThreaded){ \
		     _pSharedThread.reset(new QThread);\
		     _pSharedThread->start(); \
		     } \
		     _pSharedNetworkAccessManager.reset(new QNetworkAccessManager); \
		     _pUpdateInformation.reset(new AppImageUpdateInformationPrivate); \
		     if(!singleThreaded){ \
		     _pSharedNetworkAccessManager->moveToThread(_pSharedThread.data()); \
		     _pUpdateInformation->moveToThread(_pSharedThread.data()); \
		      } \
		     _pControlFileParser.reset(new ZsyncRemoteControlFileParserPrivate(_pSharedNetworkAccessManager.data())); \
		     if(!singleThreaded){ \
		     _pControlFileParser->moveToThread(_pSharedThread.data()); \
		     } \
		     _pControlFileParser->setObjectName("ZsyncRemoteControlFileParserPrivate"); \
		     _pUpdateInformation->setObjectName("AppImageUpdateInformationPrivate"); \
		     _pUpdateInformation->setLoggerName(APPIMAGE_DELTA_WRITER); \
		     _pControlFileParser->setLoggerName(APPIMAGE_DELTA_WRITER); \
		     connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::info , \
			      this , &AppImageDeltaWriter::embededInformation , Qt::DirectConnection); \
		     connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::progress , \
	 		      this , &AppImageDeltaWriter::progress , Qt::DirectConnection); \
	             connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::error , \
			      this , &AppImageDeltaWriter::error , Qt::DirectConnection); \
	             connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::logger , \
			      this , &AppImageDeltaWriter::logger , Qt::DirectConnection); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::statusChanged ,\
			      this , &AppImageDeltaWriter::statusChanged , Qt::DirectConnection); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::error, \
			      this , &AppImageDeltaWriter::error , Qt::DirectConnection); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::progress , \
			      this , &AppImageDeltaWriter::progress , Qt::DirectConnection);  \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::logger , \
			      this , &AppImageDeltaWriter::logger , Qt::DirectConnection); \
		     connect(_pFutureWatcher.data() , &QFutureWatcher<ZsyncCoreJobPrivate::JobResult>::started , \
			      this , &AppImageDeltaWriter::started , Qt::DirectConnection); \
		     connect(_pFutureWatcher.data() , &QFutureWatcher<ZsyncCoreJobPrivate::JobResult>::canceled , \
			      this , &AppImageDeltaWriter::canceled , Qt::DirectConnection); \
		     connect(_pFutureWatcher.data() , &QFutureWatcher<ZsyncCoreJobPrivate::JobResult>::paused , \
			      this , &AppImageDeltaWriter::paused , Qt::DirectConnection); \
		     connect(_pFutureWatcher.data() , &QFutureWatcher<ZsyncCoreJobPrivate::JobResult>::resumed , \
			      this , &AppImageDeltaWriter::resumed , Qt::DirectConnection); \
		     connect(_pFutureWatcher.data() , &QFutureWatcher<ZsyncCoreJobPrivate::JobResult>::finished , \
			      this , &AppImageDeltaWriter::handleFinished); \
		     setAppImage(x);



AppImageDeltaWriter::AppImageDeltaWriter(bool singleThreaded , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(nullptr);
	return;
}

AppImageDeltaWriter::AppImageDeltaWriter(const QString &AppImagePath , bool singleThreaded , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImagePath);
	return;
}

AppImageDeltaWriter::AppImageDeltaWriter(QFile *AppImage , bool singleThreaded , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImage);
	return;
}



AppImageDeltaWriter::~AppImageDeltaWriter()
{
	if(_pFutureWatcher->isRunning() || _pFutureWatcher->isStarted() || _pFutureWatcher->isPaused())
	{
	    _pFutureWatcher->cancel();
	    _pFutureWatcher->waitForFinished();
	}

	if(!_pSharedThread.isNull()){
	_pSharedThread->quit();
	_pSharedThread->wait();
	}
	return;
}

AppImageDeltaWriter &AppImageDeltaWriter::start(void)
{
	if(_pFutureWatcher->isRunning() || _pFutureWatcher->isStarted() || _pFutureWatcher->isPaused()){
		return *this;
	}
	connect(this , &AppImageDeltaWriter::updateAvailable , this , &AppImageDeltaWriter::handleUpdateAvailable); 
	checkForUpdate();
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::setAppImage(const QString &AppImagePath)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(const QString&)")))
		    .invoke(_pUpdateInformation.data() , Qt::QueuedConnection , Q_ARG(QString , AppImagePath));
	return *this;
}
AppImageDeltaWriter &AppImageDeltaWriter::setAppImage(QFile *AppImage)
{
	if(!AppImage){
	return *this;
	}
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(QFile*)")))
		    .invoke(_pUpdateInformation.data() , Qt::QueuedConnection , Q_ARG(QFile* , AppImage));
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::setShowLog(bool choice)
{
	{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pUpdateInformation.data() , Qt::QueuedConnection , Q_ARG(bool , choice));
	}
	{
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pControlFileParser.data() , Qt::QueuedConnection , Q_ARG(bool , choice));
	
	}
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::getAppImageEmbededInformation(void)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getInfo(void)")))
		    .invoke(_pUpdateInformation.data() , Qt::QueuedConnection);
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::verifyAndConstructTargetFile(void)
{
	if(_pTargetFile.isNull()){
		return *this;
	}
	QString CurrentTargetFileSHA1Hash;
	auto targetFileLength = _pControlFileParser->getTargetFileLength();
	qint64 bufferSize = 0;

	_pTargetFile->resize(targetFileLength);
	_pTargetFile->seek(0);
    	
	QCryptographicHash *SHA1Hasher = new QCryptographicHash(QCryptographicHash::Sha1);
    	if(targetFileLength >= 1073741824){ // 1 GiB and more.
	    bufferSize = 104857600; // copy per 100 MiB.
    	}
    	else if(targetFileLength >= 1048576 ){ // 1 MiB and more.
	    bufferSize = 1048576; // copy per 1 MiB.
   	}else if(targetFileLength >= 1024){ // 1 KiB and more.
	    bufferSize = 4096; // copy per 4 KiB.
   	}else{ // less than 1 KiB
	    bufferSize = 1024; // copy per 1 KiB.
    	}
	while(!_pTargetFile->atEnd()){
	SHA1Hasher->addData(_pTargetFile->read(bufferSize));
	QCoreApplication::processEvents();
    	}
    	CurrentTargetFileSHA1Hash= QString(SHA1Hasher->result().toHex().toUpper());	
	delete SHA1Hasher; 

	_pTargetFile->seek(0);

	if(CurrentTargetFileSHA1Hash == _pControlFileParser->getTargetFileSHA1()){
		_pTargetFile->rename(_pControlFileParser->getTargetFileName());
		_pTargetFile->setAutoRemove(false);
		_pTargetFile->close();
		_pTargetFile.reset(nullptr);
		emit verifiedAndConstructedTargetFile();
	}
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::clear(void)
{
	_sLocalAppImagePath.clear();
	_sLocalAppImageSHA1Hash.clear();
	{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
		    .invoke(_pUpdateInformation.data() , Qt::QueuedConnection);
	}
	{
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
		    .invoke(_pControlFileParser.data() , Qt::QueuedConnection);	
	}
	return *this;
}



AppImageDeltaWriter &AppImageDeltaWriter::checkForUpdate(void)
{ 
	/*
	 * First we need the resource from the 
	 * AppImage binary , such as the update information 
	 * and the AppImage binary's checksums(SHA1 Hash) and full path.
	*/
	connect(_pUpdateInformation.data() , SIGNAL(info(QJsonObject)) , 
		_pControlFileParser.data() , SLOT(setControlFileUrl(QJsonObject)));
	connect(_pControlFileParser.data() , SIGNAL(receiveControlFile(void)) ,
		_pControlFileParser.data() , SLOT(getUpdateCheckInformation(void)));
	connect(_pControlFileParser.data() , SIGNAL(updateCheckInformation(QJsonObject)) ,
		 this , SLOT(handleUpdateCheckInformation(QJsonObject)));
	getAppImageEmbededInformation();
	return *this; 
}

void AppImageDeltaWriter::handleUpdateAvailable(bool updateAvailable , QString AppImageFilePath)
{
	disconnect(this , &AppImageDeltaWriter::updateAvailable , this , &AppImageDeltaWriter::handleUpdateAvailable); 
	if(updateAvailable){
		QString targetFilePath;
		auto path = QFileInfo(AppImageFilePath).path()

		path = (path.isEmpty()) ? QDir::currentPath() : path;
		targetFilePath = path + _pControlFileParser->getTargetFileName() + ".XXXXXXXXX.part";

		QFileInfo perm(path);
		if(!perm.isWritable() || !perm.isReadable()){
			emit error(NO_PERMISSION_TO_READ_WRITE_TARGET_FILE);
			return;
		}

		_pTargetFile.reset(new QTemporaryFile(targetFilePath));
		if(!_pTargetFile->open()){
			emit error(CANNOT_OPEN_TARGET_FILE);
			return;
		}

		/*
		 * To open the target file we have to 
		 * request fileName() from the temporary file.
		*/
		(void)_pTargetFile->fileName();

		connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::zsyncCoreJobInformation ,
			this , &AppImageDeltaWriter::handleZsyncCoreJobInformation);
		auto metaObject = _pControlFileParser->metaObject();
		metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getZsyncCoreJobInformation(QFile*)")))
		    	    .invoke(_pControlFileParser.data() , Qt::QueuedConnection , Q_ARG(QFile* , (QFile*)_pTargetFile.data()));
	}
	return;
}

void AppImageDeltaWriter::handleZsyncCoreJobInformation(QList<ZsyncCoreJobPrivate::JobInformation> jobInfo)
{
	disconnect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::zsyncCoreJobInformation ,
		this , &AppImageDeltaWriter::handleZsyncCoreJobInformation);

	jobs = jobInfo;
	_pFuture.reset(new QFuture<QList<ZsyncCoreJobPrivate::JobResult>>);
	*_pFuture = QtConcurrent::map(jobs , [this](ZsyncCoreJobPrivate::JobInformation info) -> ZsyncCoreJobPrivate::JobResult {
		ZsyncCoreJobPrivate job(info);
		return job.start();
	});
	_pFutureWatcher->setFuture(*_pFuture);
	return;
}

void AppImageDeltaWriter::handleFinished(void)
{
	_pRanges.reset(new QVector<QPair<qint32 , qint32>>);
	_pBlockHashes.reset(new QHash<qint32 , QByteArray>);
	qint32 gotBlocks = 0;
	Q_FOREACH(_pFuture->results() , ZsyncCoreJobPrivate::JobResult result)
	{
		if(result.isErrored){
			emit error(result.errorCode);
			return;
		}

		if(result.requiredBlocksMd4Sums && result.requiredRanges){
		_pRanges->append(*(result.requiredRanges));
		delete (result.requiredRanges);
		_pBlockHashes->unite(*(result.requiredBlocksMd4Sums));
		delete (result.requiredBlocksMd4Sums);
		}
		gotBlocks += result.gotBlocks;
		QCoreApplication::processEvents();
	}
	if(_pRanges->isEmpty() && _pBlockHashes->isEmpty()){
		_pRanges.reset(nullptr);
		_pBlockHashes.reset(nullptr);
	}else{
		emit blockDownloaderInformation(_pBlockHashes.data() , _pRanges.data() , _pTargetFile.data());
	}
	emit finished();
	return;
}

void AppImageDeltaWriter::handleUpdateCheckInformation(QJsonObject information)
{
	disconnect(_pUpdateInformation.data() , SIGNAL(info(QJsonObject)) , 
		_pControlFileParser.data() , SLOT(setControlFileUrl(QJsonObject)));
	disconnect(_pControlFileParser.data() , SIGNAL(receiveControlFile(void)) ,
		_pControlFileParser.data() , SLOT(getUpdateCheckInformation(void)));
	disconnect(_pControlFileParser.data() , SIGNAL(updateCheckInformation(QJsonObject)) ,
		 this , SLOT(handleUpdateCheckInformation(QJsonObject)));

	if(information.isEmpty()){
		return;
	}

	auto embededUpdateInformation = information["EmbededUpdateInformation"].toObject();
	auto remoteTargetFileSHA1Hash = information["RemoteTargetFileSHA1Hash"].toString();
	QString localAppImageSHA1Hash = embededUpdateInformation["FileInformation"].toObject()["LocalAppImageSHA1Hash"] ,
		localAppImagePath = embededUpdateInformation["FileInformation"].toObject()["AppImageFilePath"];
	
	emit updateAvailable((localAppImageSHA1Hash != remoteTargetFileSHA1Hash) , localAppImagePath);
	return;	
}

QThread *AppImageDeltaWriter::sharedQThread(void) const
{
	return _pSharedThread.data();
}

QNetworkAccessManager *AppImageDeltaWriter::sharedNetworkAccessManager(void) const
{
	return _pSharedNetworkAccessManager.data();
}

QString AppImageDeltaWriter::errorCodeToString(short code)
{
	return (code < 50) ? AppImageUpdateInformationPrivate::errorCodeToString(code) :
			     ZsyncRemoteControlFileParserPrivate::errorCodeToString(code);
}

QString AppImageDeltaWriter::statusCodeToString(short code)
{
	return (code < 50) ? AppImageUpdateInformationPrivate::statusCodeToString(code) :
			     ZsyncRemoteControlFileParserPrivate::statusCodeToString(code);
}
