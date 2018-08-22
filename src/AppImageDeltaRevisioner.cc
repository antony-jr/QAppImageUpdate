#include <AppImageUpdateInformation_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncWriter_p.hpp>
#include <ZsyncBlockRangeDownloader_p.hpp>
#include <AppImageDeltaRevisioner.hpp>

using namespace AppImageUpdaterBridge;

#define CONSTRUCT(x) setObjectName("AppImageDeltaRevisioner"); \
		     if(!singleThreaded){ \
		     _pSharedThread.reset(new QThread);\
		     _pSharedThread->start(); \
		     } \
		     _pSharedNetworkAccessManager.reset(new QNetworkAccessManager); \
		     _pUpdateInformation.reset(new AppImageUpdateInformationPrivate); \
		     _pDeltaWriter.reset(new ZsyncWriterPrivate); \
		     if(!singleThreaded){ \
		     _pSharedNetworkAccessManager->moveToThread(_pSharedThread.data()); \
		     _pUpdateInformation->moveToThread(_pSharedThread.data()); \
		     _pDeltaWriter->moveToThread(_pSharedThread.data()); \
		      } \
		     _pControlFileParser.reset(new ZsyncRemoteControlFileParserPrivate(_pSharedNetworkAccessManager.data())); \
		     _pBlockDownloader.reset(new ZsyncBlockRangeDownloaderPrivate(_pControlFileParser.data() ,  \
					                                         _pDeltaWriter.data() , \
					                                         _pSharedNetworkAccessManager.data())); \
		     if(!singleThreaded){ \
		     _pControlFileParser->moveToThread(_pSharedThread.data()); \
		     _pBlockDownloader->moveToThread(_pSharedThread.data()); \
		     } \
		     _pControlFileParser->setObjectName("ZsyncRemoteControlFileParserPrivate"); \
		     _pUpdateInformation->setObjectName("AppImageUpdateInformationPrivate"); \
		     _pDeltaWriter->setObjectName("ZsyncWriterPrivate"); \
                     _pBlockDownloader->setObjectName("ZsyncBlockRangeDownloaderPrivate"); \
		     _pUpdateInformation->setLoggerName("UpdateInformation"); \
		     _pControlFileParser->setLoggerName("ControlFileParser"); \
		     _pDeltaWriter->setLoggerName("AppImageDeltaRevisioner"); \
		     connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::statusChanged , \
			      this , &AppImageDeltaRevisioner::statusChanged , Qt::DirectConnection); \
		     connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::info , \
			      this , &AppImageDeltaRevisioner::embededInformation , Qt::DirectConnection); \
	             connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::error , \
			      this , &AppImageDeltaRevisioner::error , Qt::DirectConnection); \
	             connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::logger , \
			      this , &AppImageDeltaRevisioner::logger , Qt::DirectConnection); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::statusChanged ,\
			      this , &AppImageDeltaRevisioner::statusChanged , Qt::DirectConnection); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::error, \
			      this , &AppImageDeltaRevisioner::error , Qt::DirectConnection); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::logger , \
			      this , &AppImageDeltaRevisioner::logger , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::statusChanged , \
			      this , &AppImageDeltaRevisioner::statusChanged , Qt::DirectConnection); \
                     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::error , \
			      this , &AppImageDeltaRevisioner::error , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::logger , \
			      this , &AppImageDeltaRevisioner::logger , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::started, \
			      this , &AppImageDeltaRevisioner::started , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::canceled , \
			      this , &AppImageDeltaRevisioner::canceled , Qt::DirectConnection); \
		     connect(_pBlockDownloader.data() , &ZsyncBlockRangeDownloaderPrivate::started , \
			      this,  &AppImageDeltaRevisioner::handleBlockDownloaderStarted); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::zsyncInformation, \
			     _pDeltaWriter.data() , &ZsyncWriterPrivate::setConfiguration); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::finishedConfiguring , \
			     _pDeltaWriter.data() , &ZsyncWriterPrivate::start); \
		     setAppImage(x);



AppImageDeltaRevisioner::AppImageDeltaRevisioner(bool singleThreaded , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(nullptr);
	return;
}

AppImageDeltaRevisioner::AppImageDeltaRevisioner(const QString &AppImagePath , bool singleThreaded , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImagePath);
	return;
}

AppImageDeltaRevisioner::AppImageDeltaRevisioner(QFile *AppImage , bool singleThreaded , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImage);
	return;
}

