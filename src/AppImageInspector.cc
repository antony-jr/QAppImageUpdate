#include <AppImageInspector.hpp>

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

AppImageInspector::AppImageInspector(AppImageUpdateInformation *updateInformation , QNetworkAccessManager *networkManager)
	: QObject(networkManager)
{
	_pUpdateInformation = (!updateInformation) ? new AppImageUpdateInformation(this) : updateInformation;
	connect(_pUpdateInformation , &AppImageUpdateInformation::error , 
		this , &AppImageInspector::handleUpdateInformationError);

	_pControlFileParser = QSharedPointer<ZsyncRemoteControlFileParserPrivate>
			      (new ZsyncRemoteControlFileParserPrivate(networkManager));
	_pControlFileParserThread = QSharedPointer<QThread>(new QThread , safeDeleteQThread);

	_pControlFileParser->setLoggerName("AppImageInspector");
	_pControlFileParser->moveToThread(_pControlFileParserThread.data());

	connect(_pUpdateInformation , SIGNAL(info(QJsonObject)) ,
		_pControlFileParser.data() , SLOT(setControlFileUrl(QJsonObject)));

	connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::error,
		this , &AppImageInspector::error);
	
	connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::progress ,
		this , &AppImageInspector::progress);

	connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::receiveControlFile ,
		this , &AppImageInspector::handleControlFile);
	
	connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::receiveTargetFileBlocks ,
		this , &AppImageInspector::targetFileCheckSumBlock);

	connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::logger ,
		this , &AppImageInspector::logger);

	connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::endOfTargetFileBlocks ,
		this , &AppImageInspector::endOfTargetFileCheckSumBlocks);

	_pControlFileParserThread->start();
	return;
}

AppImageInspector::~AppImageInspector()
{
	_pControlFileParserThread.clear();
	_pControlFileParser.clear();
	return;
}

bool AppImageInspector::isEmpty(void)
{
	bool ret = false;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("isEmpty(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(bool , ret));
	return ret;
}

bool AppImageInspector::isUpdatesAvailable(void)
{
	bool ret = false;
	QString targetFileSHA1Hash = getTargetFileSHA1(),
		appimageSHA1Hash = _pUpdateInformation->getAppImageSHA1();

	if(!targetFileSHA1Hash.isEmpty() && !appimageSHA1Hash.isEmpty()){
		ret = targetFileSHA1Hash.compare(appimageSHA1Hash);
	}
	return ret;
}

AppImageInspector &AppImageInspector::clear(void)
{
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
		    .invoke(_pControlFileParser.data() , Qt::QueuedConnection);
	return *this;
}

AppImageInspector &AppImageInspector::setShowLog(bool choice)
{
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
		    .invoke(_pControlFileParser.data() , Qt::QueuedConnection , Q_ARG(bool , choice));
	return *this;
}

AppImageInspector &AppImageInspector::checkForUpdates(void)
{
	_pUpdateInformation->getInfo();
	return *this;
}

AppImageInspector &AppImageInspector::getTargetFileCheckSumBlocks(void)
{
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getTargetFileBlocks(void)")))
		    .invoke(_pControlFileParser.data() , Qt::QueuedConnection);
	return *this;
}

size_t AppImageInspector::getTargetFileBlocksCount(void)
{
	size_t ret = 0;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getTargetFileBlocksCount(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(size_t , ret));
	return ret;
}
    	    
QUrl AppImageInspector::getControlFileUrl(void)
{
	QUrl ret;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getControlFileUrl(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(QUrl , ret));
	return ret;
}

QString AppImageInspector::getZsyncMakeVersion(void)
{
	QString ret;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getZsyncMakeVersion(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(QString , ret));
	return ret;
}

QString AppImageInspector::getTargetFileName(void)
{
	QString ret;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getTargetFileName(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(QString , ret));
	return ret;
}

QUrl AppImageInspector::getTargetFileUrl(void)
{
	QUrl ret;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getTargetFileUrl(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(QUrl , ret));
	return ret;
}

QString AppImageInspector::getTargetFileSHA1(void)
{
	QString ret;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getTargetFileSHA1(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(QString , ret));
	return ret;
}

QDateTime AppImageInspector::getMTime(void)
{
	QDateTime ret;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getMTime(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(QDateTime , ret));
	return ret;
}

size_t AppImageInspector::getTargetFileBlockSize(void)
{
	size_t ret = 0;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getTargetFileBlockSize(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(size_t , ret));
	return ret;
}

size_t AppImageInspector::getTargetFileLength(void)
{
	size_t ret = 0;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getTargetFileLength(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(size_t , ret));
	return ret;
}

qint32 AppImageInspector::getWeakCheckSumBytes(void)
{
	qint32 ret = 0;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getWeakCheckSumBytes(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(qint32 , ret));
	return ret;
}

qint32 AppImageInspector::getStrongCheckSumBytes(void)
{
	qint32 ret = 0;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getStrongCheckSumBytes(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(qint32 , ret));
	return ret;
}

qint32 AppImageInspector::getConsecutiveMatchNeeded(void)
{
	qint32 ret = 0;
	auto metaObject = _pControlFileParser->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getConsecutiveMatchNeeded(void)")))
		    .invoke(_pControlFileParser.data() , Qt::DirectConnection , Q_RETURN_ARG(qint32, ret));
	return ret;
}

void AppImageInspector::handleUpdateInformationError(short)
{
	emit error(APPIMAGE_UPDATE_INFORMATION_ERRORED);
	return;
}

void AppImageInspector::handleControlFile(void)
{
	bool result = isUpdatesAvailable();
	emit updatesAvailable(result);
	return;
}

QString AppImageInspector::errorCodeToString(short code)
{
	if(code == APPIMAGE_UPDATE_INFORMATION_ERRORED){
		return QString("AppImageInspector::error_code(APPIMAGE_UPDATE_INFORMATION_ERRORED)");
	}else{
		return ZsyncRemoteControlFileParserPrivate::errorCodeToString(code);
	}
	return QString();
}
