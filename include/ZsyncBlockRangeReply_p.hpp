#ifndef ZSYNC_BLOCK_RANGE_REPLY_PRIVATE_HPP_INCLUDED
#define ZSYNC_BLOCK_RANGE_REPLY_PRIVATE_HPP_INCLUDED
#include <QtGlobal>
#include <QObject>
#include <QNetworkReply>
namespace AppImageUpdaterBridge {
class ZsyncWriterPrivate;
class ZsyncBlockRangeReplyPrivate : public QObject {
	Q_OBJECT
public:
	ZsyncBlockRangeReplyPrivate(ZsyncWriterPrivate*,QNetworkReply*,qint32,qint32);
	~ZsyncBlockRangeReplyPrivate();

public Q_SLOTS:
	void cancel(void);

private Q_SLOTS:	
	void handleError(QNetworkReply::NetworkError);
	void handleFinished(void);
	void handleProgress(qint64 , qint64);

Q_SIGNALS:
	void cancelReply(void);
	void canceled(void);
	void progress(qint64 , double , QString);
	void error(QNetworkReply::NetworkError);
	void finished(void);
	void sendBlockDataToWriter(qint32 , qint32 , QByteArray *);

private:
	QTime downloadSpeed;
	qint64 _nPreviousBytesReceived = 0;
	qint32 _nRangeFrom = 0,
	       _nRangeTo = 0;
};
}
#endif // ZSYNC_BLOCK_RANGE_REPLY_PRIVATE_HPP_INCLUDED
