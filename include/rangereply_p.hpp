#ifndef RANGE_REPLY_PRIVATE_HPP_INCLUDED
#define RANGE_REPLY_PRIVATE_HPP_INCLUDED
#include <QObject>
#include <QUrl>
#include <QVector>
#include <QByteArray>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScopedPointer>


class RangeReplyPrivate : public QObject {
	Q_OBJECT 
public:
	RangeReplyPrivate(int, QNetworkReply*, const QPair<qint32, qint32>&, qint32);
	~RangeReplyPrivate();

public Q_SLOTS:
	void destroy();
	void retry(int);
	void cancel();

private Q_SLOTS:
	void resetInternalFlags(bool value = false);
	void restart();
	void handleData(qint64, qint64);
	void handleError(QNetworkReply::NetworkError);
	void handleFinish();
Q_SIGNALS:
	void restarted(int);
	void error(QNetworkReply::NetworkError, int);
	void finished(qint32, qint32,qint32, QByteArray*, int);
	void canceled(int);
private:
	bool b_Running = false,
	     b_Finished = false,
	     b_Canceled = false,
	     b_CancelRequested = false,
	     b_Retrying = false,
	     b_Halted = false;
	int n_Index;
	qint32 n_FromRange,
	       n_ToRange;
	qint32 n_Blocks;
	QTimer m_Timer;
	QScopedPointer<QNetworkReply> m_Reply;
	QNetworkRequest m_Request;
	QNetworkAccessManager *m_Manager;
	QScopedPointer<QByteArray> m_Data;
};
#endif // RANGE_REPLY_PRIVATE_INCLUDED
