#ifndef SIMPLE_DOWNLOAD_HPP_INCLUDED
#define SIMPLE_DOWNLOAD_HPP_INCLUDED
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QScopedPointer>
#include <QUrl>
#include <QFile>

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

		QNetworkRequest req;
		req.setUrl(url);
		

		file.reset(new QFile);
		file->setFileName(destination);
		if(!file->open(QIODevice::WriteOnly)){
			return -1;
		}

		reply.reset(m_Manager->get(req));


		while(!reply->isFinished()) {
			if(reply->error() != QNetworkReply::NoError){
				reply->deleteLater();
				return -1;
			}
			if(reply->isReadable()){
				file->write(reply->readAll());
			}
			QCoreApplication::processEvents();
		}

		file->write(reply->readAll());
		file->close();
		return 0;
	}

	~SimpleDownload() { }
};

#endif
