#ifndef RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#define RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#include <QObject>
#include <QUrl>
#include <QVector>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "rangereply.hpp"

class RangeDownloaderPrivate : public QObject {
	Q_OBJECT

	struct Range {
		Range(qint32 f = 0, qint32 t = 0, qint32 b = 0)
		 : from(f),
		   to(t),
		   blocks(b) { }
		qint32 from;
		qint32 to;
		qint32 blocks;
	};

public:
	RangeDownloaderPrivate(QNetworkAccessManager*, QObject *parent = nullptr);
	~RangeDownloaderPrivate();
public Q_SLOTS:
	void setTargetFileUrl(const QUrl&);
	void setFullDownload(bool);
	void appendRange(qint32, qint32,qint32);

	void start();
	void cancel();

private Q_SLOTS:
	QNetworkRequest makeRangeRequest(const QUrl&, const Range&);
	void handleUrlCheckError(QNetworkReply::NetworkError);
	void handleUrlCheck(qint64, qint64);
	void handleRangeReplyCancel(int);
	void handleRangeReplyRestart(int);
	void handleRangeReplyError(QNetworkReply::NetworkError, int);
	void handleRangeReplyFinished(qint32,qint32,qint32,QByteArray*, int);
Q_SIGNALS:
	void started();
	void canceled();
	void finished();
	void error(QNetworkReply::NetworkError);

	void data(QByteArray *);
	void rangeData(qint32, qint32, qint32, QByteArray *);
private:
	bool b_Finished = false,
	     b_Running = false,
	     b_CancelRequested = false,
	     b_FullDownload = false;
	int n_Active = -1,
	    n_Canceled = -1,
	    n_Done = 0;
	QUrl m_Url;


	QNetworkAccessManager *m_Manager;
	QVector<Range> m_RequiredRanges;
	QVector<RangeReply*> m_ActiveRequests;

};
#endif // RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
