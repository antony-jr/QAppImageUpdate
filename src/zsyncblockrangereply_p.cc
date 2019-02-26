/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018-2019, Antony jr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @filename    : zsyncblockrangereply_p.cc
 * @description : The is where ZsyncBlockRangeReplyPrivate is implemented.
 * This private class is responsible to handle a single QNetworkReply.
 * Since each QNetworkReply works parallel , This private class provides a
 * a simple way to control a single QNetworkReply via signals and slots and
 * also submits the download data on the fly to ZsyncWriterPrivate.
*/
#include "../include/zsyncblockrangereply_p.hpp"
#include "../include/zsyncwriter_p.hpp"

using namespace AppImageUpdaterBridge;

ZsyncBlockRangeReplyPrivate::ZsyncBlockRangeReplyPrivate(ZsyncWriterPrivate *deltaWriter,
        QNetworkReply *reply,
        qint32 rangeFrom,
        qint32 rangeTo)
    : QObject(reply),
      n_RangeFrom(rangeFrom),
      n_RangeTo(rangeTo)
{
    downloadSpeed.start();
    p_RawData.reset(new QByteArray);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleError(QNetworkReply::NetworkError)));
    /*
     * Check if this is an invalid range , if so then prepare for
     * a simple sequential download.
    */
    if(!(n_RangeFrom || n_RangeTo)) {
        connect(reply, &QNetworkReply::downloadProgress,
                this, &ZsyncBlockRangeReplyPrivate::handleSeqProgress);
        connect(this, &ZsyncBlockRangeReplyPrivate::sendData,
                deltaWriter, &ZsyncWriterPrivate::writeSeqRaw, Qt::QueuedConnection);
    } else {
        connect(reply, &QNetworkReply::downloadProgress,
                this, &ZsyncBlockRangeReplyPrivate::handleProgress);
        connect(this,  &ZsyncBlockRangeReplyPrivate::sendBlockDataToWriter,
                deltaWriter, &ZsyncWriterPrivate::writeBlockRanges, Qt::QueuedConnection);
    }
    connect(reply, &QNetworkReply::finished,
            this, &ZsyncBlockRangeReplyPrivate::handleFinished, Qt::QueuedConnection);
    connect(this,  &ZsyncBlockRangeReplyPrivate::cancelReply,
            reply, &QNetworkReply::abort, Qt::QueuedConnection);
    return;
}

ZsyncBlockRangeReplyPrivate::~ZsyncBlockRangeReplyPrivate()
{
    return;
}

void ZsyncBlockRangeReplyPrivate::cancel(void)
{
    emit cancelReply();
    return;
}

void ZsyncBlockRangeReplyPrivate::handleFinished(void)
{
    auto reply = (QNetworkReply*)QObject::sender();
    disconnect(this, &ZsyncBlockRangeReplyPrivate::cancelReply, reply, &QNetworkReply::abort);
    disconnect(reply, &QNetworkReply::downloadProgress, this, &ZsyncBlockRangeReplyPrivate::handleSeqProgress);
    disconnect(reply, &QNetworkReply::downloadProgress, this, &ZsyncBlockRangeReplyPrivate::handleProgress);

    if(!p_RawData->isEmpty()) {
        p_RawData->append(reply->readAll());

        /* Send all the data to ZsyncWriterPrivate. */
        emit sendBlockDataToWriter(n_RangeFrom, n_RangeTo, p_RawData.take());
    }
    emit finished();
    return;
}

void ZsyncBlockRangeReplyPrivate::handleError(QNetworkReply::NetworkError ecode)
{
    auto reply = (QNetworkReply*)QObject::sender();
    disconnect(reply, &QNetworkReply::finished, this, &ZsyncBlockRangeReplyPrivate::handleFinished);

    if(ecode == QNetworkReply::OperationCanceledError) {
        emit canceled();
        return;
    }
    emit error(ecode);
    return;
}

void ZsyncBlockRangeReplyPrivate::handleSeqProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    auto reply = (QNetworkReply*)QObject::sender();

    if(!reply->isReadable()) {
        return;
    }

    auto data = new QByteArray(reply->readAll());

    /* Send whatever data got to ZsyncWriterPrivate which
     * will write this sequentially.
    */
    emit sendData(data);

    int nPercentage = static_cast<int>(
                          (static_cast<float>
                           (bytesReceived) * 100.0
                          ) / static_cast<float>
                          (bytesTotal)
                      );

    QString sUnit;
    double nSpeed =  bytesReceived * 1000.0 / downloadSpeed.elapsed();
    if (nSpeed < 1024) {
        sUnit = "bytes/sec";
    } else if (nSpeed < 1024 * 1024) {
        nSpeed /= 1024;
        sUnit = "kB/s";
    } else {
        nSpeed /= 1024 * 1024;
        sUnit = "MB/s";
    }
    emit seqProgress(nPercentage, bytesReceived, bytesTotal, nSpeed, sUnit);
    return;
}

void ZsyncBlockRangeReplyPrivate::handleProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesTotal);
    auto reply = (QNetworkReply*)QObject::sender();
    if(!reply->isReadable()) {
        return;
    }

    qint64 nowReceived = bytesReceived - n_PreviousBytesReceived;
    n_PreviousBytesReceived = bytesReceived;

    p_RawData->append(reply->readAll());

    QString sUnit;
    double nSpeed =  bytesReceived * 1000.0 / downloadSpeed.elapsed();
    if (nSpeed < 1024) {
        sUnit = "bytes/sec";
    } else if (nSpeed < 1024 * 1024) {
        nSpeed /= 1024;
        sUnit = "kB/s";
    } else {
        nSpeed /= 1024 * 1024;
        sUnit = "MB/s";
    }
    emit progress(nowReceived, nSpeed, sUnit);
    return;
}

