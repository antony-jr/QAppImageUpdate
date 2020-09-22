#ifndef TORRENT_DOWNLOADER_HPP_INCLUDED
#define TORRENT_DOWNLOADER_HPP_INCLUDED
#include <QObject>
#include <QSharedPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTemporaryFile>

class TorrentDownloaderPrivate;

class TorrentDownloader : public QObject { 
	Q_OBJECT
	QSharedPointer<TorrentDownloaderPrivate> m_Private;
public:
	TorrentDownloader(QNetworkAccessManager*, QObject *parent = nullptr);
public Q_SLOTS:
	void setTargetFileLength(qint64);
	void setTargetFile(QTemporaryFile*);
	void setTargetFileName(const QString&);
	void setTorrentFileUrl(const QUrl&);
	
	void start();
	void cancel();
Q_SIGNALS:
	void started();
	void canceled();
	void finished();
	void error(QNetworkReply::NetworkError);

	void progress(int, qint64, qint64, double, QString);
};
#endif // RANGE_DOWNLOADER_HPP_INCLUDED
