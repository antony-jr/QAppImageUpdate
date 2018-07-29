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
		     _pUpdateInformationParserThread = QSharedPointer<QThread>(new QThread , safeDeleteQThread); \
		     _pUpdateInformationParser->moveToThread(_pUpdateInformationParserThread.data()); \
		     connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::info , \
			      this , &AppImageUpdateInformation::info); \
		     connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::progress , \
	 		      this , &AppImageUpdateInformation::progress); \
	             connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::error , \
			      this , &AppImageUpdateInformation::error); \
	             connect(_pUpdateInformationParser.data() , &AppImageUpdateInformationPrivate::logger , \
			      this , &AppImageUpdateInformation::logger); \
		     _pUpdateInformationParserThread->start(); \
		     setAppImage(x);


AppImageUpdateInformation::AppImageUpdateInformation(QObject *parent)
	: QObject(parent)
{
	CONSTRUCT(nullptr);
	return;
}

AppImageUpdateInformation::AppImageUpdateInformation(const QString &AppImagePath, QObject *parent)
{
	CONSTRUCT(AppImagePath);
	return;
}
 
AppImageUpdateInformation::AppImageUpdateInformation(QFile *AppImage, QObject *parent)
{
	CONSTRUCT(AppImage);
	return;
}
    
AppImageUpdateInformation::~AppImageUpdateInformation()
{
	_pUpdateInformationParserThread.clear();
	_pUpdateInformationParser.clear();
	return;
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
