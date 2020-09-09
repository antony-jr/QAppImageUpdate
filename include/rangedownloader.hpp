#ifndef RANGE_DOWNLOADER_HPP_INCLUDED
#define RANGE_DOWNLOADER_HPP_INCLUDED
#include <QObject>
#include <QSharedPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class RangeDownloaderPrivate;

class RangeDownloader : public QObject { 
	Q_OBJECT
	QSharedPointer<RangeDownloaderPrivate> m_Private;
public:
	RangeDownloader(QNetworkAccessManager*, QObject *parent = nullptr);
public Q_SLOTS:
	void setTargetFileUrl(const QUrl&);
	void setFullDownload(bool);
	void appendRange(qint32, qint32);

	void start();
	void cancel();
Q_SIGNALS:
	void started();
	void canceled();
	void finished();
	void error(QNetworkReply::NetworkError);

	void data(QByteArray *);
	void rangeData(qint32, qint32, QByteArray *);
};
#endif // RANGE_DOWNLOADER_HPP_INCLUDED
