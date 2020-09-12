#include <QCoreApplication>
#include <QThread>

#include "rangedownloader_p.hpp"

RangeDownloaderPrivate::RangeDownloaderPrivate(QNetworkAccessManager *manager, QObject *parent) 
	: QObject(parent) {
	m_Manager = manager;

}
	
RangeDownloaderPrivate::~RangeDownloaderPrivate() { 
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

/// Public Slots.

void RangeDownloaderPrivate::setTargetFileUrl(const QUrl &url) {
		if(b_Running) {
			return;
		}
		m_Url = url;
}

void RangeDownloaderPrivate::setFullDownload(bool fullDownload) {
		if(b_Running) { 
			return;
		}
		b_FullDownload = fullDownload;
}

void RangeDownloaderPrivate::appendRange(qint32 from, qint32 to, qint32 blocks) {
		if(b_Running) {
			return;
		}


		m_RequiredRanges.append(Range(from, to, blocks));
}

void RangeDownloaderPrivate::start() {
		if(b_Running) {
			return;
		}
		b_Running = b_Finished = false;
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
            		this, SLOT(handleUrlCheckError(QNetworkReply::NetworkError)));
    		connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            		this, SLOT(handleUrlCheck(qint64, qint64)));
		b_Running = true;
		emit started();
}

void RangeDownloaderPrivate::cancel() {
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

/// Private Slots
QNetworkRequest RangeDownloaderPrivate::makeRangeRequest(const QUrl &url, const Range &range){
		QNetworkRequest request;

		request.setUrl(url);
		if(range.from || range.to) {
			QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(range.from) + "-";
        		rangeHeaderValue += QByteArray::number(range.to);
			request.setRawHeader("Range", rangeHeaderValue);
    		}
    		request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    		request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
		return request;	
}


// Slots which does the url check routine
void RangeDownloaderPrivate::handleUrlCheckError(QNetworkReply::NetworkError code) {
	       	QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
		if(!reply) {
			return;
		}

		reply->disconnect();
		reply->deleteLater();

		emit error(code);
}

void RangeDownloaderPrivate::handleUrlCheck(qint64 br, qint64 bt) {
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
		int i = n_Done;

		for(;i < m_RequiredRanges.size(); ++i){
			 if(n_Active + 1 >= max_allowed) {
				 break;
			 }
			 
			 auto range = m_RequiredRanges.at(i);

			 QNetworkRequest request = makeRangeRequest(m_Url, range);
			 ++n_Active;
			
			auto rangeReply = new RangeReply(n_Active, m_Manager->get(request), 
							 qMakePair<qint32, qint32>(range.from, range.to), range.blocks);

			connect(rangeReply, SIGNAL(canceled(int)),
				this, SLOT(handleRangeReplyCancel(int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(restarted(int)),
				this, SLOT(handleRangeReplyRestart(int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(error(QNetworkReply::NetworkError, int)),
				this, SLOT(handleRangeReplyError(QNetworkReply::NetworkError, int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(finished(qint32, qint32,qint32, QByteArray*, int)),
				this, SLOT(handleRangeReplyFinished(qint32, qint32,qint32, QByteArray*, int)),
				Qt::QueuedConnection);	

			m_ActiveRequests.append(rangeReply);
		}
		n_Done = i;
}

/// ----

/// Range Reply Handlers
void RangeDownloaderPrivate::handleRangeReplyCancel(int index) {
		--n_Active;
		if(n_Active == -1) {
			b_Running = b_Finished = b_CancelRequested = false;
			emit canceled();
		}
}


void RangeDownloaderPrivate::handleRangeReplyRestart(int index) {
		/// TODO: Anything useful in this?	
}

void RangeDownloaderPrivate::handleRangeReplyError(QNetworkReply::NetworkError code, int index) {
		/// TODO: If the error is not severe then we might want to retry to 
		//        a specific threshold
		if( /* code is not severe and can be retried and max tries is not reached */0) {
			(m_ActiveRequests.at(index))->retry();
			return;
		}else{
			--n_Active;
		}
		if(n_Active == -1) {
			b_Running = b_Finished = b_CancelRequested = false;
			emit error(code);
		}
}

void RangeDownloaderPrivate::handleRangeReplyFinished(qint32 from, qint32 to, qint32 blocks, QByteArray *data, int index) {
		--n_Active;
		if(b_CancelRequested) {
			return;
		}

		(m_ActiveRequests.at(index))->destroy();

		emit rangeData(from, to, blocks, data);
		if(n_Active == -1) {
			b_Running = false;
			b_Finished = true;
			emit finished();
			return;
		}

		if(n_Done >= m_RequiredRanges.size())
			return;

		auto range = m_RequiredRanges.at(n_Done++);
		QNetworkRequest request = makeRangeRequest(m_Url, range);
		auto rangeReply = new RangeReply(index, m_Manager->get(request), 
							qMakePair<qint32, qint32>(range.from, range.to), range.blocks);
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

		connect(rangeReply, SIGNAL(finished(qint32, qint32, qint32, QByteArray*, int)),
				this, SLOT(handleRangeReplyFinished(qint32, qint32, qint32,  QByteArray*, int)),
				Qt::QueuedConnection);	
}
