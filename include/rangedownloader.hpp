#ifndef RANGE_DOWNLOADER_HPP_INCLUDED
#define RANGE_DOWNLOADER_HPP_INCLUDED
#include <QObject>
#include <QScopedPointer>

class RangeDownloaderPrivate;

class RangeDownloader : public QObject { 
	Q_OBJECT
	QScopedPointer<RangeDownloaderPrivate> m_Private;
public:
	RangeDownloader(QObject *parent = nullptr);
Q_SIGNALS:
	void started();
	void canceled();
	void finished();
	void error(QNetworkReply::NetworkError);

	void data(QByteArray *);
	void rangeData(qint32, qint32, QByteArray *);
};
#endif // RANGE_DOWNLOADER_HPP_INCLUDED
