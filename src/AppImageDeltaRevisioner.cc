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
 * @filename    : AppImageDeltaRevisioner.hpp
 * @description : This where the delta revisioner is implemented.
 * Delta Revisioner is the public API to manage the entire revision
 * of a AppImage. From retrival of the embeded information from the
 * AppImage to the retrival of required remaining block ranges.
 * This is a controller to all the internal mechanisms , Handled neatly
 * via Qt's signals and slots.
 *
*/
#include <AppImageUpdateInformation_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <ZsyncWriter_p.hpp>
#include <ZsyncBlockRangeDownloader_p.hpp>
#include <AppImageDeltaRevisioner.hpp>

using namespace AppImageUpdaterBridge;

#define CONSTRUCT(x) setObjectName("AppImageDeltaRevisioner"); \
		     if(!singleThreaded){ \
		     _pSharedThread.reset(new QThread);\
		     _pSharedThread->start(); \
		     } \
		     _pSharedNetworkAccessManager.reset(new QNetworkAccessManager); \
		     _pUpdateInformation.reset(new AppImageUpdateInformationPrivate); \
		     _pDeltaWriter.reset(new ZsyncWriterPrivate); \
		     if(!singleThreaded){ \
		     _pSharedNetworkAccessManager->moveToThread(_pSharedThread.data()); \
		     _pUpdateInformation->moveToThread(_pSharedThread.data()); \
		     _pDeltaWriter->moveToThread(_pSharedThread.data()); \
		      } \
		     _pControlFileParser.reset(new ZsyncRemoteControlFileParserPrivate(_pSharedNetworkAccessManager.data())); \
		     _pBlockDownloader.reset(new ZsyncBlockRangeDownloaderPrivate(_pControlFileParser.data() ,  \
					                                         _pDeltaWriter.data() , \
					                                         _pSharedNetworkAccessManager.data())); \
		     if(!singleThreaded){ \
		     _pControlFileParser->moveToThread(_pSharedThread.data()); \
		     _pBlockDownloader->moveToThread(_pSharedThread.data()); \
		     } \
		     _pControlFileParser->setObjectName("ZsyncRemoteControlFileParserPrivate"); \
		     _pUpdateInformation->setObjectName("AppImageUpdateInformationPrivate"); \
		     _pDeltaWriter->setObjectName("ZsyncWriterPrivate"); \
                     _pBlockDownloader->setObjectName("ZsyncBlockRangeDownloaderPrivate"); \
		     _pUpdateInformation->setLoggerName("UpdateInformation"); \
		     _pControlFileParser->setLoggerName("ControlFileParser"); \
		     _pDeltaWriter->setLoggerName("AppImageDeltaRevisioner"); \
		     connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::statusChanged , \
			      this , &AppImageDeltaRevisioner::statusChanged , Qt::DirectConnection); \
		     connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::progress , \
			      this , &AppImageDeltaRevisioner::handleIndeterminateProgress); \
		     connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::info , \
			      this , &AppImageDeltaRevisioner::embededInformation , Qt::DirectConnection); \
	             connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::error , \
			      this , &AppImageDeltaRevisioner::error , Qt::DirectConnection); \
	             connect(_pUpdateInformation.data() , &AppImageUpdateInformationPrivate::logger , \
			      this , &AppImageDeltaRevisioner::logger , Qt::DirectConnection); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::statusChanged ,\
			      this , &AppImageDeltaRevisioner::statusChanged , Qt::DirectConnection); \
                     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::progress , \
			      this , &AppImageDeltaRevisioner::handleIndeterminateProgress); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::error, \
			      this , &AppImageDeltaRevisioner::handleZsyncRemoteControlFileParserError); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::networkError, \
			      this , &AppImageDeltaRevisioner::handleNetworkError); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::logger , \
			      this , &AppImageDeltaRevisioner::logger , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::statusChanged , \
			      this , &AppImageDeltaRevisioner::statusChanged , Qt::DirectConnection); \
                     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::error , \
			      this , &AppImageDeltaRevisioner::error , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::logger , \
			      this , &AppImageDeltaRevisioner::logger , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::started, \
			      this , &AppImageDeltaRevisioner::started , Qt::DirectConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::canceled , \
			      this , &AppImageDeltaRevisioner::canceled , Qt::DirectConnection); \
                     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::finished , \
			      this , &AppImageDeltaRevisioner::finished , Qt::DirectConnection); \
                     connect(_pBlockDownloader.data() , &ZsyncBlockRangeDownloaderPrivate::error , \
			      this,  &AppImageDeltaRevisioner::handleNetworkError); \
		     connect(_pBlockDownloader.data() , &ZsyncBlockRangeDownloaderPrivate::canceled , \
			      this , &AppImageDeltaRevisioner::canceled , Qt::DirectConnection); \
                     connect(_pBlockDownloader.data() , &ZsyncBlockRangeDownloaderPrivate::progress , \
			      this , &AppImageDeltaRevisioner::progress , Qt::DirectConnection); \
                     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::progress , \
			      this , &AppImageDeltaRevisioner::progress , Qt::DirectConnection); \
		     connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::zsyncInformation, \
			     _pDeltaWriter.data() , &ZsyncWriterPrivate::setConfiguration , Qt::QueuedConnection); \
		     connect(_pDeltaWriter.data() , &ZsyncWriterPrivate::finishedConfiguring , \
			     _pDeltaWriter.data() , &ZsyncWriterPrivate::start , Qt::QueuedConnection); \
		     setAppImage(x);



