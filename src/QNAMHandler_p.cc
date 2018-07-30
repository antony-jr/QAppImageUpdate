#include <QNAMHandler_p.hpp>

using namespace QNetworkAccessManagerHandler::Private;

QNAMHandlerPrivate::QNAMHandlerPrivate(QNetworkAccessManager *networkManager)
	: QObject(nullptr)
{
	_pManager = networkManager;
	return;
}
	
QNAMHandlerPrivate::~QNAMHandlerPrivate()
{
	return;
}

void QNAMHandlerPrivate::get(const QNetworkRequest &request)
{
	if(!_pManager){
		emit getReply(nullptr);
	}
	auto reply = _pManager->get(request);
	emit getReply(reply);
	return;
}
