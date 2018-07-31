#include <AppImageUpdateInformation.hpp>

using namespace AppImageUpdaterBridge;
using namespace AppImageUpdaterBridge::Private;

static void safeDeleteQThread(QThread *thread)
{
	if(!thread){
		return;
	}

	thread->quit();
	thread->wait();
	thread->deleteLater();
	return;
}

static void doNotDelete(QNetworkAccessManager *m)
{
	(void)m;
	return;
}

#define CONSTRUCT(x) _pUpdateInformationParser = QSharedPointer<AppImageUpdateInformationPrivate>\
						 (new AppImageUpdateInformationPrivate); \
		     _pSharedThread = QSharedPointer<QThread>(new QThread , safeDeleteQThread); \
		     _pSharedNetworkManager = (!networkManager) ?  \
					      QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager) : \
					      QSharedPointer<QNetworkAccessManager>(networkManager , doNotDelete); \
		     _pUpdateInformationParser->moveToThread(_pSharedThread.data()); \
		     _pSharedNetworkManager->moveToThread(_pSharedThread.data()); \
		     _pUpdateInformationParser->setLoggerName("AppImageUpdateInformation"); \
		     connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::info , \
			      this , &AppImageUpdateInformation::info , Qt::DirectConnection); \
		     connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::progress , \
	 		      this , &AppImageUpdateInformation::progress , Qt::DirectConnection); \
	             connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::error , \
			      this , &AppImageUpdateInformation::error , Qt::DirectConnection); \
	             connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::logger , \
			      this , &AppImageUpdateInformation::logger , Qt::DirectConnection); \
		     _pSharedThread->start(); \
		     setAppImage(x);


AppImageUpdateInformation::AppImageUpdateInformation(QNetworkAccessManager *networkManager)
	: QObject()
{
	CONSTRUCT(nullptr);
	return;
}

AppImageUpdateInformation::AppImageUpdateInformation(const QString &AppImagePath, QNetworkAccessManager *networkManager)
	: QObject()
{
	CONSTRUCT(AppImagePath);
	return;
}
 
AppImageUpdateInformation::AppImageUpdateInformation(QFile *AppImage, QNetworkAccessManager *networkManager)
	: QObject()
{
	CONSTRUCT(AppImage);
	return;
}
    
AppImageUpdateInformation::~AppImageUpdateInformation()
{
	_pSharedThread.clear();
	_pSharedNetworkManager.clear();
	_pUpdateInformationParser.clear();
	return;
}

void AppImageUpdateInformation::shareThreadWith(QObject *other)
{
	if(!other){
		return;
	}
	other->moveToThread(_pSharedThread.data());
	return;
}

QNetworkAccessManager *AppImageUpdateInformation::getSharedNetworkManager(void)
{
	return _pSharedNetworkManager.data();
}

bool AppImageUpdateInformation::isEmpty(void)
{
	bool ret = false;
	auto metaObject = _pUpdateInformationParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("isEmpty(void)")))
		    .invoke(_pUpdateInformationParser.data() , Qt::DirectConnection , Q_RETURN_ARG(bool , ret));
	return ret;
}

AppImageUpdateInformation &AppImageUpdateInformation::setAppImage(const QString &AppImagePath)
{
	auto metaObject = _pUpdateInformationParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(const QString&)")))
		    .invoke(_pUpdateInformationParser.data() , Qt::QueuedConnection , Q_ARG(QString , AppImagePath));
	return *this;
}
AppImageUpdateInformation &AppImageUpdateInformation::setAppImage(QFile *AppImage)
{
	if(!AppImage){
	return *this;
	}
	auto metaObject = _pUpdateInformationParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(QFile*)")))
		    .invoke(_pUpdateInformationParser.data() , Qt::QueuedConnection , Q_ARG(QFile* , AppImage));
	return *this;
}

AppImageUpdateInformation &AppImageUpdateInformation::setShowLog(bool choice)
{
	auto metaObject = _pUpdateInformationParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pUpdateInformationParser.data() , Qt::QueuedConnection , Q_ARG(bool , choice));
	return *this;
}

AppImageUpdateInformation &AppImageUpdateInformation::getInfo(void)
{
	auto metaObject = _pUpdateInformationParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getInfo(void)")))
		    .invoke(_pUpdateInformationParser.data() , Qt::QueuedConnection);
	return *this;
}	

QString AppImageUpdateInformation::getAppImageSHA1(void)
{
	QString ret;
	auto metaObject = _pUpdateInformationParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getAppImageSHA1(void)")))
		    .invoke(_pUpdateInformationParser.data() , Qt::DirectConnection , Q_RETURN_ARG(QString, ret));
	return ret;
}

QString AppImageUpdateInformation::getAppImageName(void)
{
	QString ret;
	auto metaObject = _pUpdateInformationParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getAppImageName(void)")))
		    .invoke(_pUpdateInformationParser.data() , Qt::DirectConnection , Q_RETURN_ARG(QString, ret));
	return ret;
}

QString AppImageUpdateInformation::getAppImagePath(void)
{
	QString ret;
	auto metaObject = _pUpdateInformationParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getAppImagePath(void)")))
		    .invoke(_pUpdateInformationParser.data() , Qt::DirectConnection , Q_RETURN_ARG(QString, ret));
	return ret;
}

AppImageUpdateInformation &AppImageUpdateInformation::clear(void)
{
	auto metaObject = _pUpdateInformationParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
		    .invoke(_pUpdateInformationParser.data() , Qt::QueuedConnection);
	return *this;
}

QString AppImageUpdateInformation::errorCodeToString(short code)
{
	return AppImageUpdateInformationPrivate::errorCodeToString(code);
}
