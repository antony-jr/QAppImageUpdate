#ifndef ZSYNC_BLOCK_RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#define ZSYNC_BLOCK_RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#include <QtGlobal>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace AppImageUpdaterBridge {
class ZsyncRemoteControlFileParserPrivate;
class ZsyncWriterPrivate;
class ZsyncBlockRangeDownloaderPrivate : public QObject {
	Q_OBJECT
public:
	ZsyncBlockRangeDownloaderPrivate(ZsyncRemoteControlFileParserPrivate*,ZsyncWriterPrivate*,QNetworkAccessManager*);
	~ZsyncBlockRangeDownloaderPrivate();

public Q_SLOTS:
	void cancel(void);
private Q_SLOTS:
	void initDownloader(bool);
	void handleTargetFileUrl(QUrl);
	void handleBlockRange(qint32,qint32);
	void handleBlockReplyFinished(void);
	void handleBlockReplyCancel(void);
	void handleCheckHeadError(QNetworkReply::NetworkError);
	void checkHead(qint64, qint64);

Q_SIGNALS:
	void cancelAllReply(void);
	void canceled(void);
	void error(QNetworkReply::NetworkError);
	void finished(void);

private:
	QUrl _uTargetFileUrl;
	QAtomicInteger<bool> _bCancelRequested = false;
	QAtomicInteger<qint64> _nBlockReply = 0;
	ZsyncRemoteControlFileParserPrivate *_pParser = nullptr;
	ZsyncWriterPrivate *_pWriter = nullptr;
	QNetworkAccessManager *_pManager = nullptr;
};
}
#endif // ZSYNC_BLOCK_RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
