#ifndef RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#define RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#include <QObject>
#include <QUrl>
#include <QVector>
#include <QNetworkReply>
#include <QScopedPointer>

class RangeReply : public QObject { 
	Q_OBJECT
	QScopedPointer<RangeReplyPrivate> m_Private;
public:
	RangeReply(int index, QNetworkReply *reply, const QPair<qint32, qint32> &range)
	 	: QObject() {
		m_Private.reset(new RangeReplyPrivate(index, reply, range));
		
		auto ptr = m_Private.data();
		connect(ptr, &RangeReplyPrivate::restarted,
			 this, &RangeReply::restarted,
		 	 Qt::DirectConnection);
		connect(ptr, &RangeReplyPrivate::error,
			 this, &RangeReply::error,
		 	 Qt::DirectConnection);
		connect(ptr, &RangeReplyPrivate::finished,
			 this, &RangeReply::finished,
		 	 Qt::DirectConnection);
		connect(ptr, &RangeReplyPrivate::canceled,
			 this, &RangeReply::canceled,
		 	 Qt::DirectConnection);
	}

	~RangeReply() {
		getMethod(m_Private.data(), "destroy()")
			.invoke(m_Private.data(), Qt::QueuedConnection);
	}
public Q_SLOTS:
	void destroy() {
	    getMethod(m_Private.data(), "destroy()")
		.invoke(m_Private.data(), Qt::QueuedConnection);	
	}
	void retry(int timeout = 3000) {
	    getMethod(m_Private.data(), "retry(int)")
		.invoke(m_Private.data(), Q_ARG(int, timeout), Qt::QueuedConnection);
	
	}
	void cancel() {
	    getMethod(m_Private.data(), "cancel()")
		.invoke(m_Private.data(), Qt::QueuedConnection);
	
	}
Q_SIGNALS:
	void restarted(int);
	void error(int, int);
	void finished(qint32, qint32, QByteArray*, int);
	void canceled(int);
};

class RangeReplyPrivate : public QObject {
	Q_OBJECT 
public:
	RangeReplyPrivate(int index, QNetworkReply *reply, const QPair<qint32, qint32> &range) {
		n_Index = index;
		n_FromRange = range.first;
		n_ToRange = range.second;
		m_Request = reply->request();
		m_Manager = reply->manager();
		m_Reply.reset(reply);
		m_Data.reset(new QByteArray);
		m_Timer.setSingleShot(true);

		connect(reply, SIGNAL(progress(qint64, qint64)),
			this, SLOT(handleData(qint64, qint64)),
			Qt::QueuedConnection);
		connect(reply, SIGNAL(finished()),
			this, SLOT(handleFinish),
			Qt::QueuedConnenction);
		connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
			this, SLOT(handleError(QNetworkReply::NetworkError)),
			Qt::QueuedConnection);
	
		//// Connect timer for retry action
		connect(&m_Timer, SIGNAL(timeout()),
			 this, SLOT(restart()));
	}

	~RangeReplyPrivate() {
		if(b_Halted) {
			return;
		}else if(b_Retrying) {
			m_Timer.stop();
		}	
		else if(b_Running) {	
			m_Reply->disconnect();
			m_Reply->abort();
		}	
	}

public Q_SLOTS:
	void destroy() {
		if(b_Retrying) {
			m_Timer.stop();
		}	
		else if(b_Running) {	
			m_Reply->disconnect();
			m_Eeply->abort();
		}
		
		resetInternalFlags();	
		b_Halted = true;

		disconnect();
		this->deleteLater();
	}

	void retry(int timeout) {
		if(b_Running || b_Finished || b_Halted) {
			return;
		}
	
		resetInternalFlags();
		b_Retrying = true;

		m_Timer.setInterval(timeout);
		m_Timer.start();
	}

	void cancel() {
		if(b_Retrying) {
			m_Timer.stop();
			resetInternalFlags();
			b_Canceled = true;
			emit canceled(n_Index);
			return;
		}

		if(!b_Running || 
		    b_Canceled || 
		    b_Finished || 
		    b_CancelRequested){
			return;
		}

		b_CancelRequested = true;
		m_Reply->abort();
	}
