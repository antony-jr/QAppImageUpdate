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
 * @filename    : AppImageDeltaRevisioner_p.cc
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
        _pSharedThread.reset(new QThread);
        _pSharedThread->start();
    }
    _pSharedNetworkAccessManager.reset(new QNetworkAccessManager);
    _pUpdateInformation.reset(new AppImageUpdateInformationPrivate);
    _pDeltaWriter.reset(new ZsyncWriterPrivate);
    if(!singleThreaded) {
        _pSharedNetworkAccessManager->moveToThread(_pSharedThread.data());
        _pUpdateInformation->moveToThread(_pSharedThread.data());
        _pDeltaWriter->moveToThread(_pSharedThread.data());
    }
    _pControlFileParser.reset(new ZsyncRemoteControlFileParserPrivate(_pSharedNetworkAccessManager.data()));
    _pBlockDownloader.reset(new ZsyncBlockRangeDownloaderPrivate(_pDeltaWriter.data(),
                                                                 _pSharedNetworkAccessManager.data()));
    if(!singleThreaded) {
        _pControlFileParser->moveToThread(_pSharedThread.data());
        _pBlockDownloader->moveToThread(_pSharedThread.data());
    }
    _pControlFileParser->setObjectName("ZsyncRemoteControlFileParserPrivate");
    _pUpdateInformation->setObjectName("AppImageUpdateInformationPrivate");
    _pDeltaWriter->setObjectName("ZsyncWriterPrivate");
    _pBlockDownloader->setObjectName("ZsyncBlockRangeDownloaderPrivate");
    _pUpdateInformation->setLoggerName("UpdateInformation");
    _pControlFileParser->setLoggerName("ControlFileParser");
    _pDeltaWriter->setLoggerName("AppImageDeltaRevisioner");
    connect(_pUpdateInformation.data(), &AppImageUpdateInformationPrivate::statusChanged,
            this, &AppImageDeltaRevisionerPrivate::statusChanged, Qt::DirectConnection);
    connect(_pUpdateInformation.data(), &AppImageUpdateInformationPrivate::progress,
            this, &AppImageDeltaRevisionerPrivate::handleIndeterminateProgress);
    connect(_pUpdateInformation.data(), &AppImageUpdateInformationPrivate::info,
            this, &AppImageDeltaRevisionerPrivate::embededInformation, Qt::DirectConnection);
    connect(_pUpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
            this, &AppImageDeltaRevisionerPrivate::error, Qt::DirectConnection);
    connect(_pUpdateInformation.data(), &AppImageUpdateInformationPrivate::logger,
            this, &AppImageDeltaRevisionerPrivate::logger, Qt::DirectConnection);
    connect(_pControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::statusChanged,
            this, &AppImageDeltaRevisionerPrivate::statusChanged, Qt::DirectConnection);
    connect(_pControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::progress,
            this, &AppImageDeltaRevisionerPrivate::handleIndeterminateProgress);
    connect(_pControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
            this, &AppImageDeltaRevisionerPrivate::error , Qt::DirectConnection);
    connect(_pControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::logger,
            this, &AppImageDeltaRevisionerPrivate::logger, Qt::DirectConnection);
    connect(_pDeltaWriter.data(), &ZsyncWriterPrivate::statusChanged,
            this, &AppImageDeltaRevisionerPrivate::statusChanged, Qt::DirectConnection);
    connect(_pDeltaWriter.data(), &ZsyncWriterPrivate::error,
            this, &AppImageDeltaRevisionerPrivate::error, Qt::DirectConnection);
    connect(_pDeltaWriter.data(), &ZsyncWriterPrivate::logger,
            this, &AppImageDeltaRevisionerPrivate::logger, Qt::DirectConnection);
    connect(_pDeltaWriter.data(), &ZsyncWriterPrivate::started,
            this, &AppImageDeltaRevisionerPrivate::started, Qt::DirectConnection);
    connect(_pDeltaWriter.data(), &ZsyncWriterPrivate::canceled,
            this, &AppImageDeltaRevisionerPrivate::canceled, Qt::DirectConnection);
    connect(_pDeltaWriter.data(), &ZsyncWriterPrivate::finished,
            this, &AppImageDeltaRevisionerPrivate::finished, Qt::DirectConnection);
    connect(_pBlockDownloader.data(), &ZsyncBlockRangeDownloaderPrivate::error,
            this,  &AppImageDeltaRevisionerPrivate::handleNetworkError);
    connect(_pBlockDownloader.data(), &ZsyncBlockRangeDownloaderPrivate::canceled,
            this, &AppImageDeltaRevisionerPrivate::canceled, Qt::DirectConnection);
    connect(_pBlockDownloader.data(), &ZsyncBlockRangeDownloaderPrivate::progress,
            this, &AppImageDeltaRevisionerPrivate::progress, Qt::DirectConnection);
    connect(_pDeltaWriter.data(), &ZsyncWriterPrivate::progress,
            this, &AppImageDeltaRevisionerPrivate::progress, Qt::DirectConnection);
    connect(_pControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
            _pDeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration, Qt::QueuedConnection);
    connect(_pDeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
            _pDeltaWriter.data(), &ZsyncWriterPrivate::start, Qt::QueuedConnection);
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
    if(!_pSharedThread.isNull()) {
        _pSharedThread->quit();
        _pSharedThread->wait();
    }
    return;
}

