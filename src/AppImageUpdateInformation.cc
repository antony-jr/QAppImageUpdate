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


#define CONSTRUCT(x) _pUpdateInformationParser = QSharedPointer<AppImageUpdateInformationPrivate>\
						 (new AppImageUpdateInformationPrivate); \
		     _pSharedThread = QSharedPointer<QThread>(new QThread , safeDeleteQThread); \
		     _pUpdateInformationParser->moveToThread(_pSharedThread.data()); \
		     _pUpdateInformationParser->setLoggerName("AppImageUpdateInformation"); \
		     connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::info , \
			      this , &AppImageUpdateInformation::info); \
		     connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::progress , \
	 		      this , &AppImageUpdateInformation::progress); \
	             connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::error , \
			      this , &AppImageUpdateInformation::error); \
	             connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::logger , \
			      this , &AppImageUpdateInformation::logger); \
		     _pSharedThread->start(); \
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
	_pSharedThread.clear();
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

void AppImageUpdateInformation::waitForSharedThread(void)
{
	_pSharedThread->wait();
	return;
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
