#ifndef APPIMAGE_UPDATER_BRIDGE_BLOCK_REPLY_PRIVATE_HPP_INCLUDED
#define APPIMAGE_UPDATER_BRIDGE_BLOCK_REPLY_PRIVATE_HPP_INCLUDED
#include <QtCore>
#include <QNetworkReply>
#include <ZsyncWriter_p.hpp>

namespace AppImageUpdaterBridge {
class BlockReplyPrivate : public QObject {
	Q_OBJECT
public:
	BlockReplyPrivate(ZsyncWriterPrivate *writer , QNetworkReply *reply , qint32 rf , qint32 rt)
		: QObject(reply),
		  _nRangeFrom(rf),
		  _nRangeTo(rt)
	{
		connect(reply , &QNetworkReply::finished , this , &BlockReplyPrivate::handleFinished);
		connect(reply , SIGNAL(error(QNetworkReply::NetworkError)) , this , SLOT(handleError(QNetworkReply::NetworkError)));
		connect(this , &BlockReplyPrivate::sendBlockDataToWriter , writer , &ZsyncWriterPrivate::writeBlockRanges);
		connect(this , &BlockReplyPrivate::cancelReply , reply , &QNetworkReply::abort);
		return;
	}

	~BlockReplyPrivate()
	{
		return;
	}

public Q_SLOTS:
	void cancel(void)
	{
		emit cancelReply();
		return;
	}

private Q_SLOTS:
	void handleFinished(void)
	{
		auto reply = (QNetworkReply*)QObject::sender();
		QByteArray *rawData = new QByteArray;

		while(!reply->atEnd()){
			/* Use standard size of 4096 bytes. */
			rawData->append(reply->read(4096));
		}

		emit sendBlockDataToWriter(_nRangeFrom , _nRangeTo , rawData);
		return;
	}

	void handleError(QNetworkReply::NetworkError ecode)
	{
		if(ecode == QNetworkReply::OperationCanceledError)
		{
			emit canceled();
			return;
		}
		emit error(ecode);
		return;
	}
Q_SIGNALS:
	void canceled(void);
	void error(QNetworkReply::NetworkError);
	void cancelReply(void);
	void sendBlockDataToWriter(qint32 , qint32 , QByteArray *);
private:
	qint32 _nRangeFrom = 0,
	       _nRangeTo = 0;
};
}
#endif // APPIMAGE_UPDATER_BRIDGE_BLOCK_REPLY_PRIVATE_HPP_INCLUDED
