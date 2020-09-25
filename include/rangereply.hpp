#ifndef RANGE_REPLY_HPP_INCLUDED
#define RANGE_REPLY_HPP_INCLUDED
#include <QObject>
#include <QSharedPointer>
#include <QNetworkReply>

class RangeReplyPrivate; // Forward Declare.

class RangeReply : public QObject { 
	Q_OBJECT
	QSharedPointer<RangeReplyPrivate> m_Private;
public:
	RangeReply(int, QNetworkReply*, const QPair<qint32, qint32>&);
	~RangeReply();
public Q_SLOTS:
	void destroy();
	void retry(int timeout = 3000);
	void cancel();
Q_SIGNALS:
	void restarted(int);
	void error(QNetworkReply::NetworkError, int, bool);
	void progress(qint64, int);
	void data(QByteArray*, bool);
	void finished(qint32,qint32,  QByteArray*, int);
	void canceled(int);
};
#endif // RANGE_REPLY_HPP_INCLUDED
