#ifndef QNETWORK_ACCESS_MANAGER_HANDLER_PRIVATE_HPP_INCLUDED
#define QNETWORK_ACCESS_MANAGER_HANDLER_PRIVATE_HPP_INCLUDED
#include <QtCore>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace QNetworkAccessManagerHandler {
namespace Private {
class QNAMHandlerPrivate : public QObject {
	Q_OBJECT
public:
	explicit QNAMHandlerPrivate(QNetworkAccessManager*);	
	~QNAMHandlerPrivate();
public Q_SLOTS:
	void get(const QNetworkRequest&);
Q_SIGNALS:
	void getReply(QNetworkReply*);
private:
	QNetworkAccessManager *_pManager = nullptr;
};
}
}
#endif
