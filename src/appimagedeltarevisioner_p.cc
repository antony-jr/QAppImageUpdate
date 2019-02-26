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
 * @filename    : appimagedeltarevisioner_p.cc
 * @description : This where the delta revisioner is implemented.
 * Delta Revisioner is the private API to manage the entire revision
 * of a AppImage. From retrival of the embeded information from the
 * AppImage to the retrival of required remaining block ranges.
 * This is a controller to all the internal mechanisms , Handled neatly
 * via Qt's signals and slots.
 *
*/

#include "../include/appimagedeltarevisioner_p.hpp"

using namespace AppImageUpdaterBridge;

static QMetaMethod getMethod(QScopedPointer<AppImageUpdateInformationPrivate> &object, const char *function)
{
    auto metaObject = object->metaObject();
    return metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(function)));
}

static QMetaMethod getMethod(QScopedPointer<ZsyncRemoteControlFileParserPrivate> &object, const char *function)
{
    auto metaObject = object->metaObject();
    return metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(function)));
}

static QMetaMethod getMethod(QScopedPointer<ZsyncWriterPrivate> &object, const char *function)
{
    auto metaObject = object->metaObject();
    return metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(function)));
}

static QMetaMethod getMethod(QScopedPointer<ZsyncBlockRangeDownloaderPrivate> &object, const char *function)
{
    auto metaObject = object->metaObject();
    return metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(function)));
}




AppImageDeltaRevisionerPrivate::AppImageDeltaRevisionerPrivate(bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    setObjectName("AppImageDeltaRevisionerPrivate");
    if(!singleThreaded) {
        p_SharedThread.reset(new QThread);
        p_SharedThread->start();
    }
    p_SharedNetworkAccessManager.reset(new QNetworkAccessManager);
    p_UpdateInformation.reset(new AppImageUpdateInformationPrivate);
    p_DeltaWriter.reset(new ZsyncWriterPrivate);
    if(!singleThreaded) {
        p_SharedNetworkAccessManager->moveToThread(p_SharedThread.data());
        p_UpdateInformation->moveToThread(p_SharedThread.data());
        p_DeltaWriter->moveToThread(p_SharedThread.data());
    }
    p_ControlFileParser.reset(new ZsyncRemoteControlFileParserPrivate(p_SharedNetworkAccessManager.data()));
    p_BlockDownloader.reset(new ZsyncBlockRangeDownloaderPrivate(p_DeltaWriter.data(),
                            p_SharedNetworkAccessManager.data()));
    if(!singleThreaded) {
        p_ControlFileParser->moveToThread(p_SharedThread.data());
        p_BlockDownloader->moveToThread(p_SharedThread.data());
    }
    p_ControlFileParser->setObjectName("ZsyncRemoteControlFileParserPrivate");
    p_UpdateInformation->setObjectName("AppImageUpdateInformationPrivate");
    p_DeltaWriter->setObjectName("ZsyncWriterPrivate");
    p_BlockDownloader->setObjectName("ZsyncBlockRangeDownloaderPrivate");
    p_UpdateInformation->setLoggerName("UpdateInformation");
    p_ControlFileParser->setLoggerName("ControlFileParser");
    p_DeltaWriter->setLoggerName("AppImageDeltaRevisioner");
    connect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::statusChanged,
            this, &AppImageDeltaRevisionerPrivate::statusChanged, Qt::DirectConnection);
    connect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::progress,
            this, &AppImageDeltaRevisionerPrivate::handleIndeterminateProgress);
    connect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::info,
            this, &AppImageDeltaRevisionerPrivate::embededInformation, Qt::DirectConnection);
    connect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
            this, &AppImageDeltaRevisionerPrivate::error, Qt::DirectConnection);
    connect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::logger,
            this, &AppImageDeltaRevisionerPrivate::logger, Qt::DirectConnection);
    connect(p_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::statusChanged,
            this, &AppImageDeltaRevisionerPrivate::statusChanged, Qt::DirectConnection);
    connect(p_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::progress,
            this, &AppImageDeltaRevisionerPrivate::handleIndeterminateProgress);
    connect(p_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
            this, &AppImageDeltaRevisionerPrivate::error, Qt::DirectConnection);
    connect(p_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::logger,
            this, &AppImageDeltaRevisionerPrivate::logger, Qt::DirectConnection);
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::statusChanged,
            this, &AppImageDeltaRevisionerPrivate::statusChanged, Qt::DirectConnection);
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::error,
            this, &AppImageDeltaRevisionerPrivate::error, Qt::DirectConnection);
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::logger,
            this, &AppImageDeltaRevisionerPrivate::logger, Qt::DirectConnection);
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::started,
            this, &AppImageDeltaRevisionerPrivate::started, Qt::DirectConnection);
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::canceled,
            this, &AppImageDeltaRevisionerPrivate::canceled, Qt::DirectConnection);
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::finished,
            this, &AppImageDeltaRevisionerPrivate::finished, Qt::DirectConnection);
    connect(p_BlockDownloader.data(), &ZsyncBlockRangeDownloaderPrivate::error,
            this,  &AppImageDeltaRevisionerPrivate::error, Qt::DirectConnection);
    connect(p_BlockDownloader.data(), &ZsyncBlockRangeDownloaderPrivate::canceled,
            this, &AppImageDeltaRevisionerPrivate::canceled, Qt::DirectConnection);
    connect(p_BlockDownloader.data(), &ZsyncBlockRangeDownloaderPrivate::progress,
            this, &AppImageDeltaRevisionerPrivate::progress, Qt::DirectConnection);
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
            this, &AppImageDeltaRevisionerPrivate::progress, Qt::DirectConnection);
    connect(p_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
            p_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration, Qt::QueuedConnection);
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
            p_DeltaWriter.data(), &ZsyncWriterPrivate::start, Qt::QueuedConnection);
}

