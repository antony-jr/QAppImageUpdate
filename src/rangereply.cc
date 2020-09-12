#include "rangereply.hpp"
#include "rangereply_p.hpp"
#include "helpers_p.hpp"

RangeReply::RangeReply(int index, QNetworkReply *reply, const QPair<qint32, qint32> &range, qint32 blocks)
	 	: QObject() {
	m_Private = QSharedPointer<RangeReplyPrivate>(
			new RangeReplyPrivate(index, reply, range, blocks));
		
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

RangeReply::~RangeReply() {
		getMethod(m_Private.data(), "destroy()")
			.invoke(m_Private.data(), Qt::QueuedConnection);
}


// Public Slots
void RangeReply::destroy() {
	    getMethod(m_Private.data(), "destroy()")
		.invoke(m_Private.data(), Qt::QueuedConnection);	
}

void RangeReply::retry(int timeout) {
	    getMethod(m_Private.data(), "retry(int)")
		.invoke(m_Private.data(), Qt::QueuedConnection, Q_ARG(int, timeout));
	
}

void RangeReply::cancel() {
	  getMethod(m_Private.data(), "cancel()")
		.invoke(m_Private.data(), Qt::QueuedConnection);
	
}
