#include <AppImageUpdateInformation.hpp>
#include <AppImageUpdateInformation_p.hpp>

#define APPIMAGE_UPDATE_INFORMATION QString("AppImageUpdateInformation")
#define APPIMAGE_UPDATE_INFORMATION_PRIVATE QString("AppImageUpdateInformationPrivate")
#define APPIMAGE_UPDATE_INFORMATION_LOGGER_NAME APPIMAGE_UPDATE_INFORMATION

using namespace AppImageUpdaterBridge;


#define CONSTRUCT(x) _pUpdateInformation = new AppImageUpdateInformationPrivate(this); \
		     _pUpdateInformation->setObjectName(APPIMAGE_UPDATE_INFORMATION_PRIVATE); \
		     _pUpdateInformation->setLoggerName(APPIMAGE_UPDATE_INFORMATION_LOGGER_NAME); \
		     setObjectName(APPIMAGE_UPDATE_INFORMATION); \
		     connect(_pUpdateInformation , &AppImageUpdateInformationPrivate::info , \
			      this , &AppImageUpdateInformation::info , Qt::DirectConnection); \
		     connect(_pUpdateInformation , &AppImageUpdateInformationPrivate::progress , \
	 		      this , &AppImageUpdateInformation::progress , Qt::DirectConnection); \
	             connect(_pUpdateInformation , &AppImageUpdateInformationPrivate::error , \
			      this , &AppImageUpdateInformation::error , Qt::DirectConnection); \
	             connect(_pUpdateInformation , &AppImageUpdateInformationPrivate::logger , \
			      this , &AppImageUpdateInformation::logger , Qt::DirectConnection); \
		     setAppImage(x);


AppImageUpdateInformation::AppImageUpdateInformation(QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(nullptr);
	return;
}

AppImageUpdateInformation::AppImageUpdateInformation(const QString &AppImagePath, QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImagePath);
	return;
}
 
AppImageUpdateInformation::AppImageUpdateInformation(QFile *AppImage, QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(AppImage);
	return;
}
    
AppImageUpdateInformation::~AppImageUpdateInformation()
{
	/*
	 * No need to deallocate anything because of Qt Parent to Child
	 * deallocation.
	 * i.e., it will automatically delete its children in its destructor. 
	 * You can look for an object by name and optionally type using findChild() or findChildren().
	 *
	 * See https://doc.qt.io/qt-5/qobject.html#details.
	*/
	return;
}

AppImageUpdateInformation &AppImageUpdateInformation::setAppImage(const QString &AppImagePath)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(const QString&)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection , Q_ARG(QString , AppImagePath));
	return *this;
}
AppImageUpdateInformation &AppImageUpdateInformation::setAppImage(QFile *AppImage)
{
	if(!AppImage){
	return *this;
	}
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(QFile*)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection , Q_ARG(QFile* , AppImage));
	return *this;
}

AppImageUpdateInformation &AppImageUpdateInformation::setShowLog(bool choice)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection , Q_ARG(bool , choice));
	return *this;
}

AppImageUpdateInformation &AppImageUpdateInformation::getInfo(void)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getInfo(void)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection);
	return *this;
}	

AppImageUpdateInformation &AppImageUpdateInformation::clear(void)
{
	auto metaObject = _pUpdateInformation->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
		    .invoke(_pUpdateInformation , Qt::QueuedConnection);
	return *this;
}

QString AppImageUpdateInformation::errorCodeToString(short code)
{
	return AppImageUpdateInformationPrivate::errorCodeToString(code);
}
