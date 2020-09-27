#include <QDebug>
#include "rangereply_p.hpp"

/// The number of times a request can be retried if the error
/// is not severe.
#define FAIL_THRESHOLD 50

RangeReplyPrivate::RangeReplyPrivate(int index, QNetworkReply *reply, const QPair<qint32, qint32> &blockRange) {
    n_Index = index;
    n_BytesRecieved = 0;
    n_FromBlock = blockRange.first;
    n_ToBlock = blockRange.second;
    n_Fails = 0;
    m_Request = reply->request();
    m_Manager = reply->manager();
    b_FullDownload = (!n_FromBlock && !n_ToBlock); // Careful on this logic expression
    m_Reply.reset(reply);
    if(!b_FullDownload) {
        m_Data.reset(new QByteArray);
    }
    m_Timer.setSingleShot(true);

    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(handleData(qint64, qint64)),
            Qt::QueuedConnection);
    connect(reply, SIGNAL(finished()),
            this, SLOT(handleFinish()),
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
    } else if(b_Retrying) {
        m_Timer.stop();
    } else if(b_Running) {
        m_Reply->disconnect();
        m_Reply->abort();
    }
}



///// Public Slots
// =========================

void RangeReplyPrivate::destroy() {
    if(b_Retrying) {
        m_Timer.stop();
    } else if(b_Running) {
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
            b_CancelRequested) {
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

    m_Reply.reset(m_Manager->get(m_Request));

    auto reply = m_Reply.data();
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(handleData(qint64, qint64)),
            Qt::QueuedConnection);
    connect(reply, SIGNAL(finished()),
            this, SLOT(handleFinish()),
            Qt::QueuedConnection);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleError(QNetworkReply::NetworkError)),
            Qt::QueuedConnection);

    b_Running = true;
    emit restarted(n_Index);
}

void RangeReplyPrivate::handleData(qint64 bytesRec, qint64 bytesTotal) {
    Q_UNUSED(bytesTotal);
    if(b_CancelRequested) {
        m_Reply->abort();
        return;
    }

    qint64 actualBytesRec = bytesRec - n_BytesRecieved;
    n_BytesRecieved = bytesRec;

    emit progress(actualBytesRec, n_Index);

    if(m_Reply.isNull() || b_Halted) {
        return;
    }

    if(m_Reply->error() != QNetworkReply::NoError) {
        return;
    }

    if(m_Reply->isOpen() && m_Reply->isReadable()) {
        if(!b_FullDownload) {
            m_Data->append(m_Reply->readAll());
        } else {
            QByteArray *datafrag = new QByteArray;
            datafrag->append(m_Reply->readAll());
            emit data(datafrag, false);
        }

    }
}


void RangeReplyPrivate::handleError(QNetworkReply::NetworkError code) {
    if(b_Halted) {
        return;
    }

    if(code == QNetworkReply::OperationCanceledError || b_CancelRequested) {
        if(!b_FullDownload) {
            m_Data->clear();
        }
        m_Reply->disconnect();

        resetInternalFlags();
        b_Canceled = true;

        emit canceled(n_Index);
        return;
    }
    resetInternalFlags();
    ++n_Fails;
    bool thresholdReached = (n_Fails > FAIL_THRESHOLD);
    emit error(code, n_Index, thresholdReached);
    return;

}

void RangeReplyPrivate::handleFinish() {
    if(b_Halted) {
        return;
    }

    if(b_CancelRequested) {
        if(!b_FullDownload) {
            m_Data->clear();
        }
        resetInternalFlags();
        b_Canceled = true;

        emit canceled(n_Index);
        return;
    }
    resetInternalFlags();
    b_Finished = true;

    /// Append any data that is left.
    if(!b_FullDownload) {
        m_Data->append(m_Reply->readAll());
    }

    /// Finish the range reply
    if(!b_FullDownload) {
        emit finished(n_FromBlock, n_ToBlock, m_Data.take(), n_Index);
    } else {
        QByteArray *datafrag = new QByteArray;
        datafrag->append(m_Reply->readAll());
        emit finished(n_FromBlock, n_ToBlock, datafrag, n_Index);
    }
    m_Reply->disconnect();
}
