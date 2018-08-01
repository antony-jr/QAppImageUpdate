#include <AppImageUpdateInformation_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <AppImageDeltaWriter.hpp>

#define APPIMAGE_DELTA_WRITER QString("AppImageDeltaWriter")

using namespace AppImageUpdaterBridge;

#define CONSTRUCT(x) setObjectName(APPIMAGE_DELTA_WRITER); \
		     _pSharedThread = new QThread;\
		     _pSharedThread->start(); \
		     _pSharedNetworkAccessManager = new QNetworkAccessManager; \
		     _pUpdateInformation = new AppImageUpdateInformationPrivate; \
		     _pSharedNetworkAccessManager->moveToThread(_pSharedThread); \
		     _pUpdateInformation->moveToThread(_pSharedThread); \
		     _pControlFileParser = new ZsyncRemoteControlFileParserPrivate(_pSharedNetworkAccessManager); \
		     _pControlFileParser->moveToThread(_pSharedThread); \
		     _pControlFileParser->setObjectName("ZsyncRemoteControlFileParserPrivate"); \
		     _pUpdateInformation->setObjectName("AppImageUpdateInformationPrivate"); \
		     _pUpdateInformation->setLoggerName(APPIMAGE_DELTA_WRITER); \
		     _pControlFileParser->setLoggerName(APPIMAGE_DELTA_WRITER); \
		     connect(_pUpdateInformation , &AppImageUpdateInformationPrivate::info , \
			      this , &AppImageDeltaWriter::embededInformation , Qt::DirectConnection); \
		     connect(_pUpdateInformation , &AppImageUpdateInformationPrivate::progress , \
	 		      this , &AppImageDeltaWriter::progress , Qt::DirectConnection); \
	             connect(_pUpdateInformation , &AppImageUpdateInformationPrivate::error , \
			      this , &AppImageDeltaWriter::error , Qt::DirectConnection); \
	             connect(_pUpdateInformation , &AppImageUpdateInformationPrivate::logger , \
			      this , &AppImageDeltaWriter::logger , Qt::DirectConnection); \
		     connect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::statusChanged ,\
			      this , &AppImageDeltaWriter::statusChanged , Qt::DirectConnection); \
		     connect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::error, \
			      this , &AppImageDeltaWriter::error , Qt::DirectConnection); \
		     connect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::progress , \
			      this , &AppImageDeltaWriter::progress , Qt::DirectConnection);  \
		     connect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::logger , \
			      this , &AppImageDeltaWriter::logger , Qt::DirectConnection); \
		     setAppImage(x);



AppImageDeltaWriter::AppImageDeltaWriter(QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(nullptr);
	return;
}

AppImageDeltaWriter::AppImageDeltaWriter(const QString &AppImagePath , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImagePath);
	return;
}

AppImageDeltaWriter::AppImageDeltaWriter(QFile *AppImage , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImage);
	return;
}



AppImageDeltaWriter::~AppImageDeltaWriter()
{
	_pUpdateInformation->deleteLater();
	_pControlFileParser->deleteLater();
	_pSharedNetworkAccessManager->deleteLater();
	_pSharedThread->quit();
	_pSharedThread->wait();
	_pSharedThread->deleteLater();
	return;
}

AppImageDeltaWriter &AppImageDeltaWriter::setAppImage(const QString &AppImagePath)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(const QString&)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection , Q_ARG(QString , AppImagePath));
	return *this;
}
AppImageDeltaWriter &AppImageDeltaWriter::setAppImage(QFile *AppImage)
{
	if(!AppImage){
	return *this;
	}
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(QFile*)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection , Q_ARG(QFile* , AppImage));
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::setShowLog(bool choice)
{
	{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection , Q_ARG(bool , choice));
	}
	{
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pControlFileParser , Qt::QueuedConnection , Q_ARG(bool , choice));
	
	}
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::getAppImageEmbededInformation(void)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getInfo(void)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection);
	return *this;
}

AppImageDeltaWriter &AppImageDeltaWriter::clear(void)
{
	_sLocalAppImagePath.clear();
	_sLocalAppImageSHA1Hash.clear();
	{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection);
	}
	{
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
		    .invoke(_pControlFileParser , Qt::QueuedConnection);	
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
	connect(_pUpdateInformation , &AppImageUpdateInformationPrivate::info , 
		this , &AppImageDeltaWriter::handleInfo);	
	getAppImageEmbededInformation();
	return *this; 
}

void AppImageDeltaWriter::handleInfo(QJsonObject information)
{
	disconnect(_pUpdateInformation , &AppImageUpdateInformationPrivate::info , 
		this , &AppImageDeltaWriter::handleInfo);
	
	_sLocalAppImagePath = (information["FileInformation"].toObject())["AppImageFilePath"].toString();
        _sLocalAppImageSHA1Hash = (information["FileInformation"].toObject())["AppImageSHA1Hash"].toString();

	connect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::receiveControlFile ,
		_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::getTargetFileSHA1);

	connect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::targetFileSHA1 ,
		this , &AppImageDeltaWriter::compareLocalAndRemoteAppImageSHA1Hash , Qt::QueuedConnection);

	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setControlFileUrl(QJsonObject)")))
		    .invoke(_pControlFileParser , Qt::QueuedConnection , Q_ARG(QJsonObject , information));
	return;	
}

void AppImageDeltaWriter::compareLocalAndRemoteAppImageSHA1Hash(QString RemoteAppImageSHA1Hash)
{
	disconnect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::receiveControlFile ,
		_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::getTargetFileSHA1);

	disconnect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::targetFileSHA1 ,
		this , &AppImageDeltaWriter::compareLocalAndRemoteAppImageSHA1Hash);
	emit updateAvailable((_sLocalAppImageSHA1Hash != RemoteAppImageSHA1Hash) , _sLocalAppImagePath);
	return;
}

QThread *AppImageDeltaWriter::sharedQThread(void) const
{
	return _pSharedThread;
}

QNetworkAccessManager *AppImageDeltaWriter::sharedNetworkAccessManager(void) const
{
	return _pSharedNetworkAccessManager;
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
