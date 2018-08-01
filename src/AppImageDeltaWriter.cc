#include <ZsyncRemoteControlFileParser_p.hpp>
#include <AppImageDeltaWriter.hpp>

#define APPIMAGE_DELTA_WRITER QString("AppImageDeltaWriter")

using namespace AppImageUpdaterBridge;

AppImageDeltaWriter::AppImageDeltaWriter(AppImageUpdateResource *resource)
	: QObject(resource),
	  _pResource(resource),
	  _pControlFileParser(new ZsyncRemoteControlFileParserPrivate(resource->sharedQNetworkAccessManager()))
{
	setObjectName(APPIMAGE_DELTA_WRITER);
	_pControlFileParser->moveToThread(_pResource->sharedQThread());
	_pControlFileParser->setLoggerName(APPIMAGE_DELTA_WRITER);

	connect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::error,
		this , &AppImageDeltaWriter::error , Qt::DirectConnection);
	
	connect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::progress ,
		this , &AppImageDeltaWriter::progress , Qt::DirectConnection);
	
	connect(_pControlFileParser , &ZsyncRemoteControlFileParserPrivate::logger ,
		this , &AppImageDeltaWriter::logger , Qt::DirectConnection);
	return;
}

AppImageDeltaWriter::~AppImageDeltaWriter()
{
	_pControlFileParser->deleteLater();
	return;
}

AppImageDeltaWriter &AppImageDeltaWriter::checkForUpdate(void)
{ 
	/*
	 * First we need the resource from the 
	 * AppImage binary , such as the update information 
	 * and the AppImage binary's checksums(SHA1 Hash) and full path.
	*/
	connect(_pResource , &AppImageUpdateResource::info , this , &AppImageDeltaWriter::handleInfo);
	_pResource->getInfo();
	return *this; 
}

void AppImageDeltaWriter::handleUpdateResourceError(short code)
{
	(void)code;
	emit error(APPIMAGE_UPDATE_RESOURCE_ERRORED);
	return;
}

void AppImageDeltaWriter::handleInfo(QJsonObject information)
{
	disconnect(_pResource , &AppImageUpdateResource::info , this , &AppImageDeltaWriter::handleInfo);	
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

QString AppImageDeltaWriter::errorCodeToString(short code)
{
	if(code == APPIMAGE_UPDATE_RESOURCE_ERRORED){
		return QString("AppImageDeltaWriter::error_code(APPIMAGE_UPDATE_RESOURCE_ERRORED)");
	}else{
		return ZsyncRemoteControlFileParserPrivate::errorCodeToString(code);
	}
	return QString();
}
