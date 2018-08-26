/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018, Antony jr
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
 * @filename    : ZsyncBlockRangeDownloader_p.hpp
 * @description : This the main class which manages all block requests and reply
 * also emits the progress overall , This is where the class is described.
*/
#ifndef ZSYNC_BLOCK_RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#define ZSYNC_BLOCK_RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
#include <QtGlobal>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncWriter_p.hpp>

namespace AppImageUpdaterBridge
{
class ZsyncBlockRangeDownloaderPrivate : public QObject
{
    Q_OBJECT
public:
    ZsyncBlockRangeDownloaderPrivate(ZsyncRemoteControlFileParserPrivate*,ZsyncWriterPrivate*,QNetworkAccessManager*);
    ~ZsyncBlockRangeDownloaderPrivate();

public Q_SLOTS:
    void cancel(void);

private Q_SLOTS:
    void initDownloader(void);
    void handleBlockRange(qint32,qint32);
    void handleBlockReplyFinished(void);
    void handleBlockReplyCancel(void);
    void handleBlockReplyProgress(qint64, double, QString);

Q_SIGNALS:
    void progress(int, qint64, qint64, double, QString);
    void cancelAllReply(void);
    void canceled(void);
    void error(QNetworkReply::NetworkError);
    void started(void);
    void finished(void);

private:
    QUrl _uTargetFileUrl;
    qint64 _nBytesTotal = 0;
    QAtomicInteger<qint64> _nBytesReceived = 0;
    QAtomicInteger<bool> _bCancelRequested = false;
    QAtomicInteger<qint64> _nBlockReply = 0;
    ZsyncRemoteControlFileParserPrivate *_pParser = nullptr;
    ZsyncWriterPrivate *_pWriter = nullptr;
    QNetworkAccessManager *_pManager = nullptr;
};
}
#endif // ZSYNC_BLOCK_RANGE_DOWNLOADER_PRIVATE_HPP_INCLUDED
