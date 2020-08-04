#ifndef RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#define RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#include <QObject>
#include <QUrl>
#include <QVector>
#include <QNetworkReply>
#include <QScopedPointer>

class RangeReplySignalEmitterPrivate : public QObject {
	Q_OBJECT
public:
	RangeReplySignalEmitterPrivate() : 
		QObject() {
	}

	~RangeReplySignalEmitterPrivate() { }

	void enlist(RangeReplyPrivate *obj) {
		if(!obj) {
			return;
		}

		connect(this, SIGNAL(retryRequested(int)),
			obj, SLOT(retry(int)),
			Qt::UniqueConnection);
		connect(this, SIGNAL(cancelRequested()),
			obj, SLOT(cancel()),
			Qt::UniqueConnection);
		connect(this, SIGNAL(disownRequested()),
			obj, SLOT(disown()),
			Qt::UniqueConnection);	
	}

	void disownAll() {
		emit disownRequested();
	}

	void retry(int timeout = 3000 /*miliseconds*/) {
		emit retryRequested(timeout);
	}

	void cancel() {
		emit cancelRequested();
	}
Q_SIGNALS:
	void retryRequested(int);
	void cancelRequested();
	void disownRequested();
};



class RangeReplyPrivate : public QObject {
	Q_OBJECT 
public:
	RangeReplyPrivate(int index, QNetworkReply *reply, QPair<qint32, qint32> range) {
		n_Index = index;
		n_FromRange = range.first;
		n_ToRange = range.second;
		m_Reply = reply;
		m_Request = reply->request();
		m_Manager = reply->manager();
		m_Data.reset(new QByteArray);
		m_Timer.setSingleShot(true);

		connect(m_Reply, SIGNAL(progress(qint64, qint64)),
			this, SLOT(handleData(qint64, qint64)),
			Qt::QueuedConnection);
		connect(m_Reply, SIGNAL(finished()),
			this, SLOT(handleFinish),
			Qt::QueuedConnenction);
		connect(m_Reply, SIGNAL(error(QNetworkReply::QNetworkError)),
			this, SLOT(handleError(QNetworkReply::QNetworkError)),
			Qt::QueuedConnection);
	
		//// Connect timer for retry action
		connect(&m_Timer, SIGNAL(timeout()),
			 this, SLOT(restart()));
	}

	~RangeReplyPrivate() {
		m_Timer.stop();
		if(b_Running) {
			m_Reply->disconnect();
			m_Reply->abort();
			m_Reply->deleteLater();
			b_Running = false;
		}
	}

public Q_SLOTS:
	void disown() {
		disconnect();
	}

	void retry(int timeout) {
		if(b_Running || b_Finished) {
			return;
		}

		m_Timer.setInterval(timeout);
		m_Timer.start();
	}

	void cancel() {
		if(b_CancelRequested) {
			return;
		}

		if(!b_Running || b_Canceled || b_Finished){
			return;
		}

		b_CancelRequested = true;
		m_Reply->abort();
	}
private Q_SLOTS:
	void restart() {

	}

	void handleData(qint64 bytesRec, qint64 bytesTotal) {
		Q_UNUSED(bytesRec);
		Q_UNUSED(bytesTotal);

		if(m_Reply->error() != QNetworkReply::NoError){
			return;
		}

		if(m_Reply->isOpen() && m_Reply->isReadable()){
			m_Data->append(m_Reply->readAll());
		}
	}

	void handleError(QNetworkReply::QNetworkError code) {
		if(code == QNetworkReply::OperationCanceled) {
			m_Data->clear();
			b_Canceled = true;
			b_Running = b_Finished = false;
			emit canceled();
			return;
		}
		b_Running = b_Finished = b_Canceled = false;
		emit error(code);
		return;
	}

	void handleFinish() {
		b_Running = b_Canceled = false;
		b_Finished = true;
		
		/// Append any data that is left.
		m_Data->append(m_Reply->readAll());
	
		/// Finish the range reply	
		emit finished(n_FromRange, n_ToRange, m_Data.take(), n_Index);
	
		/// Delete resource	
		m_Reply->disconnect();
		m_Reply->deleteLater();
	}

Q_SIGNALS:
	void started(int);
	void error(int, int);
	void finished(qint32, qint32, QByteArray*, int);
	void canceled(int);
private:
	bool b_Running = false,
	     b_Finished = false,
	     b_Canceled = false,
	     b_CancelRequested = false;
	int n_Index;
	qint32 n_FromRange,
	       n_ToRange;
	QTimer m_Timer;
	QNetworkReply *m_Reply;
	QNetworkRequest m_Request;
	QNetworkAccessManager *m_Manager;
	QScopedPointer<QByteArray> m_Data;
};

class RangeDownloaderPrivate : public QObject {
	Q_OBJECT
public:
	RangeDownloaderPrivate(QObject *parent = nullptr);
	~RangeDownloaderPrivate();
public Q_SLOTS:
	void setTargetFileUrl(const QUrl&);
	void setFullDownload(bool);
	void setRequiredRanges(const QVector<QPair<qint32, qint32>>&);
	void appendRange(qint32, qint32);

	void start();
	void cancel();
Q_SIGNALS:
	void started();
	void canceled();
	void error(short);

	void data(QByteArray *);
	void rangeData(qint32, qint32, QByteArray *);
private:
	bool b_Started = false,
	     b_Finished = false,
	     b_Running = false,
	     b_Canceled = false;
	int n_Active = 0,
	    n_Canceled = 0;

	QVector<QPair<qint32, qint32>> m_RequiredRanges;
};

#endif // RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
