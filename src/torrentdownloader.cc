#ifdef DECENTRALIZED_UPDATE_ENABLED
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

    connect(obj, &TorrentDownloaderPrivate::logger,
            this, &TorrentDownloader::logger,
            Qt::DirectConnection);

    connect(obj, &TorrentDownloaderPrivate::progress,
            this, &TorrentDownloader::progress,
            Qt::DirectConnection);
}

void TorrentDownloader::setTargetFileDone(qint64 n) {
    getMethod(m_Private.data(), "setTargetFileDone(qint64)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(qint64,n));

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

void TorrentDownloader::setTargetFileUrl(const QUrl &url) {
    getMethod(m_Private.data(), "setTargetFileUrl(const QUrl&)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(QUrl,url));

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
#endif // DECENTRALIZED_UPDATE_ENABLED
