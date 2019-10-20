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
 * @filename    : zsyncblockrangereply_p.hpp
 * @description : The is where ZsyncBlockRangeReplyPrivate is described.
 * This private class is responsible to handle a single QNetworkReply.
 * Since each QNetworkReply works parallel , This private class provides a
 * a simple way to control a single QNetworkReply via signals and slots and
 * also submits the download data on the fly to ZsyncWriterPrivate.
*/
#ifndef ZSYNC_BLOCK_RANGE_REPLY_PRIVATE_HPP_INCLUDED
#define ZSYNC_BLOCK_RANGE_REPLY_PRIVATE_HPP_INCLUDED
#include <QtGlobal>
#include <QObject>
#include <QNetworkReply>

#include "zsyncwriter_p.hpp"

namespace AppImageUpdaterBridge {
class ZsyncBlockRangeReplyPrivate : public QObject {
    Q_OBJECT
  public:
    ZsyncBlockRangeReplyPrivate(ZsyncWriterPrivate*,QNetworkReply*,qint32,qint32);
    ~ZsyncBlockRangeReplyPrivate();

  public Q_SLOTS:
    void cancel(void);

  private Q_SLOTS:
    void handleError(QNetworkReply::NetworkError);
    void handleFinished(void);
    void handleSeqProgress(qint64, qint64);
    void handleProgress(qint64, qint64);

  Q_SIGNALS:
    void cancelReply(void);
    void canceled(void);
    void seqProgress(int, qint64, qint64, double, QString);
    void progress(qint64, double, QString);
    void error(QNetworkReply::NetworkError);
    void finished(void);
    void sendData(QByteArray*);
    void sendBlockDataToWriter(qint32, qint32, QByteArray *);

  private:
    QTime downloadSpeed;
    QScopedPointer<QByteArray> p_RawData;
    qint64 n_PreviousBytesReceived = 0;
    qint32 n_RangeFrom = 0,
           n_RangeTo = 0;
};
}
#endif // ZSYNC_BLOCK_RANGE_REPLY_PRIVATE_HPP_INCLUDED
