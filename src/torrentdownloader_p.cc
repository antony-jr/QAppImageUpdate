#ifdef DECENTRALIZED_UPDATE_ENABLED
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <vector>
#include <iostream>

#include "torrentdownloader_p.hpp"

TorrentDownloaderPrivate::TorrentDownloaderPrivate(QNetworkAccessManager *manager) 
	: QObject() {
	n_TargetFileLength = n_TargetFileDone = 0;
	lt::session_params p = lt::session_params();
  	p.settings.set_int(lt::settings_pack::alert_mask, 
			lt::alert_category::status | 
			lt::alert_category::error | 
			lt::alert_category::storage);	
	m_Manager = manager;
	m_Session.reset(new lt::session(p));
	m_TorrentMeta.reset(new QByteArray);

	m_TimeoutTimer.setSingleShot(true);
	m_TimeoutTimer.setInterval(100 * 1000); // 100 seconds
	
	connect(&m_Timer, &QTimer::timeout, 
		 this, &TorrentDownloaderPrivate::torrentDownloadLoop,
		 Qt::QueuedConnection);

	connect(&m_TimeoutTimer, &QTimer::timeout, 
		 this, &TorrentDownloaderPrivate::handleTimeout,
		 Qt::QueuedConnection);
}
TorrentDownloaderPrivate::~TorrentDownloaderPrivate() {

}

void TorrentDownloaderPrivate::setTargetFileDone(qint64 done) {
	if(b_Running) {
		return;
	}

	n_TargetFileDone = done;
}

void TorrentDownloaderPrivate::setTargetFileLength(qint64 n) {
	if(b_Running) {
		return;
	}

	n_TargetFileLength = n;
}

void TorrentDownloaderPrivate::setTargetFile(QTemporaryFile *file) {
	if(b_Running) {
		return;
	}

	m_File = file;
}

void TorrentDownloaderPrivate::setTorrentFileUrl(const QUrl &url) {
	if(b_Running) {
		return;
	}
	m_TorrentFileUrl = url;
}

void TorrentDownloaderPrivate::setTargetFileUrl(const QUrl &url) {
	if(b_Running) {
		return;
	}
	m_TargetFileUrl = url;
	
}

void TorrentDownloaderPrivate::start() {
	if(b_Running) {
		return;
	}
	b_Running = b_Finished = false;


	m_TorrentMeta->clear();

	QNetworkRequest request;
    	request.setUrl(m_TorrentFileUrl);
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

void TorrentDownloaderPrivate::cancel() {
	if(!b_Running || b_CancelRequested) {
		return;
	}
	b_CancelRequested = true;
}

void TorrentDownloaderPrivate::handleTorrentFileError(QNetworkReply::NetworkError code) {
	       	QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
		if(!reply) {
			return;
		}

		reply->disconnect();
		reply->deleteLater();

		emit error(code);
}

void TorrentDownloaderPrivate::handleTorrentFileData(qint64 br, qint64 bt) {
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

void TorrentDownloaderPrivate::handleTorrentFileFinish() {
	auto reply = qobject_cast<QNetworkReply*>(QObject::sender());
	m_TorrentMeta->append(reply->readAll());

	reply->disconnect();
	reply->deleteLater();

	if(b_CancelRequested) {
		m_File->setAutoRemove(true);
		m_File->open();
		b_CancelRequested = false;
		b_Running = b_Finished = false;	
		emit canceled();
		return;
	}

	lt::add_torrent_params params;
       	QString savePath = QFileInfo(m_File->fileName()).path() + "/";

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
		   QFileInfo(m_File->fileName()).fileName().toStdString());	


	/// Add the target file url as web seed
	/// See BEP 17 and BEP 19
	ti->add_url_seed(m_TargetFileUrl.toString().toStdString());

	params.ti = ti;
	m_Handle = m_Session->add_torrent(params);
	if(!m_Handle.is_valid()) {
		emit error(QNetworkReply::ProtocolFailure);
		return;
	}

	m_Timer.setSingleShot(false);
	m_Timer.setInterval(200);
	m_Timer.start();

	m_TimeoutTimer.start();	
}

void TorrentDownloaderPrivate::handleTimeout() {
	m_Timer.stop();
	m_TimeoutTimer.stop();

	emit logger(QString::fromStdString(" handleTimeout: Torrent Downloader Timeout, falling back to range downloader."));	
	m_File->setAutoRemove(true);
	m_File->open();
	m_Session->abort();
	b_Running = false;
	b_Finished = false;
	emit error(QNetworkReply::ProtocolFailure);
}

void TorrentDownloaderPrivate::torrentDownloadLoop() {
	if(b_Finished) {
		m_Timer.stop();
		return;
	}
	if(b_CancelRequested) {
		m_File->setAutoRemove(true);
		m_File->open();
		m_Timer.stop();
		m_Session->abort();
		b_CancelRequested = false;
		b_Running = b_Finished = false;
		emit canceled();
		return;
	}
	auto status = m_Handle.status();

	if(status.state == lt::torrent_status::seeding) {
		emit progress((int)(status.progress * 100),
			      (qint64)(status.total_done), 
			      n_TargetFileLength, 
			      (double)(status.download_payload_rate/1024),
			      QString::fromUtf8(" KB/s "));

		m_Session->abort();
		m_File->setAutoRemove(true);
		m_File->open();
		m_Timer.stop();
		m_TimeoutTimer.stop();
		b_Running = false;
		b_Finished = true;
		emit finished();
		return;		
	
	}
	
	if(status.state == lt::torrent_status::downloading) {	
		m_TimeoutTimer.start(); // Reset timeout timer on every progress in download.

		emit progress((int)(status.progress * 100),
			      (qint64)(status.total_done), 
		 	      n_TargetFileLength, 
		 	      (double)(status.download_payload_rate/1024),
		 	      QString::fromUtf8(" KB/s "));
	}

	std::vector<lt::alert*> alerts;
	m_Session->pop_alerts(&alerts);
	for (lt::alert const* a : alerts) {
		if (lt::alert_cast<lt::torrent_error_alert>(a)) {
			emit logger(QString::fromStdString(a->message()));	
			m_File->setAutoRemove(true);
			m_File->open();
			m_Timer.stop();
	       		m_TimeoutTimer.stop();
			m_Session->abort();
			b_Running = false;
			b_Finished = false;
			emit error(QNetworkReply::ProtocolFailure);
			return;	
		}
		QCoreApplication::processEvents();
	}
}
#endif // DECENTRALIZED_UPDATE_ENABLED
