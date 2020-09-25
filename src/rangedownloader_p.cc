#include <QCoreApplication>
#include <QThread>

#include "rangedownloader_p.hpp"

RangeDownloaderPrivate::RangeDownloaderPrivate(QNetworkAccessManager *manager, QObject *parent) 
	: QObject(parent) {
	m_Manager = manager;
	m_Manager->clearAccessCache();
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

void RangeDownloaderPrivate::setBlockSize(qint32 blockSize) {
	if(b_Running) {
		return;
	}
	n_BlockSize = blockSize;
}

void RangeDownloaderPrivate::setTargetFileUrl(const QUrl &url) {
		if(b_Running) {
			return;
		}
		m_Url = url;
}

void RangeDownloaderPrivate::setTargetFileLength(qint32 len) {
		if(b_Running) {
			return;
		}
		n_TotalSize = len;
}

void RangeDownloaderPrivate::setBytesWritten(qint64 len) {
		if(b_Running) {
			return;
		}
		n_BytesWritten = len;
}

void RangeDownloaderPrivate::setFullDownload(bool fullDownload) {
		if(b_Running) { 
			return;
		}
		b_FullDownload = fullDownload;
}

void RangeDownloaderPrivate::appendRange(qint32 from, qint32 to) {
		if(b_Running) {
			return;
		}


		m_RequiredBlocks.append(qMakePair<qint32, qint32>(from,to));
}

void RangeDownloaderPrivate::start() {
		if(b_Running) {
			return;
		}
		b_Running = b_Finished = false;
		n_Active = -1;

		QNetworkRequest request;
		
		// Before starting the download we have to resolve the url such that it
		// does not have any redirections whatsoever.
    		// For this we send a get request and abort it before it even begin.
    		// We should not send a HEAD request since it may not be supported by some
    		// hosts.
    		request.setUrl(m_Url);
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
QNetworkRequest RangeDownloaderPrivate::makeRangeRequest(const QUrl &url, const QPair<qint32, qint32> &range){
		QNetworkRequest request;

		request.setUrl(url);
		if(range.first || range.second) {

			auto fromRange = range.first * n_BlockSize;
			auto toRange = range.second * n_BlockSize;


			QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(fromRange) + "-";
        		rangeHeaderValue += QByteArray::number(toRange);
			request.setRawHeader("Range", rangeHeaderValue);
    		}
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

		/// Amount of bytes downloaded
		n_RecievedBytes = 0;
		m_ElapsedTimer.start();

		/// If this flag is set then it means we can't use range request and 
		//  we have to initiate a very simple download.
		if(b_FullDownload) {
			/// Full download just launch a single RangeReply object.
			++n_Active;
			auto range = qMakePair<qint32,qint32>(0,0);
			auto rangeReply = new RangeReply(n_Active, m_Manager->get(makeRangeRequest(m_Url, range)), range);

			connect(rangeReply, SIGNAL(canceled(int)),
				this, SLOT(handleRangeReplyCancel(int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(restarted(int)),
				this, SLOT(handleRangeReplyRestart(int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(error(QNetworkReply::NetworkError, int,bool)),
				this, SLOT(handleRangeReplyError(QNetworkReply::NetworkError, int,bool)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(finished(qint32,qint32, QByteArray*, int)),
				this, SLOT(handleRangeReplyFinished(qint32,qint32, QByteArray*, int)),
				Qt::QueuedConnection);
		
			connect(rangeReply, SIGNAL(progress(qint64, int)), 
				 this, SLOT(handleRangeReplyProgress(qint64, int)),
				 Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(data(QByteArray*, bool)),
				 this, SIGNAL(data(QByteArray*, bool)),
				 Qt::DirectConnection);

			m_ActiveRequests.append(rangeReply);

			return;
		}

		// Now we will determine the maximum no. of requests to be handled at 
		// a time.
		int max_allowed = QThread::idealThreadCount() * 2;
		int i = n_Done;

		for(;i < m_RequiredBlocks.size(); ++i){
			 if(n_Active + 1 >= max_allowed) {
				 break;
			 }
			 
			 auto range = m_RequiredBlocks.at(i);

			 QNetworkRequest request = makeRangeRequest(m_Url, range);
			 ++n_Active;

			auto rangeReply = new RangeReply(n_Active, m_Manager->get(request), range);

			connect(rangeReply, SIGNAL(canceled(int)),
				this, SLOT(handleRangeReplyCancel(int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(restarted(int)),
				this, SLOT(handleRangeReplyRestart(int)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(error(QNetworkReply::NetworkError, int,bool)),
				this, SLOT(handleRangeReplyError(QNetworkReply::NetworkError, int,bool)),
				Qt::QueuedConnection);

			connect(rangeReply, SIGNAL(finished(qint32,qint32, QByteArray*, int)),
				this, SLOT(handleRangeReplyFinished(qint32,qint32, QByteArray*, int)),
				Qt::QueuedConnection);
			
			connect(rangeReply, SIGNAL(progress(qint64, int)), 
				 this, SLOT(handleRangeReplyProgress(qint64, int)),
				 Qt::QueuedConnection);

			m_ActiveRequests.append(rangeReply);
		}
		n_Done = i;
}

/// ----

/// Range Reply Handlers
void RangeDownloaderPrivate::handleRangeReplyCancel(int index) {
		(m_ActiveRequests.at(index))->destroy();
		m_ActiveRequests[index] = nullptr;
		--n_Active;
		if(n_Active == -1) {
			b_Running = b_Finished = b_CancelRequested = false;
			emit canceled();
		}
}


void RangeDownloaderPrivate::handleRangeReplyRestart(int index) {
		Q_UNUSED(index);

}

void RangeDownloaderPrivate::handleRangeReplyError(QNetworkReply::NetworkError code, int index, bool threshReached) {
		if(b_CancelRequested) {
			(m_ActiveRequests.at(index))->destroy();
			m_ActiveRequests[index] = nullptr;
			--n_Active;
			if(n_Active == -1) {
				b_Running = b_Finished = b_CancelRequested = false;
				emit canceled();
			}
			return;
		}



		/// Let's try to retry some type of errors.	
		/// We don't try to retry a full download, if it 
		/// fails then the update has to be started from the 
		/// start. This is because even if we try to restart
		/// we have to download it from the begining and so
		/// It has some complications.
		if((code == QNetworkReply::RemoteHostClosedError ||
		   code == QNetworkReply::HostNotFoundError ||
		   code == QNetworkReply::TimeoutError ||
		   code == QNetworkReply::TemporaryNetworkFailureError ||
		   code == QNetworkReply::BackgroundRequestNotAllowedError ||
		   code == QNetworkReply::ProxyConnectionClosedError ||
		   code == QNetworkReply::ProxyTimeoutError ||
		   code == QNetworkReply::ContentAccessDenied ||
		   code == QNetworkReply::ContentReSendError || 
		   code == QNetworkReply::InternalServerError ||
		   code == QNetworkReply::ServiceUnavailableError) && !threshReached && !b_FullDownload) {
			(m_ActiveRequests.at(index))->retry();
			return;
		}else {
			n_Active = -1;
			for(auto iter = m_ActiveRequests.begin(),
				 end = m_ActiveRequests.end();
	 		 	 iter != end;
			 	 ++iter) {
		   	     if(*iter){ 
				(*iter)->disconnect();
				(*iter)->destroy();
		   	     }
			}
			m_ActiveRequests.clear();	
			b_Running = b_Finished = b_CancelRequested = false;
			emit error(code);
		}
}

void RangeDownloaderPrivate::handleRangeReplyFinished(qint32 from, qint32 to, QByteArray *Data, int index) {
		(m_ActiveRequests.at(index))->destroy();
		m_ActiveRequests[index] = nullptr;

		if(b_CancelRequested) {
			--n_Active;
			if(n_Active == -1) {
				b_Running = b_Finished = b_CancelRequested = false;
				emit canceled();
			}
			return;
		}
		
		if(b_FullDownload) {
			emit data(Data, true);
			return;
		}else {	
			bool isLast = (n_Done >= m_RequiredBlocks.size() && n_Active - 1 == -1);	
			emit rangeData(from, to,  Data, isLast);
		}

		if(n_Done >= m_RequiredBlocks.size()){
			--n_Active;
			if(n_Active == -1) {
				b_Running = false;
				b_Finished = true;
				emit finished();
			}
			return;
		}

		auto range = m_RequiredBlocks.at(n_Done++);
		QNetworkRequest request = makeRangeRequest(m_Url, range);
		auto rangeReply = new RangeReply(index, m_Manager->get(request), range);
		m_ActiveRequests[index] = rangeReply;

		connect(rangeReply, SIGNAL(canceled(int)),
				this, SLOT(handleRangeReplyCancel(int)),
				Qt::QueuedConnection);

		connect(rangeReply, SIGNAL(restarted(int)),
				this, SLOT(handleRangeReplyRestart(int)),
				Qt::QueuedConnection);

		connect(rangeReply, SIGNAL(error(QNetworkReply::NetworkError, int, bool)),
				this, SLOT(handleRangeReplyError(QNetworkReply::NetworkError, int, bool)),
				Qt::QueuedConnection);

		connect(rangeReply, SIGNAL(finished(qint32, qint32, QByteArray*, int)),
				this, SLOT(handleRangeReplyFinished(qint32, qint32, QByteArray*, int)),
				Qt::QueuedConnection);	

		connect(rangeReply, SIGNAL(progress(qint64, int)), 
				 this, SLOT(handleRangeReplyProgress(qint64, int)),
				 Qt::QueuedConnection);


}

void RangeDownloaderPrivate::handleRangeReplyProgress(qint64 bytesRc, int index) {
	Q_UNUSED(index);
	n_RecievedBytes += bytesRc;
	qint64 totalBytesRecieved = n_BytesWritten + n_RecievedBytes;
	
	if(totalBytesRecieved >= n_TotalSize) {
		totalBytesRecieved = n_TotalSize;
	}

	QString sUnit;
        int nPercentage = static_cast<int>(
                              (static_cast<float>
                               (totalBytesRecieved) * 100.0
                              ) / static_cast<float>
                              (n_TotalSize)
                          );

        double nSpeed =  (n_RecievedBytes) * 1000.0 / m_ElapsedTimer.elapsed();
        if (nSpeed < 1024) {
            sUnit = "bytes/sec";
        } else if (nSpeed < 1024 * 1024) {
            nSpeed /= 1024;
            sUnit = "kB/s";
        } else {
            nSpeed /= 1024 * 1024;
            sUnit = "MB/s";
        }
        emit progress(nPercentage, totalBytesRecieved, n_TotalSize, nSpeed, sUnit);	
}
