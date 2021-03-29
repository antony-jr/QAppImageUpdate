#ifdef DECENTRALIZED_UPDATE_ENABLED
#include <QFileInfo>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QNetworkProxy>
#include <vector>
#include <iostream>

#include "helpers_p.hpp"
#include "qappimageupdateenums.hpp"
#include "seeder_p.hpp"

SeederPrivate::SeederPrivate(QNetworkAccessManager *manager)
    : QObject() {
    lt::session_params p = lt::session_params();
    p.settings.set_int(lt::settings_pack::alert_mask,
                       lt::alert_category::status |
                       lt::alert_category::error |
                       lt::alert_category::storage);
    
    //// Set proxy for libtorrent.
    auto proxy = manager->proxy();
    if(proxy.type() != QNetworkProxy::NoProxy) {
	emit logger("Using proxy for torrent seeding.");
	p.settings.set_str(lt::settings_pack::proxy_hostname,
			   proxy.hostName().toStdString());
	p.settings.set_int(lt::settings_pack::proxy_port,
			   (int)proxy.port());
	p.settings.set_str(lt::settings_pack::proxy_username,
			   proxy.user().toStdString());
	p.settings.set_str(lt::settings_pack::proxy_password,
			   proxy.password().toStdString());
	
	/// Set Proxy type.
	if(proxy.type() == QNetworkProxy::Socks5Proxy) {
		p.settings.set_int(lt::settings_pack::proxy_type,
				   lt::settings_pack::socks5_pw);
	}else if(proxy.type() == QNetworkProxy::HttpProxy) {
		p.settings.set_int(lt::settings_pack::proxy_type,
				   lt::settings_pack::http_pw);
	}else{
		emit logger("Cannot find proxy type. Failed to set proxy.");
	}
    }

    m_Manager = manager;
    m_Session.reset(new lt::session(p));
    m_TorrentMeta.reset(new QByteArray);

    m_TimeoutTimer.setSingleShot(true);
    m_TimeoutTimer.setInterval(100 * 1000); // 100 seconds

    connect(&m_TimeoutTimer, &QTimer::timeout,
            this, &SeederPrivate::handleTimeout,
            Qt::QueuedConnection);

    m_Timer.setSingleShot(false);
    m_Timer.setInterval(100); // 1ms?
    connect(&m_Timer, &QTimer::timeout,
            this, &SeederPrivate::torrentLoop,
            Qt::QueuedConnection);
}

SeederPrivate::~SeederPrivate() {
	m_Session->abort();
}

void SeederPrivate::start(QJsonObject info) {
    if(b_Running) {
        return;
    }
    
    if(info.isEmpty()) {
	emit error(QAppImageUpdateEnums::Error::ProtocolFailure);
        return;
    }

    b_Running = false;

    auto embeddedUpdateInformation = info["EmbededUpdateInformation"].toObject();
    auto oldVersionInformation = embeddedUpdateInformation["FileInformation"].toObject();

    QString remoteTargetFileSHA1Hash = info["RemoteTargetFileSHA1Hash"].toString(),
            localAppImageSHA1Hash = oldVersionInformation["AppImageSHA1Hash"].toString(),
            localAppImagePath = oldVersionInformation["AppImageFilePath"].toString();

    bool torrentSupported = info["TorrentSupported"].toBool();
    auto torrentFileUrl = info["TorrentFileUrl"].toString();
    auto targetFileName = info["RemoteTargetFileName"].toString();

    if(localAppImageSHA1Hash != remoteTargetFileSHA1Hash) {
	    emit error(QAppImageUpdateEnums::Error::OutdatedAppImageForSeed);
	    return;
    }

    if(!torrentSupported) {
	    emit error(QAppImageUpdateEnums::Error::TorrentNotSupported);
	    return;
    }

    m_TargetFilePath = localAppImagePath;

    m_TorrentMeta->clear();

    QNetworkRequest request;
    request.setUrl(torrentFileUrl);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    auto reply = m_Manager->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleTorrentFileError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(handleTorrentFileData(qint64, qint64)));
    connect(reply, SIGNAL(finished()),
            this, SLOT(handleTorrentFileFinish()));
    b_Running = true;
    emit started();
}

void SeederPrivate::cancel() {
    if(!b_Running || b_CancelRequested) {
        return;
    }
    b_CancelRequested = true;
}

