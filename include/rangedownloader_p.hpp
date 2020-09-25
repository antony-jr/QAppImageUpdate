#ifndef RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#define RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#include <QObject>
#include <QUrl>
#include <QVector>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QElapsedTimer>

#include "rangereply.hpp"

class RangeDownloaderPrivate : public QObject {
	Q_OBJECT
public:
	RangeDownloaderPrivate(QNetworkAccessManager*, QObject *parent = nullptr);
	~RangeDownloaderPrivate();
public Q_SLOTS:
	void setBlockSize(qint32);
	void setTargetFileUrl(const QUrl&);
	void setBytesWritten(qint64);
	void setTargetFileLength(qint32);
	void setFullDownload(bool);
	void appendRange(qint32, qint32);

	void start();
	void cancel();

private Q_SLOTS:
	QNetworkRequest makeRangeRequest(const QUrl&, const QPair<qint32,qint32>&);
	void handleUrlCheckError(QNetworkReply::NetworkError);
	void handleUrlCheck(qint64, qint64);
	void handleRangeReplyCancel(int);
	void handleRangeReplyRestart(int);
	void handleRangeReplyProgress(qint64, int);
	void handleRangeReplyError(QNetworkReply::NetworkError, int, bool);
	void handleRangeReplyFinished(qint32,qint32,QByteArray*, int);
Q_SIGNALS:
	void started();
	void canceled();
	void finished();
	void error(QNetworkReply::NetworkError);

	void data(QByteArray *, bool);
	void rangeData(qint32, qint32, QByteArray *, /*this is true when the given range is the last one*/bool);

	void progress(int, qint64, qint64, double, QString);
private:
	bool b_Finished = false,
	     b_Running = false,
	     b_CancelRequested = false,
	     b_FullDownload = false;
	int n_Active = -1,
	    n_Done = 0;
	QUrl m_Url;
	qint32 n_BlockSize = 1024;
	qint64 n_BytesWritten = 0;
	qint64 n_TotalSize = -1;
	qint64 n_RecievedBytes;
	
	QNetworkAccessManager *m_Manager;
	QElapsedTimer m_ElapsedTimer;
	QVector<QPair<qint32, qint32>> m_RequiredBlocks;
	QVector<RangeReply*> m_ActiveRequests;

};
#endif // RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