private Q_SLOTS:
	void resetInternalFlags(bool value = false) {
		b_Halted = b_Running = b_Finished = b_CancelRequested = b_Retrying = false;
	}

	void restart() {
		if(b_Halted) {
			return;
		}

		resetInternalFlags();
		b_Retrying = false;

		m_Reply.reset(m_Manager->get(m_Request));

		auto reply = m_Reply.data();
		connect(reply, SIGNAL(progress(qint64, qint64)),
			this, SLOT(handleData(qint64, qint64)),
			Qt::QueuedConnection);
		connect(reply, SIGNAL(finished()),
			this, SLOT(handleFinish),
			Qt::QueuedConnenction);
		connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
			this, SLOT(handleError(QNetworkReply::NetworkError)),
			Qt::QueuedConnection);
	
		b_Running = true;
		emit restarted(n_Index);
	}

	void handleData(qint64 bytesRec, qint64 bytesTotal) {
		Q_UNUSED(bytesRec);
		Q_UNUSED(bytesTotal);

		if(m_Reply.isNull() || b_Halted) {
			return;
		}

		if(m_Reply->error() != QNetworkReply::NoError){
			return;
		}

		if(m_Reply->isOpen() && m_Reply->isReadable()){
			m_Data->append(m_Reply->readAll());
		}
	}

	void handleError(QNetworkReply::NetworkError code) {
		if(b_Halted) {
			return;
		}

		if(code == QNetworkReply::OperationCanceled || b_CancelRequested) {
			m_Data->clear();
			m_Reply->disconnect();

			resetInternalFlags();
			b_Canceled = true;
			
			emit canceled(n_Index);
			return;
		}
		resetInternalFlags();
		emit error(code, n_Index);
		return;
	}

	void handleFinish() {
		if(b_Halted) {
			return;
		}

		if(b_CancelRequested) {
			m_Data->clear();
			resetInternalFlags();
			b_Canceled = true;

			emit canceled(n_Index);
			return;
		}
		resetInternalFlags();
		b_Finished = true;
		
		/// Append any data that is left.
		m_Data->append(m_Reply->readAll());
	
		/// Finish the range reply	
		emit finished(n_FromRange, n_ToRange, m_Data.take(), n_Index);
	
		m_Reply->disconnect();
	}

Q_SIGNALS:
	void restarted(int);
	void error(int, int);
	void finished(qint32, qint32, QByteArray*, int);
	void canceled(int);
private:
	bool b_Running = false,
	     b_Finished = false,
	     b_Canceled = false,
	     b_CancelRequested = false,
	     b_Retrying = false,
	     b_Halted = false;
	int n_Index;
	qint32 n_FromRange,
	       n_ToRange;
	QTimer m_Timer;
	QScopedPointer<QNetworkReply> m_Reply;
	QNetworkRequest m_Request;
	QNetworkAccessManager *m_Manager;
	QScopedPointer<QByteArray> m_Data;
};

class RangeDownloaderPrivate : public QObject {
	Q_OBJECT
public:
	RangeDownloaderPrivate(QObject *parent = nullptr) {

	}
	
	~RangeDownloaderPrivate() { 
		if(b_Running) {
			for(auto iter = m_ActiveRequests.begin(),
				 end = m_ActiveRequests.end();
	 			 iter != end;
				 ++iter) {
			   if(*iter){ 
				(*iter)->disconnect();
				(*iter)->destroy();	
			   }
			}				
		}
	}
public Q_SLOTS:
	void setTargetFileUrl(const QUrl &url) {
		if(b_Running) {
			return;
		}
		m_Url = url;
	}

	void setFullDownload(bool fullDownload) {
		if(b_Running) { 
			return;
		}
		b_FullDownload = fullDownlaod;
	}

	void setRequiredRanges(const QVector<QPair<qint32, qint32>> &ranges) {
		if(b_Running) {
			return;
		}

		m_RequiredRanges = ranges;
	}

	void appendRange(qint32 from, qint32 to) {
		if(b_Running) {
			return;
		}

		m_RequiredRanges.append(qMakePair<qint32, qint32>(from, to));
	}

	void appendRange(QPair<qint32, qint32> range) {
		if(b_Running) {
			return;
		}
		m_RequiredRanges.append(range);
	}

	void start() {
		if(b_Running) {
			return;
		}
		b_Running = b_Finished = b_Canceled = false;
		n_Active = n_Canceled = -1;		

		QNetworkRequest request;
		
		// Before starting the download we have to resolve the url such that it
		// does not have any redirections whatsoever.
    		// For this we send a get request and abort it before it even begin.
    		// We should not send a HEAD request since it may not be supported by some
    		// hosts.
    		request.setUrl(m_Url);
    		request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    		request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    		auto reply = m_Manager->get(request);
    		connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            		this, SLOT(handleUrlCheckError(QNetworkReply::NetworkError)),
            		Qt::QueuedConnection);
    		connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            		this, SLOT(handleUrlCheck(qint64, qint64)),
            		Qt::QueuedConnection);
		b_Running = true;
		emit started();
	}

	void cancel() {
		if(!b_Running || b_CancelRequested) {
			return;
		}
		b_CancelRequested = true;
		for(auto iter = m_ActiveRequests.begin(),
			 end = m_ActiveRequests.end();
			 iter != end;
			 ++iter) {
			(*iter)->cancel();
			QCoreApplication::processEvents();
		}
	}
