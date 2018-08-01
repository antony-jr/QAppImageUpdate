#include <AppImageUpdateResource.hpp>
#include <AppImageUpdateResource_p.hpp>


#define APPIMAGE_UPDATE_INFORMATION QString("AppImageUpdateResource")
#define APPIMAGE_UPDATE_INFORMATION_PRIVATE QString("AppImageUpdateResourcePrivate")
#define APPIMAGE_UPDATE_INFORMATION_LOGGER_NAME APPIMAGE_UPDATE_INFORMATION

using namespace AppImageUpdaterBridge;


#define CONSTRUCT(x) _pSharedThread = new QThread;\
		     _pSharedThread->start(); \
		     _pSharedNetworkAccessManager = new QNetworkAccessManager; \
		     _pUpdateInformation = new AppImageUpdateResourcePrivate; \
		     _pSharedNetworkAccessManager->moveToThread(_pSharedThread); \
		     _pUpdateInformation->moveToThread(_pSharedThread); \
		     _pUpdateInformation->setObjectName(APPIMAGE_UPDATE_INFORMATION_PRIVATE); \
		     _pUpdateInformation->setLoggerName(APPIMAGE_UPDATE_INFORMATION_LOGGER_NAME); \
		     setObjectName(APPIMAGE_UPDATE_INFORMATION); \
		     connect(_pUpdateInformation , &AppImageUpdateResourcePrivate::info , \
			      this , &AppImageUpdateResource::info , Qt::DirectConnection); \
		     connect(_pUpdateInformation , &AppImageUpdateResourcePrivate::progress , \
	 		      this , &AppImageUpdateResource::progress , Qt::DirectConnection); \
	             connect(_pUpdateInformation , &AppImageUpdateResourcePrivate::error , \
			      this , &AppImageUpdateResource::error , Qt::DirectConnection); \
	             connect(_pUpdateInformation , &AppImageUpdateResourcePrivate::logger , \
			      this , &AppImageUpdateResource::logger , Qt::DirectConnection); \
		     setAppImage(x);


AppImageUpdateResource::AppImageUpdateResource(QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(nullptr);
	return;
}

AppImageUpdateResource::AppImageUpdateResource(const QString &AppImagePath , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImagePath);
	return;
}
 
AppImageUpdateResource::AppImageUpdateResource(QFile *AppImage , QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImage);
	return;
}
    
AppImageUpdateResource::~AppImageUpdateResource()
{
	_pSharedThread->quit();
	_pSharedThread->wait();
	_pSharedThread->deleteLater();
	delete _pSharedNetworkAccessManager;
	delete _pUpdateInformation;
	return;
}

AppImageUpdateResource &AppImageUpdateResource::setAppImage(const QString &AppImagePath)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(const QString&)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection , Q_ARG(QString , AppImagePath));
	return *this;
}
AppImageUpdateResource &AppImageUpdateResource::setAppImage(QFile *AppImage)
{
	if(!AppImage){
	return *this;
	}
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(QFile*)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection , Q_ARG(QFile* , AppImage));
	return *this;
}

AppImageUpdateResource &AppImageUpdateResource::setShowLog(bool choice)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection , Q_ARG(bool , choice));
	return *this;
}

AppImageUpdateResource &AppImageUpdateResource::getInfo(void)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getInfo(void)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection);
	return *this;
}	

AppImageUpdateResource &AppImageUpdateResource::clear(void)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection);
	return *this;
}

QThread *AppImageUpdateResource::sharedQThread(void) const 
{
	return _pSharedThread;
}

QNetworkAccessManager *AppImageUpdateResource::sharedQNetworkAccessManager(void) const
{
	return _pSharedNetworkAccessManager;
}

QString AppImageUpdateResource::errorCodeToString(short code)
{
	return AppImageUpdateResourcePrivate::errorCodeToString(code);
}