void SeederPrivate::handleTorrentFileError(QNetworkReply::NetworkError code) {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if(!reply) {
        return;
    }

    reply->disconnect();
    reply->deleteLater();

    emit error(translateQNetworkReplyError(code));
}

void SeederPrivate::handleTorrentFileData(qint64 br, qint64 bt) {
    Q_UNUSED(br);
    Q_UNUSED(bt);

    auto reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if(!reply) {
        return;
    }

    if(reply->error() != QNetworkReply::NoError) {
        return;
    }

    if(reply->isReadable()) {
        m_TorrentMeta->append(reply->readAll());
    }
}

void SeederPrivate::handleTorrentFileFinish() {
    auto reply = qobject_cast<QNetworkReply*>(QObject::sender());
    m_TorrentMeta->append(reply->readAll());

    reply->disconnect();
    reply->deleteLater();

    if(b_CancelRequested) {
        b_CancelRequested = false;
        b_Running = false;
        emit canceled();
        return;
    }

    lt::add_torrent_params params;
    QString savePath = QFileInfo(m_TargetFilePath).path() + "/";

    params.save_path = savePath.toStdString();
    auto ti = std::make_shared<lt::torrent_info>(m_TorrentMeta->constData(), (int)m_TorrentMeta->size());

    /// We know that MakeAppImageTorrent only packs a single file that is the
    /// the Target AppImage. So We just need to check if it is bundled correctly.
    if(ti->num_files() != 1) {
        emit error(QNetworkReply::ProtocolFailure);
        return;
    }

    /// Since only 1 file is packaged in the torrent, we can
    /// assume that the file index for our Target AppImage is 0
    ti->rename_file(0,
                    QFileInfo(m_TargetFilePath).fileName().toStdString());


    //ti->add_url_seed(m_TargetFileUrl.toString().toStdString());

    params.ti = ti;
    m_Handle = m_Session->add_torrent(params);
    if(!m_Handle.is_valid()) {
        emit error(QAppImageUpdateEnums::Error::ProtocolFailure);
        return;
    }

    m_Timer.setSingleShot(false);
    m_Timer.setInterval(100);
    m_Timer.start();
    m_TimeoutTimer.start();
    return;
}

void SeederPrivate::handleTimeout() {
    m_TimeoutTimer.stop();

    emit logger(QString::fromStdString(" handleTimeout: Torrent Seeder Timeout, failing."));
    m_Session->abort();
    b_Running = false;
    emit error(QAppImageUpdateEnums::Error::ProtocolFailure);
}

void SeederPrivate::torrentLoop() {
    if(!b_Running) {
        /// To avoid queued calls from being called
        return;
    }
    if(b_CancelRequested) {
        m_TimeoutTimer.stop();
        m_Timer.stop();
        {
	// The destruction of session proxy 
	// assures that all call writes and everything
	// is finished. This is sync.
	auto sess_proxy = m_Session->abort();
	}  
        b_CancelRequested = false;
        b_Running = false;
        emit canceled();
        return;
    }
    auto status = m_Handle.status();

    emit torrentStatus(status.num_seeds, status.num_peers);
    if(status.state == lt::torrent_status::seeding) {
	    m_TimeoutTimer.start();
    }

    if(status.state == lt::torrent_status::downloading) {
        m_Timer.stop();
        m_TimeoutTimer.stop();
        {
	// The destruction of session proxy 
	// assures that all call writes and everything
	// is finished. This is sync.
	auto sess_proxy = m_Session->abort();
	}  
        b_Running = false;
	emit error(QAppImageUpdateEnums::Error::IncompleteAppImageForSeed);
	return;
    }

    std::vector<lt::alert*> alerts;
    m_Session->pop_alerts(&alerts);
    for (lt::alert const* a : alerts) {
        if (lt::alert_cast<lt::torrent_error_alert>(a)) {
            emit logger(QString::fromStdString(a->message()));
            m_Timer.stop();
            m_TimeoutTimer.stop();
	    {
		// The destruction of session proxy 
		// assures that all call writes and everything
		// is finished. This is sync.
	    	auto sess_proxy = m_Session->abort();
	    }
            b_Running = false;
            emit error(QNetworkReply::ProtocolFailure);
            return;
        }
        QCoreApplication::processEvents();
    }
}

#endif // DECENTRALIZED_UPDATE_ENABLED
