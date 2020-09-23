#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <vector>
#include <iostream>

#include "torrentdownloader_p.hpp"


TorrentDownloaderPrivate::TorrentDownloaderPrivate(QNetworkAccessManager *manager) 
	: QObject() {
	n_TargetFileLength = 0;
	lt::session_params p = lt::session_params();
  	p.settings.set_int(lt::settings_pack::alert_mask, 
			lt::alert_category::status | 
			lt::alert_category::error | 
			lt::alert_category::storage);	
	m_Manager = manager;
	m_Session.reset(new lt::session(p));
	m_TorrentFile.reset(new QTemporaryFile);
}
TorrentDownloaderPrivate::~TorrentDownloaderPrivate() {

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

void TorrentDownloaderPrivate::setTargetFileName(const QString &fileName) {
	if(b_Running) {
		return;
	}

	m_FileName = fileName;
}

void TorrentDownloaderPrivate::setTorrentFileUrl(const QUrl &url) {
	if(b_Running) {
		return;
	}
	qDebug() <<"Set::" << url;
	m_Url = url;
}
	

void TorrentDownloaderPrivate::start() {
	if(b_Running) {
		return;
	}
	b_Running = b_Finished = false;

	m_TorrentFile->open();
	(void)m_TorrentFile->fileName();

	qDebug() << "Downloading... " << m_Url;

	QNetworkRequest request;
    	request.setUrl(m_Url);
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
			m_TorrentFile->write(reply->readAll());
		}
}

void TorrentDownloaderPrivate::handleTorrentFileFinish() {
	auto reply = qobject_cast<QNetworkReply*>(QObject::sender());
	m_TorrentFile->write(reply->readAll());
	m_TorrentFile->close();

	reply->disconnect();
	reply->deleteLater();

	if(b_CancelRequested) {
		b_CancelRequested = false;
		b_Running = b_Finished = false;	
		emit canceled();
		return;
	}

	// Now start the actual torrent download.
	m_OldTempName = m_File->fileName();
	QFileInfo sameFile(QFileInfo(m_File->fileName()).path() + "/" + m_FileName);
        if(sameFile.exists()) {
                QString newTargetFileName = sameFile.baseName() +
                                    QString("-moved-on-") +
                                    QDateTime::currentDateTime().toString(Qt::ISODate)
                                    .replace(":", "-")
                                    .replace(" ", "-") +
                                    QString(".") +
                                    sameFile.completeSuffix();
		
		QFile f;
		f.setFileName(sameFile.absoluteFilePath());
		if(!f.open(QIODevice::ReadWrite)) {

		}
		f.rename(QFileInfo(m_File->fileName()).path() + "/" + newTargetFileName);
		f.close();
	}
        m_File->rename(QFileInfo(m_File->fileName()).path() + "/" + m_FileName);

	lt::add_torrent_params params;
       	QString savePath = QFileInfo(m_File->fileName()).path() + "/";
	params.save_path = savePath.toStdString();
	params.ti = std::make_shared<lt::torrent_info>(m_TorrentFile->fileName().toStdString());
	m_Handle = m_Session->add_torrent(params);

	m_Timer.setSingleShot(false);
	m_Timer.setInterval(500);
	
	connect(&m_Timer, &QTimer::timeout, 
		 this, &TorrentDownloaderPrivate::torrentDownloadLoop,
		 Qt::QueuedConnection);

	m_Timer.start();	
}

void TorrentDownloaderPrivate::torrentDownloadLoop() {
	if(b_Finished) {
		m_Timer.stop();
		return;
	}
	if(b_CancelRequested) {
		m_Timer.stop();
		m_Session->abort();
		b_CancelRequested = false;
		b_Running = b_Finished = false;
		emit canceled();
		return;
	}
	std::vector<lt::alert*> alerts;
	m_Session->pop_alerts(&alerts);
	for (lt::alert const* a : alerts) {
		if (lt::alert_cast<lt::torrent_finished_alert>(a)) {
			m_File->setAutoRemove(true);
			m_File->open();
			m_Timer.stop();
			m_Session->abort();
			b_Running = false;
			b_Finished = true;
			emit finished();
			QCoreApplication::processEvents();
			return;		
		}
		if (lt::alert_cast<lt::torrent_error_alert>(a)) {
			m_Timer.stop();
	       		m_Session->abort();
			b_Running = false;
			b_Finished = false;
			emit error(QNetworkReply::UnknownNetworkError);
			return;	
		}

		if (auto st = lt::alert_cast<lt::state_update_alert>(a)) {
			if (!st->status.empty()) { 

			lt::torrent_status const& s = st->status[0];
			emit progress((int)(s.progress_ppm / 10000),
				      (qint64)(s.total_done / 10000), 
				       n_TargetFileLength, 
				       (double)s.download_payload_rate,
				       QString::fromUtf8(" KB/s "));
			}
      		}
		QCoreApplication::processEvents();
	}
}
