#include "torrentdownloader.hpp"
#include "torrentdownloader_p.hpp"
#include "helpers_p.hpp"

TorrentDownloader::TorrentDownloader(QNetworkAccessManager *manager, QObject *parent) 
		: QObject(parent) {
		m_Private = QSharedPointer<TorrentDownloaderPrivate>(new TorrentDownloaderPrivate(manager));
		auto obj = m_Private.data();

		connect(obj, &TorrentDownloaderPrivate::started, 
			 this, &TorrentDownloader::started,
			 Qt::DirectConnection);

		connect(obj, &TorrentDownloaderPrivate::canceled, 
			 this, &TorrentDownloader::canceled,
			 Qt::DirectConnection);

		connect(obj, &TorrentDownloaderPrivate::finished, 
			this, &TorrentDownloader::finished,
			 Qt::DirectConnection);

		connect(obj, &TorrentDownloaderPrivate::error, 
			 this, &TorrentDownloader::error,
			  Qt::DirectConnection);

		connect(obj, &TorrentDownloaderPrivate::progress,
			 this, &TorrentDownloader::progress,
			 Qt::DirectConnection);
}

void TorrentDownloader::setTargetFileLength(qint64 n) {
    getMethod(m_Private.data(), "setTargetFileLength(qint64)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection,
                    Q_ARG(qint64,n));

}



void TorrentDownloader::setTorrentFileUrl(const QUrl &url) {
    getMethod(m_Private.data(), "setTorrentFileUrl(const QUrl&)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection,
                    Q_ARG(QUrl,url));

}

void TorrentDownloader::setTargetFileName(const QString &name) {
    getMethod(m_Private.data(), "setTargetFileName(const QString&)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection,
                    Q_ARG(QString,name));

}


void TorrentDownloader::setTargetFile(QTemporaryFile *file) {
    getMethod(m_Private.data(), "setTargetFile(QTemporaryFile*)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection,
                    Q_ARG(QTemporaryFile*,file));

}

void TorrentDownloader::start() {
    getMethod(m_Private.data(), "start(void)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection);
}

void TorrentDownloader::cancel() {
    getMethod(m_Private.data(), "cancel(void)")
	    .invoke(m_Private.data(),
                    Qt::QueuedConnection);
}

