#include <QNAMHandler.hpp>

using namespace QNetworkAccessManagerHandler;
using namespace QNetworkAccessManagerHandler::Private;

QNAMHandler::QNAMHandler(QNetworkAccessManager *networkManager)
	: QObject(nullptr)
{
	_pHandler = QSharedPointer<QNAMHandlerPrivate>(new QNAMHandlerPrivate(networkManager));
	if(networkManager)
	{
		_pHandler->moveToThread(networkManager->thread());
	}
	connect(_pHandler.data() , &QNAMHandlerPrivate::getReply , this , &QNAMHandler::getReply , Qt::DirectConnection);
	return;
}

QNAMHandler::~QNAMHandler()
{
	_pHandler.clear();
	return;
}

void QNAMHandler::get(const QNetworkRequest &request)
{
	/*
	 * No need to use mutex since this will be executed in the caller
	 * threads stack and therefore any further requests as to be queued
	 * automatically.
	*/
	auto metaObject = _pHandler->metaObject();
	metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("get(const QNetworkRequest&)")))
		    	   .invoke(_pHandler.data() , Qt::QueuedConnection , Q_ARG(QNetworkRequest , request));
	return returnReply;
}
