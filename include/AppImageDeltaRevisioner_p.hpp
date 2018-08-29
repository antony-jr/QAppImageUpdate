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
 * @filename    : AppImageDeltaRevisioner_p.hpp
 * @description : This where the delta revisioner is described.
 * Delta Revisioner is the private API to manage the entire revision
 * of a AppImage. From retrival of the embeded information from the
 * AppImage to the retrival of required remaining block ranges.
 * This is a controller to all the internal mechanisms , Handled neatly
 * via Qt's signals and slots.
 *
 * Since internally there is a need to disconnect and connect certain 
 * signals and slots in a Queued manner , We still need a even higher
 * class to manage that which will be the public API.
 *
*/
#ifndef APPIMAGE_DELTA_REVISIONER_PRIVATE_HPP_INCLUDED
#define APPIMAGE_DELTA_REVISIONER_PRIVATE_HPP_INCLUDED
#include <QFile>
#include <QtGlobal>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QThread>
#include <AppImageUpdaterBridgeErrorCodes.hpp>
#include <AppImageUpdaterBridgeStatusCodes.hpp>

/* Private Libraries needed. */
#include <AppImageUpdateInformation_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncWriter_p.hpp>
#include <ZsyncBlockRangeDownloader_p.hpp>

namespace AppImageUpdaterBridge
{
class AppImageDeltaRevisionerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit AppImageDeltaRevisionerPrivate(bool singleThreaded = true, QObject *parent = nullptr);
    explicit AppImageDeltaRevisionerPrivate(const QString&, bool singleThreaded = true, QObject *parent = nullptr);
    explicit AppImageDeltaRevisionerPrivate(QFile *, bool singleThreaded = true, QObject *parent = nullptr);
    ~AppImageDeltaRevisionerPrivate();

    static QString errorCodeToString(short);
    static QString statusCodeToString(short);

public Q_SLOTS:
    void start(void);
    void cancel(void);
    void setAppImage(const QString&);
    void setAppImage(QFile*);
    void setShowLog(bool);
    void setOutputDirectory(const QString&);
    void getAppImageEmbededInformation(void);
    void checkForUpdate(void);
    void clear(void);

    QNetworkReply::NetworkError getNetworkError(void);
private Q_SLOTS:
    void doStart(QJsonObject);
    void handleZsyncRemoteControlFileParserError(short);
    void handleNetworkError(QNetworkReply::NetworkError);
    void handleIndeterminateProgress(int);
    void handleUpdateCheckInformation(QJsonObject);

Q_SIGNALS:
    void started(void);
    void canceled(void);
    void finished(QJsonObject, QString);
    void embededInformation(QJsonObject);
    void updateAvailable(bool, QJsonObject);
    void statusChanged(short);
    void error(short);
    void progress(int, qint64, qint64, double, QString);
    void logger(QString, QString);
private:
    QAtomicInteger<int> _pRecentNetworkErrorCode = QNetworkReply::NoError;
    QScopedPointer<AppImageUpdateInformationPrivate> _pUpdateInformation;
    QScopedPointer<ZsyncRemoteControlFileParserPrivate> _pControlFileParser;
    QScopedPointer<ZsyncWriterPrivate> _pDeltaWriter;
    QScopedPointer<ZsyncBlockRangeDownloaderPrivate> _pBlockDownloader;
    QScopedPointer<QThread> _pSharedThread;
    QScopedPointer<QNetworkAccessManager> _pSharedNetworkAccessManager;
};
}

#endif // APPIMAGE_DELTA_REVISIONER_PRIVATE_HPP_INCLUDED
