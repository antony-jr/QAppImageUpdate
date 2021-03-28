#ifndef SEEDER_PRIVATE_HPP_INCLUDED
#define SEEDER_PRIVATE_HPP_INCLUDED
#ifdef DECENTRALIZED_UPDATE_ENABLED
#include <QObject>
#include <QScopedPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
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


class SeederPrivate : public QObject {
    Q_OBJECT
  public:
    SeederPrivate(QNetworkAccessManager*);
    ~SeederPrivate();
  public Q_SLOTS:
    void start(QJsonObject);
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
    void error(short);

    void logger(QString);
    void torrentStatus(int,int);
  private:
    bool b_Running = false,
         b_CancelRequested = false;

    QTimer m_Timer;
    QTimer m_TimeoutTimer;
    QString m_TargetFilePath;
    QNetworkAccessManager *m_Manager;
    QScopedPointer<QByteArray> m_TorrentMeta;
    QScopedPointer<lt::session> m_Session;
    lt::torrent_handle m_Handle;
};
#endif // DECENTRALIZED_UPDATE_ENABLED
#endif // SEEDER_PRIVATE_HPP_INCLUDED
