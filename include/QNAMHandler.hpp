#ifndef QNETWORK_ACCESS_MANAGER_HANDLER_HPP_INCLUDED
#define QNETWORK_ACCESS_MANAGER_HANDLER_HPP_INCLUDED
#include <QNAMHandler_p.hpp>

/*
 * Important:  QNAMHandler has to be contructed in the thread
 * where you want to use QNetworkAccessManager.
 * Normally when you use QNetworkAccessManager , Your network manager
 * and all operations you do with your network manager has to be in
 * the same thread and therefore defeats the purpose of QNetworkAccessManager's
 * ability to work accross multiple threads. And also we only need a single QNetworkAccessManager
 * for an entire Qt Application , Thus with QNAMHandler , You can use any QNetworkAccessManager's operation 
 * from a thread where you constructed QNAMHandler. 
 * To construct a QNAMHandler you only need a pointer to a QNetworkAccessManager.
 *
 * Note: This class is exclusively for private use only.
*/

namespace QNetworkAccessManagerHandler {
class QNAMHandler : public QObject {
	Q_OBJECT
public:
	explicit QNAMHandler(QNetworkAccessManager*);
	~QNAMHandler();
public Q_SLOTS:
	void get(const QNetworkRequest&);
Q_SIGNALS:
	void getReply(QNetworkReply*);
private:
	QSharedPointer<Private::QNAMHandlerPrivate> _pHandler = nullptr;
};
}
#endif