void AppImageDeltaRevisionerPrivate::start(void)
{
    connect(_pUpdateInformation.data(), SIGNAL(info(QJsonObject)),
            _pControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)), Qt::UniqueConnection);
    connect(_pControlFileParser.data(), SIGNAL(receiveControlFile(void)),
            _pControlFileParser.data(), SLOT(getUpdateCheckInformation(void)), Qt::UniqueConnection);
    connect(_pControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
            this, SLOT(doStart(QJsonObject)), Qt::UniqueConnection);
    disconnect(_pUpdateInformation.data(), &AppImageUpdateInformationPrivate::info,
               this, &AppImageDeltaRevisionerPrivate::embededInformation);
    getAppImageEmbededInformation();
    return;
}

void AppImageDeltaRevisionerPrivate::doStart(QJsonObject information)
{
    disconnect(_pUpdateInformation.data(), SIGNAL(info(QJsonObject)),
               _pControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(_pControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               _pControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(_pControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
               this, SLOT(doStart(QJsonObject)));
    connect(_pUpdateInformation.data(), &AppImageUpdateInformationPrivate::info,
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
        auto metaObject = _pControlFileParser->metaObject();
        metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getZsyncInformation(void)")))
        .invoke(_pControlFileParser.data(), Qt::QueuedConnection);
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
    getMethod(_pDeltaWriter,"cancel(void)").invoke(_pDeltaWriter.data() , Qt::QueuedConnection);	    
    getMethod(_pBlockDownloader , "cancel(void)").invoke(_pBlockDownloader.data() , Qt::QueuedConnection);
    return;
}

void AppImageDeltaRevisionerPrivate::setAppImage(const QString &AppImagePath)
{
    getMethod(_pUpdateInformation , "setAppImage(const QString&)").invoke(_pUpdateInformation.data() ,
		    							  Qt::QueuedConnection , 
									  Q_ARG(QString,AppImagePath)); 
    return;
}

void AppImageDeltaRevisionerPrivate::setAppImage(QFile *AppImage)
{
    getMethod(_pUpdateInformation , "setAppImage(QFile *)").invoke(_pUpdateInformation.data() ,
		    					          Qt::QueuedConnection , 
								  Q_ARG(QFile*,AppImage)); 
    return;
}

void AppImageDeltaRevisionerPrivate::setShowLog(bool choice)
{
    getMethod(_pUpdateInformation , "setShowLog(bool)").invoke(_pUpdateInformation.data() ,
		                                               Qt::QueuedConnection , Q_ARG(bool , choice));
    getMethod(_pControlFileParser , "setShowLog(bool)").invoke(_pControlFileParser.data() ,
		    					       Qt::QueuedConnection , Q_ARG(bool , choice));
    getMethod(_pDeltaWriter , "setShowLog(bool)").invoke(_pDeltaWriter.data(), 
		                                         Qt::QueuedConnection, Q_ARG(bool, choice));
    return;
}

void AppImageDeltaRevisionerPrivate::setOutputDirectory(const QString &dir)
{
    getMethod(_pDeltaWriter , "setOutputDirectory(const QString&)").invoke(_pDeltaWriter.data(), 
		                                                           Qt::QueuedConnection, 
									   Q_ARG(QString, dir));
    return;
}

void AppImageDeltaRevisionerPrivate::getAppImageEmbededInformation(void)
{
    getMethod(_pUpdateInformation , "getInfo(void)").invoke(_pUpdateInformation.data(), Qt::QueuedConnection);
    return;
}

void AppImageDeltaRevisionerPrivate::clear(void)
{
    getMethod(_pUpdateInformation , "clear(void)").invoke(_pUpdateInformation.data(), Qt::QueuedConnection);
    getMethod(_pControlFileParser , "clear(void)").invoke(_pControlFileParser.data(), Qt::QueuedConnection);
    return;
}



void AppImageDeltaRevisionerPrivate::checkForUpdate(void)
{
    connect(_pUpdateInformation.data(), SIGNAL(info(QJsonObject)),
            _pControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)), Qt::UniqueConnection);
    connect(_pControlFileParser.data(), SIGNAL(receiveControlFile(void)),
            _pControlFileParser.data(), SLOT(getUpdateCheckInformation(void)), Qt::UniqueConnection);
    connect(_pControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
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
    disconnect(_pUpdateInformation.data(), SIGNAL(info(QJsonObject)),
               _pControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(_pControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               _pControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(_pControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
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