AppImageDeltaRevisioner::AppImageDeltaRevisioner(bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    CONSTRUCT(nullptr);
    return;
}

AppImageDeltaRevisioner::AppImageDeltaRevisioner(const QString &AppImagePath, bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    CONSTRUCT(AppImagePath);
    return;
}

AppImageDeltaRevisioner::AppImageDeltaRevisioner(QFile *AppImage, bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    CONSTRUCT(AppImage);
    return;
}

AppImageDeltaRevisioner::~AppImageDeltaRevisioner()
{
    if(!_pSharedThread.isNull()) {
        _pSharedThread->quit();
        _pSharedThread->wait();
    }
    return;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::start(void)
{
    connect(this, &AppImageDeltaRevisioner::updateAvailable, this, &AppImageDeltaRevisioner::handleUpdateAvailable);
    checkForUpdate();
    return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::cancel(void)
{
    {
        auto metaObject = _pDeltaWriter->metaObject();
        metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("cancel(void)")))
        .invoke(_pDeltaWriter.data(), Qt::QueuedConnection);
    }
    {
        auto metaObject = _pBlockDownloader->metaObject();
        metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("cancel(void)")))
        .invoke(_pBlockDownloader.data(), Qt::QueuedConnection);
    }
    return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::setAppImage(const QString &AppImagePath)
{
    clear();
    auto metaObject = _pUpdateInformation->metaObject();
    metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(const QString&)")))
    .invoke(_pUpdateInformation.data(), Qt::QueuedConnection, Q_ARG(QString, AppImagePath));
    return *this;
}
AppImageDeltaRevisioner &AppImageDeltaRevisioner::setAppImage(QFile *AppImage)
{
    if(!AppImage) {
        return *this;
    }
    clear();
    auto metaObject = _pUpdateInformation->metaObject();
    metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setAppImage(QFile*)")))
    .invoke(_pUpdateInformation.data(), Qt::QueuedConnection, Q_ARG(QFile*, AppImage));
    return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::setShowLog(bool choice)
{
    {
        auto metaObject = _pUpdateInformation->metaObject();
        metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
        .invoke(_pUpdateInformation.data(), Qt::QueuedConnection, Q_ARG(bool, choice));
    }
    {
        auto metaObject = _pControlFileParser->metaObject();
        metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
        .invoke(_pControlFileParser.data(), Qt::QueuedConnection, Q_ARG(bool, choice));

    }
    {
        auto metaObject = _pDeltaWriter->metaObject();
        metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setShowLog(bool)")))
        .invoke(_pDeltaWriter.data(), Qt::QueuedConnection, Q_ARG(bool, choice));

    }
    return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::setOutputDirectory(const QString &dir)
{
    if(dir.isEmpty()) {
        return *this;
    }
    QFileInfo info(dir);
    if(!info.isDir() || !info.isWritable() || !info.isReadable() || !info.exists()) {
        return *this;
    }
    auto metaObject = _pDeltaWriter->metaObject();
    metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("setOutputDirectory(const QString&)")))
    .invoke(_pDeltaWriter.data(), Qt::QueuedConnection, Q_ARG(QString, info.absoluteFilePath()));
    return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::getAppImageEmbededInformation(void)
{
    auto metaObject = _pUpdateInformation->metaObject();
    metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("getInfo(void)")))
    .invoke(_pUpdateInformation.data(), Qt::QueuedConnection);
    return *this;
}

