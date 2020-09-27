#ifndef TORRENT_DOWNLOADER_PRIVATE_HPP_INCLUDED
#define TORRENT_DOWNLOADER_PRIVATE_HPP_INCLUDED
#ifdef DECENTRALIZED_UPDATE_ENABLED
#include <QObject>
#include <QScopedPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTemporaryFile>
#include <QTimer>

#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/error_code.hpp>


class TorrentDownloaderPrivate : public QObject {
    Q_OBJECT
  public:
    TorrentDownloaderPrivate(QNetworkAccessManager*);
    ~TorrentDownloaderPrivate();
  public Q_SLOTS:
    void setTargetFileDone(qint64);
    void setTargetFileLength(qint64);
    void setTargetFile(QTemporaryFile*);
    void setTorrentFileUrl(const QUrl&);
    void setTargetFileUrl(const QUrl&);

    void start();
    void cancel();

  private Q_SLOTS:
    void handleTorrentFileData(qint64, qint64);
    void handleTorrentFileError(QNetworkReply::NetworkError);
    void handleTorrentFileFinish();
    void handleTimeout();

    void torrentLoop();

  Q_SIGNALS:
    void started();
    void canceled();
    void finished();
    void error(QNetworkReply::NetworkError);

    void logger(QString);
    void progress(int, qint64, qint64, double, QString);
  private:
    bool b_Finished = false,
         b_Running = false,
         b_CancelRequested = false;

    qint64 n_TargetFileLength,
           n_TargetFileDone;
    QTimer m_Timer;
    QTimer m_TimeoutTimer;
    QUrl m_TorrentFileUrl,
         m_TargetFileUrl;
    QTemporaryFile *m_File;
    QNetworkAccessManager *m_Manager;
    QScopedPointer<QByteArray> m_TorrentMeta;
    QScopedPointer<lt::session> m_Session;
    lt::torrent_handle m_Handle;
};
#endif // DECENTRALIZED_UPDATE_ENABLED
#endif // TORRENT_DOWNLOADER_PRIVATE_HPP_INCLUDED
