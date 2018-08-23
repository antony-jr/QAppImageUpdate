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
 * @filename    : ZsyncBlockRangeDownloader_p.cc
 * @description : This the main class which manages all block requests and reply
 * also emits the progress overall , This is where the class is implemented.
*/
#include <ZsyncBlockRangeDownloader_p.hpp>
#include <ZsyncBlockRangeReply_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncWriter_p.hpp>

using namespace AppImageUpdaterBridge;

/*
 * This is the main class which manages the block downloads for ZsyncWriterPrivate ,
 * This class provides progress overall and also gives the control to cancel the download
 * anytime without any kind of data races.
*/
ZsyncBlockRangeDownloaderPrivate::ZsyncBlockRangeDownloaderPrivate(ZsyncRemoteControlFileParserPrivate *parser,
        ZsyncWriterPrivate *writer,
        QNetworkAccessManager *nm)
    : QObject(),
      _pParser(parser),
      _pWriter(writer),
      _pManager(nm)
{
    connect(_pWriter, &ZsyncWriterPrivate::finished,
            this, &ZsyncBlockRangeDownloaderPrivate::initDownloader);
    connect(_pWriter, &ZsyncWriterPrivate::blockRange,
            this, &ZsyncBlockRangeDownloaderPrivate::handleBlockRange,Qt::QueuedConnection);
    return;
}

ZsyncBlockRangeDownloaderPrivate::~ZsyncBlockRangeDownloaderPrivate()
{
    return;
}

/* Cancels all ZsyncBlockRangeReplyPrivate QObjects. */
void ZsyncBlockRangeDownloaderPrivate::cancel(void)
{
    _bCancelRequested = true;
    emit cancelAllReply();
    return;
}

/* Starts the download of all the required blocks ,
 * Typically this is connected to the finish signal of ZsyncWriterPrivate.
*/
void ZsyncBlockRangeDownloaderPrivate::initDownloader(bool doStart)
{
    if(!doStart) {
        emit completelyFinished();
        return;
    }

    auto writerMetaObject = _pWriter->metaObject();

    _nBytesReceived = _pWriter->getBytesWritten(); // set atomic integer.
    _nBytesTotal = _pParser->getTargetFileLength();
    _uTargetFileUrl = _pParser->getTargetFileUrl();

    /*
     * Start the download , if the host cannot accept range requests then
     * blockRange signal will return with both 'from' and 'to' ranges 0.
    */
    emit started();
    writerMetaObject->method(writerMetaObject->indexOfMethod(QMetaObject::normalizedSignature("getBlockRanges(void)")))
    .invoke(_pWriter, Qt::QueuedConnection);
    return;
}

/* This is connected to the blockRange signal of ZsyncWriterPrivate and starts the
 * download of a new block range.
*/
void ZsyncBlockRangeDownloaderPrivate::handleBlockRange(qint32 fromRange, qint32 toRange)
{
    QNetworkRequest request;
    request.setUrl(_uTargetFileUrl);
    if(fromRange || toRange) {
        QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(fromRange) + "-";
        rangeHeaderValue += QByteArray::number(toRange);
        request.setRawHeader("Range", rangeHeaderValue);
    }
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    ++_nBlockReply;

    auto blockReply = new ZsyncBlockRangeReplyPrivate(_pWriter, _pManager->get(request), fromRange, toRange);
    connect(this, &ZsyncBlockRangeDownloaderPrivate::cancelAllReply,
            blockReply, &ZsyncBlockRangeReplyPrivate::cancel);
    connect(blockReply, &ZsyncBlockRangeReplyPrivate::canceled,
            this, &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyCancel,
            Qt::QueuedConnection);
    if(!(fromRange || toRange)) {
        connect(blockReply, &ZsyncBlockRangeReplyPrivate::seqProgress,
                this, &ZsyncBlockRangeDownloaderPrivate::progress, Qt::DirectConnection);
        connect(blockReply, &ZsyncBlockRangeReplyPrivate::finished,
                this, &ZsyncBlockRangeDownloaderPrivate::finished,
                Qt::DirectConnection);
        connect(blockReply, &ZsyncBlockRangeReplyPrivate::canceled,
                this, &ZsyncBlockRangeDownloaderPrivate::canceled,
                Qt::DirectConnection);
    } else {
        connect(blockReply, &ZsyncBlockRangeReplyPrivate::progress,
                this, &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyProgress,
                Qt::QueuedConnection);
        connect(blockReply, &ZsyncBlockRangeReplyPrivate::finished,
                this, &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyFinished,
                Qt::QueuedConnection);
        connect(blockReply, &ZsyncBlockRangeReplyPrivate::canceled,
                this, &ZsyncBlockRangeDownloaderPrivate::handleBlockReplyCancel,
                Qt::QueuedConnection);
    }
    connect(blockReply, &ZsyncBlockRangeReplyPrivate::error,
            this, &ZsyncBlockRangeDownloaderPrivate::error,
            Qt::DirectConnection);
    return;
}

/* Calculates the overall progress and also emits it when done. */
void ZsyncBlockRangeDownloaderPrivate::handleBlockReplyProgress(qint64 bytesReceived, double speed, QString units)
{
    _nBytesReceived += bytesReceived;
    int nPercentage = static_cast<int>(
                          (static_cast<float>
                           (_nBytesReceived.load()) * 100.0
                          ) / static_cast<float>
                          (_nBytesTotal)
                      );
    emit progress(nPercentage, _nBytesReceived.load(), _nBytesTotal, speed, units);
    return;
}

/* Boilerplate code to finish a ZsyncBlockRangeReplyPrivate. */
void ZsyncBlockRangeDownloaderPrivate::handleBlockReplyFinished(void)
{
    --_nBlockReply;

    if(_nBlockReply == 0) {
        if(_bCancelRequested == true) {
            _bCancelRequested = false;
            emit canceled();
        } else {
            emit finished();
        }
    }
    return;
}

/* When canceled , check if the current emitting ZsyncBlockRangeReply is the last
 * object emitting it , If so then emit canceled from this class.
*/
void ZsyncBlockRangeDownloaderPrivate::handleBlockReplyCancel(void)
{
    auto blockReply = (ZsyncBlockRangeReplyPrivate*)QObject::sender();
    blockReply->deleteLater();

    --_nBlockReply;

    if(_nBlockReply == 0) {
        _bCancelRequested = false;
        emit canceled();
    }
    return;
}
