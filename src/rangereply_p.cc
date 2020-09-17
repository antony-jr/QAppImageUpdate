#include <QDebug>
#include "rangereply_p.hpp"

RangeReplyPrivate::RangeReplyPrivate(int index, QNetworkReply *reply, const QPair<qint32, qint32> &blockRange) {	
		n_Index = index;
		n_FromBlock = blockRange.first;
		n_ToBlock = blockRange.second;
		m_Request = reply->request();
		m_Manager = reply->manager();
		m_Reply.reset(reply);
		m_Data.reset(new QByteArray);
		m_Timer.setSingleShot(true);

		connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
			this, SLOT(handleData(qint64, qint64)),
			Qt::QueuedConnection);
		connect(reply, SIGNAL(finished(void)),
			this, SLOT(handleFinish(void)),
			Qt::QueuedConnection);
		connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
			this, SLOT(handleError(QNetworkReply::NetworkError)),
			Qt::QueuedConnection);
		//// Connect timer for retry action
		connect(&m_Timer, SIGNAL(timeout()),
			 this, SLOT(restart()));
}

RangeReplyPrivate::~RangeReplyPrivate() {
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



///// Public Slots
// =========================	

void RangeReplyPrivate::destroy() {
		if(b_Retrying) {
			m_Timer.stop();
		}	
		else if(b_Running) {	
			m_Reply->disconnect();
			m_Reply->abort();
		}
		
		resetInternalFlags();	
		b_Halted = true;

		disconnect();
		this->deleteLater();
}


void RangeReplyPrivate::retry(int timeout) {
		if(b_Running || b_Finished || b_Halted) {
			return;
		}
	
		resetInternalFlags();
		b_Retrying = true;

		m_Timer.setInterval(timeout);
		m_Timer.start();
}

void RangeReplyPrivate::cancel() {
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

/// Private Slots
//=================================

void RangeReplyPrivate::resetInternalFlags(bool value) {
	b_Halted = b_Running = b_Finished = b_CancelRequested = b_Retrying = value;
}

void RangeReplyPrivate::restart() {
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
			Qt::QueuedConnection);
		connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
			this, SLOT(handleError(QNetworkReply::NetworkError)),
			Qt::QueuedConnection);
	
		b_Running = true;
		emit restarted(n_Index);
}

void RangeReplyPrivate::handleData(qint64 bytesRec, qint64 bytesTotal) {
		emit progress(bytesRec, bytesTotal, n_Index);

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


void RangeReplyPrivate::handleError(QNetworkReply::NetworkError code) {
		if(b_Halted) {
			return;
		}

		if(code == QNetworkReply::OperationCanceledError || b_CancelRequested) {
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

void RangeReplyPrivate::handleFinish() {
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
		emit finished(n_FromBlock, n_ToBlock,  m_Data.take(), n_Index);	
		
		m_Reply->disconnect();
}
