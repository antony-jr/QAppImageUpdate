#include <AppImageUpdateInformation_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncWriter_p.hpp>
#include <AppImageDeltaWriter.hpp>

using namespace AppImageUpdaterBridge;

#define CONSTRUCT(x) setObjectName("AppImageDeltaWriter"); \
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
		     if(!singleThreaded){ \
		     _pControlFileParser->moveToThread(_pSharedThread.data()); \
		     } \
		     _pControlFileParser->setObjectName("ZsyncRemoteControlFileParserPrivate"); \
		     _pUpdateInformation->setObjectName("AppImageUpdateInformationPrivate"); \
		     _pDeltaWriter->setObjectName("ZsyncWriterPrivate"); \
		     _pUpdateInformation->setLoggerName("UpdateInformation"); \
		     _pControlFileParser->setLoggerName("ControlFileParser"); \
		     _pDeltaWriter->setLoggerName("AppImageDeltaWriter"); \
		     connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::statusChanged , \
			      this , &AppImageDeltaWriter::statusChanged , Qt::DirectConnection); \
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
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::statusChanged , \
			      this , &AppImageDeltaWriter::statusChanged , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::progress , \
			      this , &AppImageDeltaWriter::progress , Qt::DirectConnection); \
                     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::error , \
			      this , &AppImageDeltaWriter::error , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::logger , \
			      this , &AppImageDeltaWriter::logger , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::started, \
			      this , &AppImageDeltaWriter::started , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::canceled , \
			      this , &AppImageDeltaWriter::canceled , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::paused , \
			      this , &AppImageDeltaWriter::paused , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::resumed , \
			      this , &AppImageDeltaWriter::resumed , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::finished , \
			      this , &AppImageDeltaWriter::finished , Qt::DirectConnection); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::zsyncInformation, \
			     _pDeltaWriter.data() , &ZsyncWriterPrivate::setConfiguration); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::finishedConfiguring , \
			     _pDeltaWriter.data() , &ZsyncWriterPrivate::start); \
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
	if(!_pSharedThread.isNull()){
	_pSharedThread->quit();
	_pSharedThread->wait();
	}
	return;
}

AppImageDeltaWriter &AppImageDeltaWriter::start(void)
{
	connect(this , &AppImageDeltaWriter::updateAvailable , this , &AppImageDeltaWriter::handleUpdateAvailable); 
	checkForUpdate();
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::cancel(void)
{
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("cancel(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::QueuedConnection);
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::pause(void)
{
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("pause(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::QueuedConnection);
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::resume(void)
{
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("resume(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::QueuedConnection);	
	return *this;
}

bool AppImageDeltaWriter::isCanceled(void) const
{
	bool ret;
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("isCanceled(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::DirectConnection , Q_RETURN_ARG(bool , ret));	
	return ret;
}

bool AppImageDeltaWriter::isFinished(void) const
{
	bool ret;
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("isFinished(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::DirectConnection , Q_RETURN_ARG(bool , ret));	
	return ret;
}

bool AppImageDeltaWriter::isPaused(void) const
{
	bool ret;
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("isPaused(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::DirectConnection , Q_RETURN_ARG(bool , ret));	
	return ret;
}

bool AppImageDeltaWriter::isRunning(void) const
{
	bool ret;
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("isRunning(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::DirectConnection , Q_RETURN_ARG(bool , ret));	
	return ret;
}

bool AppImageDeltaWriter::isStarted(void) const
{
	bool ret;
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("isStarted(void)")))
		    .invoke(_pDeltaWriter.data() , Qt::DirectConnection , Q_RETURN_ARG(bool , ret));	
	return ret;
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
	{
	auto metaObject = _pDeltaWriter->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pDeltaWriter.data() , Qt::QueuedConnection , Q_ARG(bool , choice));
	
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

AppImageDeltaWriter &AppImageDeltaWriter::clear(void)
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



AppImageDeltaWriter &AppImageDeltaWriter::checkForUpdate(void)
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

void AppImageDeltaWriter::handleUpdateAvailable(bool updateAvailable , QString AppImageFilePath)
{
	(void)AppImageFilePath;
	disconnect(this , &AppImageDeltaWriter::updateAvailable , this , &AppImageDeltaWriter::handleUpdateAvailable); 
	if(updateAvailable){
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getZsyncInformation(void)")))
		    .invoke(_pControlFileParser.data() , Qt::QueuedConnection);	
	}else{
		emit finished(/*is Download Needed ? = */false);
	}
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
	QString localAppImageSHA1Hash = embededUpdateInformation["FileInformation"].toObject()["AppImageSHA1Hash"].toString() ,
		localAppImagePath = embededUpdateInformation["FileInformation"].toObject()["AppImageFilePath"].toString();

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
			     (code < 100) ? ZsyncRemoteControlFileParserPrivate::errorCodeToString(code) :
			     ZsyncWriterPrivate::errorCodeToString(code);
}

QString AppImageDeltaWriter::statusCodeToString(short code)
{
	return (code < 50) ? AppImageUpdateInformationPrivate::statusCodeToString(code) :
			     (code < 100) ? ZsyncRemoteControlFileParserPrivate::statusCodeToString(code) :
			     ZsyncWriterPrivate::statusCodeToString(code);
}