AppImageDeltaRevisionerPrivate::AppImageDeltaRevisionerPrivate(const QString &AppImagePath, bool singleThreaded, QObject *parent)
    : AppImageDeltaRevisionerPrivate(singleThreaded, parent)
{
    setAppImage(AppImagePath);
    return;
}

AppImageDeltaRevisionerPrivate::AppImageDeltaRevisionerPrivate(QFile *AppImage, bool singleThreaded, QObject *parent)
    : AppImageDeltaRevisionerPrivate(singleThreaded, parent)
{
    setAppImage(AppImage);
    return;
}

AppImageDeltaRevisionerPrivate::~AppImageDeltaRevisionerPrivate()
{
    if(!p_SharedThread.isNull()) {
        p_SharedThread->quit();
        p_SharedThread->wait();
    }
    return;
}

void AppImageDeltaRevisionerPrivate::start(void)
{
    connect(p_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
            p_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)), Qt::UniqueConnection);
    connect(p_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
            p_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)), Qt::UniqueConnection);
    connect(p_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
            this, SLOT(doStart(QJsonObject)), Qt::UniqueConnection);
    disconnect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::info,
               this, &AppImageDeltaRevisionerPrivate::embededInformation);
    getAppImageEmbededInformation();
    return;
}

void AppImageDeltaRevisionerPrivate::doStart(QJsonObject information)
{
    disconnect(p_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               p_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(p_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               p_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(p_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
               this, SLOT(doStart(QJsonObject)));
    connect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::info,
            this, &AppImageDeltaRevisionerPrivate::embededInformation, Qt::UniqueConnection);

    if(information.isEmpty()) {
        return;
    }

    auto embededUpdateInformation = information["EmbededUpdateInformation"].toObject();
    auto remoteTargetFileSHA1Hash = information["RemoteTargetFileSHA1Hash"].toString();
    QString localAppImageSHA1Hash = embededUpdateInformation["FileInformation"].toObject()["AppImageSHA1Hash"].toString(),
            localAppImagePath = embededUpdateInformation["FileInformation"].toObject()["AppImageFilePath"].toString();
    auto oldVersionInformation = embededUpdateInformation["FileInformation"].toObject();

    if(localAppImageSHA1Hash != remoteTargetFileSHA1Hash) {
        auto metaObject = p_ControlFileParser->metaObject();
        metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getZsyncInformation(void)")))
        .invoke(p_ControlFileParser.data(), Qt::QueuedConnection);
    } else {
        /* Current Version is the new version. */
        QJsonObject newVersionDetails {
            { "AbsolutePath", oldVersionInformation["AppImageFilePath"].toString() },
            { "Sha1Hash", oldVersionInformation["AppImageSHA1Hash"].toString() }
        };
        emit finished(newVersionDetails, oldVersionInformation["AppImageFilePath"].toString());
    }
    return;
}

void AppImageDeltaRevisionerPrivate::cancel(void)
{
    getMethod(p_DeltaWriter,"cancel(void)").invoke(p_DeltaWriter.data(), Qt::QueuedConnection);
    getMethod(p_BlockDownloader, "cancel(void)").invoke(p_BlockDownloader.data(), Qt::QueuedConnection);
    return;
}

void AppImageDeltaRevisionerPrivate::setAppImage(const QString &AppImagePath)
{
    clear();
    getMethod(p_UpdateInformation, "setAppImage(const QString&)").invoke(p_UpdateInformation.data(),
            Qt::QueuedConnection,
            Q_ARG(QString,AppImagePath));
    return;
}

void AppImageDeltaRevisionerPrivate::setAppImage(QFile *AppImage)
{
    getMethod(p_UpdateInformation, "setAppImage(QFile *)").invoke(p_UpdateInformation.data(),
            Qt::QueuedConnection,
            Q_ARG(QFile*,AppImage));
    return;
}

void AppImageDeltaRevisionerPrivate::setShowLog(bool choice)
{
    getMethod(p_UpdateInformation, "setShowLog(bool)").invoke(p_UpdateInformation.data(),
            Qt::QueuedConnection, Q_ARG(bool, choice));
    getMethod(p_ControlFileParser, "setShowLog(bool)").invoke(p_ControlFileParser.data(),
            Qt::QueuedConnection, Q_ARG(bool, choice));
    getMethod(p_DeltaWriter, "setShowLog(bool)").invoke(p_DeltaWriter.data(),
            Qt::QueuedConnection, Q_ARG(bool, choice));
    return;
}

void AppImageDeltaRevisionerPrivate::setOutputDirectory(const QString &dir)
{
    getMethod(p_DeltaWriter, "setOutputDirectory(const QString&)").invoke(p_DeltaWriter.data(),
            Qt::QueuedConnection,
            Q_ARG(QString, dir));
    return;
}

void AppImageDeltaRevisionerPrivate::getAppImageEmbededInformation(void)
{
    getMethod(p_UpdateInformation, "getInfo(void)").invoke(p_UpdateInformation.data(), Qt::QueuedConnection);
    return;
}

void AppImageDeltaRevisionerPrivate::clear(void)
{
    getMethod(p_UpdateInformation, "clear(void)").invoke(p_UpdateInformation.data(), Qt::QueuedConnection);
    getMethod(p_ControlFileParser, "clear(void)").invoke(p_ControlFileParser.data(), Qt::QueuedConnection);
    return;
}



void AppImageDeltaRevisionerPrivate::checkForUpdate(void)
{
    connect(p_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
            p_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)), Qt::UniqueConnection);
    connect(p_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
            p_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)), Qt::UniqueConnection);
    connect(p_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
            this, SLOT(handleUpdateCheckInformation(QJsonObject)), Qt::UniqueConnection);
    getAppImageEmbededInformation();
    return;
}

void AppImageDeltaRevisionerPrivate::handleIndeterminateProgress(int percentage)
{
    emit progress(percentage,
                  /*no bytes received*/0,
                  /*no bytes total*/0,
                  /*indeterminate speed*/0.0,
                  /*no speed units*/QString());
    return;
}

void AppImageDeltaRevisionerPrivate::handleUpdateCheckInformation(QJsonObject information)
{
    disconnect(p_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               p_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(p_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               p_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(p_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
               this, SLOT(handleUpdateCheckInformation(QJsonObject)));

    if(information.isEmpty()) {
        return;
    }

    auto embededUpdateInformation = information["EmbededUpdateInformation"].toObject();
    auto remoteTargetFileSHA1Hash = information["RemoteTargetFileSHA1Hash"].toString();
    QString localAppImageSHA1Hash = embededUpdateInformation["FileInformation"].toObject()["AppImageSHA1Hash"].toString(),
            localAppImagePath = embededUpdateInformation["FileInformation"].toObject()["AppImageFilePath"].toString();

    emit updateAvailable((localAppImageSHA1Hash != remoteTargetFileSHA1Hash),
                         embededUpdateInformation["FileInformation"].toObject());
    return;
}
