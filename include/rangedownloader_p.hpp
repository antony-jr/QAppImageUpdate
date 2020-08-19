#ifndef RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#define RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#include <QObject>
#include <QUrl>
#include <QVector>
#include <QNetworkReply>
#include <QScopedPointer>

class RangeDownloaderPrivate : public QObject {
	Q_OBJECT
public:
	RangeDownloaderPrivate(QObject *parent = nullptr);
	~RangeDownloaderPrivate();
public Q_SLOTS:
	void setTargetFileUrl(const QUrl&);
	void setFullDownload(bool);
	void setRequiredRanges(const QVector<QPair<qint32, qint32>>&);
	void appendRange(qint32, qint32);
	void appendRange(QPair<qint32, qint32>);

	void start();
	void cancel();

private Q_SLOTS:
	QNetworkRequest makeRangeRequest(const QUrl&, const QPair<qint32, qint32>&);
	void handleUrlCheckError(QNetworkReply::NetworkError);
	void handleUrlCheck(qint64, qint64);
	void handleRangeReplyCancel(int);
	void handleRangeReplyRestart(int);
	void handleRangeReplyError(QNetworkReply::NetworkError, int);
	void handleRangeReplyFinished(qint32,qint32, QByteArray*, int);
Q_SIGNALS:
	void started();
	void canceled();
	void finished();
	void error(QNetworkReply::NetworkError);

	void data(QByteArray *);
	void rangeData(qint32, qint32, QByteArray *);
private:
	bool b_Finished = false,
	     b_Running = false,
	     b_CancelRequested = false,
	     b_FullDownload = false;
	int n_Active = -1,
	    n_Canceled = -1;
	QUrl m_Url;

	QVector<QPair<qint32, qint32>> m_RequiredRanges;
	QVector<RangeReply*> m_ActiveRequests;

};
#endif // RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