AppImageDeltaRevisioner &AppImageDeltaRevisioner::clear(void)
{
    {
        auto metaObject = _pUpdateInformation->metaObject();
        metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
        .invoke(_pUpdateInformation.data(), Qt::QueuedConnection);
    }
    {
        auto metaObject = _pControlFileParser->metaObject();
        metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature("clear(void)")))
        .invoke(_pControlFileParser.data(), Qt::QueuedConnection);
    }
    _pRecentNetworkErrorCode = QNetworkReply::NoError;
    return *this;
}



AppImageDeltaRevisioner &AppImageDeltaRevisioner::checkForUpdate(void)
{
    connect(_pUpdateInformation.data(), SIGNAL(info(QJsonObject)),
            _pControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    connect(_pControlFileParser.data(), SIGNAL(receiveControlFile(void)),
            _pControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    connect(_pControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
            this, SLOT(handleUpdateCheckInformation(QJsonObject)));
    getAppImageEmbededInformation();
    return *this;
}

QNetworkReply::NetworkError AppImageDeltaRevisioner::getNetworkError(void)
{
    return (QNetworkReply::NetworkError) _pRecentNetworkErrorCode.load();
}

void AppImageDeltaRevisioner::handleZsyncRemoteControlFileParserError(short ecode)
{
    if(ecode == UNKNOWN_NETWORK_ERROR) {
        return;
    }
    emit error(ecode);
    return;
}

void AppImageDeltaRevisioner::handleNetworkError(QNetworkReply::NetworkError ecode)
{
    _pRecentNetworkErrorCode = ecode;
    emit error(UNKNOWN_NETWORK_ERROR);
    return;
}

void AppImageDeltaRevisioner::handleIndeterminateProgress(int percentage)
{
    emit progress(percentage,
                  /*no bytes received*/0,
                  /*no bytes total*/0,
                  /*indeterminate speed*/0.0,
                  /*no speed units*/QString());
    return;
}

void AppImageDeltaRevisioner::handleUpdateAvailable(bool updateAvailable, QJsonObject oldVersionInformation)
{
    disconnect(this, &AppImageDeltaRevisioner::updateAvailable, this, &AppImageDeltaRevisioner::handleUpdateAvailable);
    if(updateAvailable) {
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

void AppImageDeltaRevisioner::handleUpdateCheckInformation(QJsonObject information)
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

QString AppImageDeltaRevisioner::errorCodeToString(short code)
{
    return (code == CANNOT_CONSTRUCT_TARGET_FILE) ? QString("AppImageDeltaRevisioner::errorCode(CANNOT_CONSTRUCT_TARGET_FILE)") :
           (code < 50) ? AppImageUpdateInformationPrivate::errorCodeToString(code) :
           (code < 100) ? ZsyncRemoteControlFileParserPrivate::errorCodeToString(code) :
           ZsyncWriterPrivate::errorCodeToString(code);
}

QString AppImageDeltaRevisioner::statusCodeToString(short code)
{
    return (code < 50) ? AppImageUpdateInformationPrivate::statusCodeToString(code) :
           (code < 100) ? ZsyncRemoteControlFileParserPrivate::statusCodeToString(code) :
           ZsyncWriterPrivate::statusCodeToString(code);
}