private Q_SLOTS:
	QNetworkRequest makeRangeRequest(const QUrl &url, const QPair<qint32, qint32> &range) {
		qint32 fromRange = range.first,
		       toRange = range.second;		 
		QNetworkRequest request;

		request.setUrl(url);
		if(fromRange || toRange) {
        		QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(fromRange) + "-";
        		rangeHeaderValue += QByteArray::number(toRange);
        		request.setRawHeader("Range", rangeHeaderValue);
    		}
    		request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    		request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
		return request;	
	}


	// Slots which does the url check routine
	void handleUrlCheckError(QNetworkReply::NetworkError code) {
	       	QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
		if(!reply) {
			return;
		}

		reply->disconnect();
		reply->deleteLater();

		emit error(code);
	}

	void handleUrlCheck(qint64 br, qint64 bt) {
		Q_UNUSED(br);
		Q_UNUSED(bt);
		
		auto reply = qobject_cast<QNetworkReply*>(QObject::sender());
		if(!reply) {
			return;
		}

		if(reply->error() != QNetworkReply::NoError) {
			return;
		}
	
		m_Url = reply->url();

		reply->disconnect();
    		reply->abort();
		reply->deleteLater();

		/// Now we will start the actual download since we got 
		//  the clean url to the target file.

		/// If this flag is set then it means we can't use range request and 
		//  we have to initiate a very simple download.
		if(b_FullDownload) {
			/// TODO: Use a new class to handle full download.
			return;
		}

		// Now we will determine the maximum no. of requests to be handled at 
		// a time.
		int max_allowed = QThread::idealThreadCount() * 2;

		for(auto iter = m_RequiredRanges.begin(),
			 end = m_RequiredRanges.end();
			 iter != end;) {
			 if(n_Active + 1 >= max_allowed) {
				 break;
			 }
			 QNetworkRequest request = makeRangeRequest(m_Url, *iter);
			 ++n_Active;
			
			auto rangeReply = new RangeReplyPrivate(n_Active, m_Manager->get(request), *iter);
			m_RangeReplyCancel.enlist(rangeReply);

			connect(rangeReply, SIGNAL(canceled(int)),
				this, SLOT(handleRangeReplyCancel(int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(restarted(int)),
				this, SLOT(handleRangeReplyRestart(int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(error(QNetworkReply::NetworkError, int)),
				this, SLOT(handleRangeReplyError(QNetworkReply::NetworkError, int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(finished(qint32, qint32, QByteArray*, int)),
				this, SLOT(handleRangeReplyFinished(qint32, qint32, QByteArray*, int)),
				Qt::QueuedConnection);	

			iter = m_RequiredRanges.erase(iter);			
		}
	}

	/// ----

	/// Range Reply Handlers
	void handleRangeReplyCancel(int index) {
		--n_Active;
		if(n_Active == -1) {
			b_Running = false;
			b_CancelRequested = false;
			emit canceled();
		}
	}

	void handleRangeReplyRestart(int index) {
		/// TODO: Anything useful in this?	
	}

	void handleRangeReplyError(QNetworkReply::NetworkError code, int index) {
		/// TODO: If the error is not severe then we might want to retry to 
		//        a specific threshold
		if( /* code is not severe and can be retried and max tries is not reached */0) {
			(m_ActiveRequests.at(index))->retry();
			m_RRManager.retry(/*Retry interval(in ms)=*/ 10000 /*=10 seconds*/);
			return;
		}
		if(n_Active == -1) {
			b_Running = b_Finished = b_CancelRequested = false;
			emit error(code);
		}
	}

	void handleRangeReplyFinished(qint32 from, qint32 to, QByteArray *data, int index) {
		--n_Active;
		if(b_CancelRequested) {
			return;
		}

		(m_ActiveRequests.at(index))->destroy();

		emit rangeData(from, to, data);
		if(n_Active == -1) {
			b_Running = false;
			b_Finsihed = true;
			emit finished();
			return;
		}

		if(m_RequiredRanges.isEmpty())
			return;

		auto range = m_RequiredRanges.takeFirst();
		auto rangeReply = new RangeReplyPrivate(index, m_Manager->get(makeRangeRequest(m_Url, *range)), *range);
		m_ActiveRequests[index] = rangeReply;

		connect(rangeReply, SIGNAL(canceled(int)),
				this, SLOT(handleRangeReplyCancel(int)),
				Qt::QueuedConnection);

		connect(rangeReply, SIGNAL(restarted(int)),
				this, SLOT(handleRangeReplyRestart(int)),
				Qt::QueuedConnection);

		connect(rangeReply, SIGNAL(error(QNetworkReply::NetworkError, int)),
				this, SLOT(handleRangeReplyError(QNetworkReply::NetworkError, int)),
				Qt::QueuedConnection);

		connect(rangeReply, SIGNAL(finished(qint32, qint32, QByteArray*, int)),
				this, SLOT(handleRangeReplyFinished(qint32, qint32, QByteArray*, int)),
				Qt::QueuedConnection);	
	}

Q_SIGNALS:
	void started();
	void canceled();
	void finished();
	void error(QNetworkReply::NetworkError);

	void data(QByteArray *);
	void rangeData(qint32, qint32, QByteArray *);
private:
	bool b_Finished = false,
	     b_Running = false,
	     b_CancelRequested = false,
	     b_FullDownload = false;
	int n_Active = -1,
	    n_Canceled = -1;
	QUrl m_Url;

	QVector<QPair<qint32, qint32>> m_RequiredRanges;
	QVector<RangeReply*> m_ActiveRequests;

};

#endif // RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
