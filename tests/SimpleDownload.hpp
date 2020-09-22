#ifndef SIMPLE_DOWNLOAD_HPP_INCLUDED
#define SIMPLE_DOWNLOAD_HPP_INCLUDED
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QScopedPointer>
#include <QUrl>
#include <QFile>
#include <QElapsedTimer>
#include <QDebug>
#include <QFileInfo>

// Sync Downloader
class SimpleDownload {
	QScopedPointer<QNetworkAccessManager> m_Manager;
public:
	SimpleDownload() {
		m_Manager.reset(new QNetworkAccessManager);
	}
	
	int download(const QUrl &url, const QString &destination) {
		QScopedPointer<QNetworkReply> reply;
		QScopedPointer<QFile> file;
		QString fileName = QFileInfo(destination).fileName();

		QNetworkRequest req;
		req.setUrl(url);
    		req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	

		file.reset(new QFile);
		file->setFileName(destination);
		if(!file->open(QIODevice::WriteOnly)){
			return -1;
		}

		reply.reset(m_Manager->get(req));

		QElapsedTimer elapsed;
		elapsed.start();

		qInfo().noquote() << "Downloading " << fileName;

		while(!reply->isFinished()) {
			if(reply->error() != QNetworkReply::NoError){
				reply->deleteLater();
				qCritical().noquote() << "Download Failed " << fileName << " : " << reply->error();
				return -1;
			}
			if(reply->isReadable()){
				file->write(reply->readAll());
			}
			QCoreApplication::processEvents();
		}

		file->write(reply->readAll());
		file->close();
		qInfo().noquote() << "Downloaded  " << fileName;
		return 0;
	}

	~SimpleDownload() { }
};

#endif