AppImageDeltaRevisioner::~AppImageDeltaRevisioner()
{
	if(!_pSharedThread.isNull()){
	_pSharedThread->quit();
	_pSharedThread->wait();
	}
	return;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::start(void)
{
	connect(this , &AppImageDeltaRevisioner::updateAvailable , this , &AppImageDeltaRevisioner::handleUpdateAvailable); 
	checkForUpdate();
	return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::cancel(void)
{
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("cancel(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::QueuedConnection);
	return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::setAppImage(const QString &AppImagePath)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(const QString&)")))
		    .invoke(_pUpdateInformation.data() , Qt::QueuedConnection , Q_ARG(QString , AppImagePath));
	return *this;
}
AppImageDeltaRevisioner &AppImageDeltaRevisioner::setAppImage(QFile *AppImage)
{
	if(!AppImage){
	return *this;
	}
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(QFile*)")))
		    .invoke(_pUpdateInformation.data() , Qt::QueuedConnection , Q_ARG(QFile* , AppImage));
	return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::setShowLog(bool choice)
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
	{
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pDeltaWriter.data() , Qt::QueuedConnection , Q_ARG(bool , choice));
	
	}
	return *this;
}


AppImageDeltaRevisioner &AppImageDeltaRevisioner::getAppImageEmbededInformation(void)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getInfo(void)")))
		    .invoke(_pUpdateInformation.data() , Qt::QueuedConnection);
	return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::clear(void)
{
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
	{
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::QueuedConnection);	
	}
	return *this;
}



AppImageDeltaRevisioner &AppImageDeltaRevisioner::checkForUpdate(void)
{ 
	connect(_pUpdateInformation.data() , SIGNAL(info(QJsonObject)) , 
		_pControlFileParser.data() , SLOT(setControlFileUrl(QJsonObject)));
	connect(_pControlFileParser.data() , SIGNAL(receiveControlFile(void)) ,
		_pControlFileParser.data() , SLOT(getUpdateCheckInformation(void)));
	connect(_pControlFileParser.data() , SIGNAL(updateCheckInformation(QJsonObject)) ,
		 this , SLOT(handleUpdateCheckInformation(QJsonObject)));
	getAppImageEmbededInformation();
	return *this; 
}

void AppImageDeltaRevisioner::handleBlockDownloaderStarted(void)
{
	connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::finished , 
			this , &AppImageDeltaRevisioner::handleDeltaWriterFinished);
	disconnect(_pDeltaWriter.data() , &ZsyncWriterPrivate::progress ,
			this , &AppImageDeltaRevisioner::progress);
	connect(_pBlockDownloader.data() , &ZsyncBlockRangeDownloaderPrivate::progress , 
			this , &AppImageDeltaRevisioner::progress , Qt::DirectConnection);
	return;
}

void AppImageDeltaRevisioner::handleDeltaWriterFinished(bool failed)
{
	disconnect(_pDeltaWriter.data() , &ZsyncWriterPrivate::finished , 
			this , &AppImageDeltaRevisioner::handleDeltaWriterFinished);	
	disconnect(_pBlockDownloader.data() , &ZsyncBlockRangeDownloaderPrivate::progress , 
			this , &AppImageDeltaRevisioner::progress);
	if(failed){
		emit error(CANNOT_CONSTRUCT_TARGET_FILE);
	}else{
		emit finished();
	}
	return;
}

void AppImageDeltaRevisioner::handleUpdateAvailable(bool updateAvailable , QString AppImageFilePath)
{
	(void)AppImageFilePath;
	disconnect(this , &AppImageDeltaRevisioner::updateAvailable , this , &AppImageDeltaRevisioner::handleUpdateAvailable); 
	if(updateAvailable){
 	connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::progress ,
			      this , &AppImageDeltaRevisioner::progress , Qt::DirectConnection);                
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getZsyncInformation(void)")))
		    .invoke(_pControlFileParser.data() , Qt::QueuedConnection);	
	}else{
		emit finished();
	}
	return;
}

void AppImageDeltaRevisioner::handleUpdateCheckInformation(QJsonObject information)
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
	QString localAppImageSHA1Hash = embededUpdateInformation["FileInformation"].toObject()["AppImageSHA1Hash"].toString() ,
		localAppImagePath = embededUpdateInformation["FileInformation"].toObject()["AppImageFilePath"].toString();

	emit updateAvailable((localAppImageSHA1Hash != remoteTargetFileSHA1Hash) , localAppImagePath);
	return;	
}

QString AppImageDeltaRevisioner::errorCodeToString(short code)
{
	return (code < 50) ? AppImageUpdateInformationPrivate::errorCodeToString(code) :
			     (code < 100) ? ZsyncRemoteControlFileParserPrivate::errorCodeToString(code) :
			     ZsyncWriterPrivate::errorCodeToString(code);
}

QString AppImageDeltaRevisioner::statusCodeToString(short code)
{
	return (code < 50) ? AppImageUpdateInformationPrivate::statusCodeToString(code) :
			     (code < 100) ? ZsyncRemoteControlFileParserPrivate::statusCodeToString(code) :
			     ZsyncWriterPrivate::statusCodeToString(code);
}
