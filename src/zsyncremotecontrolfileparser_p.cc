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
 * @filename    : zsyncremotecontrolfileparser_p.cc
 * @description : This is where the zsync control file parser in written.
 * The Zsync Control File Parser is responsible to parse the remote
 * zsync control file and produce us with a more sensible data to work with.
 * This also produces information for ZsyncWriterPrivate.
*/
#include <QFileInfo>

#include "zsyncremotecontrolfileparser_p.hpp"
#include "qappimageupdateenums.hpp"
#include "helpers_p.hpp"


/*
 * An efficient logging system.
 * Warning:
 * 	Hard coded to work only in this source file.
*/
#ifndef LOGGING_DISABLED
#define LOGS *(p_Logger.data()) LOGR
#define LOGR <<
#define LOGE ; \
             emit(logger(s_LogBuffer , s_AppImagePath)); \
             s_LogBuffer.clear();
#else
#define LOGS (void)
#define LOGR ;(void)
#define LOGE ;
#endif // LOGGING_DISABLED
#define INFO_START LOGS "   INFO: "
#define INFO_END LOGE

#define WARNING_START LOGS "WARNING: "
#define WARNING_END LOGE

#define FATAL_START LOGS "  FATAL: "
#define FATAL_END LOGE


/*
 * Splits a source QString(src) at the given delimiter(key) and sets the second
 * part of the split to the given destination QString(dest)
 *
 * If something unexpected happens , the given error(e) is emitted where an
 * error signal is assumed.
 *
 * Warning:
 * 	Hard coded to only work with this source file.
*/
#define STORE_SPLIT(dest , src , key , e) { \
					  auto s = src.split(key); \
					  if(s.size() < 2){ \
					     emit error(e); \
					     return; \
					  } \
					  dest = s[1]; \
					  }



/*
 * ZsyncRemoteControlFileParserPrivate is the private class to handle all things
 * related to Zsync Control File. This class must be used privately.
 * This class caches the Zsync Control File in a QBuffer , So when its needed ,We
 * can quickly seek and start feeding in the checksum blocks.
 * This class automatically parses the zsync headers on the way to buffer the
 * control file.
 *
 * Example:
 *
 *      QNetworkAccessManager qnam;
 *      ZsyncRemoteControlFileParserPrivate rcfp(&qnam);
 *
 * Note:
 *    This class will not use the QNetworkAccessManager as the parent ,
 *    Therefore the developer as to deallocate the object if he allocates it.
 *    That is , Qt parent to child deallocation does not work in this class
 *    for a reason(i.e , This class is intended to run in a QThread.).
 *
*/
ZsyncRemoteControlFileParserPrivate::ZsyncRemoteControlFileParserPrivate(QNetworkAccessManager *networkManager)
    : QObject() {
#ifndef LOGGING_DISABLED
    p_Logger.reset(new QDebug(&s_LogBuffer));
#endif // LOGGING_DISABLED
    p_NManager = networkManager;
    connect(this, SIGNAL(error(short)), this, SLOT(handleErrorSignal(short)));
    return;
}

ZsyncRemoteControlFileParserPrivate::~ZsyncRemoteControlFileParserPrivate() {
    return;
}

/* Sets the name of the logger. */
void ZsyncRemoteControlFileParserPrivate::setLoggerName(const QString &name) {
#ifndef LOGGING_DISABLED
    s_LoggerName = QString(name);
#else
    (void)name;
#endif
    return;
}

/* This public method safely turns on and off the internal logger. */
void ZsyncRemoteControlFileParserPrivate::setShowLog(bool choose) {
#ifndef LOGGING_DISABLED
    if(choose) {
        connect(this, SIGNAL(logger(QString, QString)),
                this, SLOT(handleLogMessage(QString, QString)), Qt::UniqueConnection);
        return;
    }
    disconnect(this, SIGNAL(logger(QString, QString)),
               this, SLOT(handleLogMessage(QString, QString)));
#else
    (void)choose;
#endif
    return;
}

/* This public method safely sets the zsync control file url. */
void ZsyncRemoteControlFileParserPrivate::setControlFileUrl(const QUrl &controlFileUrl) {
    INFO_START LOGR " setControlFileUrl : using " LOGR controlFileUrl LOGR " as zsync control file." INFO_END;
    u_ControlFileUrl = controlFileUrl;
    return;
}

/*
 * This public overloaded method will use the embeded information in AppImages to get
 * and parse a remote zsync control file.
 * This automatically calls getControlFile() slot.
*/
void ZsyncRemoteControlFileParserPrivate::setControlFileUrl(QJsonObject information) {
    if(information["IsEmpty"].toBool()) {
        return;
    }
    
    /*
     * Check if we are given the same information consecutively , If so then return
     * what we know. */
    if(!j_UpdateInformation.isEmpty()) {
        if(j_UpdateInformation == information) {
            emit receiveControlFile();
            return;
        }
    }

    {
        j_UpdateInformation = information;
        auto fileInfo = information["FileInformation"].toObject();
        s_AppImagePath = fileInfo["AppImageFilePath"].toString();
    }

    information = information["UpdateInformation"].toObject();
    if(information["transport"].toString() == "zsync" ) {
        INFO_START " setControlFileUrl : using direct zsync transport." INFO_END;
        setControlFileUrl(QUrl(information["zsyncUrl"].toString()));

	getControlFile();
    } else if(information["transport"].toString() == "gh-releases-zsync") {
        INFO_START " setControlFileUrl : using github releases zsync transport." INFO_END;
        QUrl apiLink = QUrl("https://api.github.com/repos/" + information["username"].toString() +
                            "/"  + information["repo"].toString() + "/releases/");
        if(information["tag"].toString() == "latest") {
            apiLink = QUrl(apiLink.toString() + information["tag"].toString());
        } else {
            apiLink = QUrl(apiLink.toString() + "tags/" + information["tag"].toString());
        }

        s_ZsyncFileName = information["filename"].toString();

        QNetworkRequest request;
        request.setUrl(apiLink);
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

        INFO_START " setControlFileUrl : github api request(" LOGR apiLink LOGR ")." INFO_END;

        auto reply = p_NManager->get(request);

        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(handleNetworkError(QNetworkReply::NetworkError)), Qt::UniqueConnection);
        connect(reply, SIGNAL(finished(void)),
                this, SLOT(handleGithubAPIResponse(void)), Qt::UniqueConnection);
    } else {
        /*
         * if its not github releases zsync or generic zsync then it must be bintray-zsync.
         * Note:
         * 	Since AppImageUpdateInformation can handle errors , Thus we don't really
         * 	have to check for integrity now.
         */
        INFO_START " setControlFileUrl : using bintray zsync transport." INFO_END;
        QUrl latestLink;
        latestLink = QUrl("https://bintray.com/" + information["username"].toString() +
                          "/" + information["repo"].toString() + "/" + information["packageName"].toString() + "/_latestVersion");

        s_ZsyncFileName = information["filename"].toString();

        QNetworkRequest request;
        request.setUrl(latestLink);
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

        QNetworkReply *reply = p_NManager->head(request);

        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(handleNetworkError(QNetworkReply::NetworkError)), Qt::UniqueConnection);
        connect(reply, SIGNAL(redirected(const QUrl&)),
                this, SLOT(handleBintrayRedirection(const QUrl&)), Qt::UniqueConnection);
    }
    return;
}

/* clears all internal cache in the class. */
void ZsyncRemoteControlFileParserPrivate::clear(void) {
    b_AcceptRange = false;
    j_UpdateInformation = QJsonObject();
    s_ZsyncMakeVersion.clear();
    s_TargetFileName.clear();
    s_TargetFileSHA1.clear();
    s_ZsyncFileName.clear();
    s_ReleaseNotes.clear();
#ifndef LOGGING_DISABLED
    s_LogBuffer.clear();
#endif // LOGGING_DISABLED
    m_MTime = QDateTime();
    n_TargetFileBlockSize = n_TargetFileLength = n_TargetFileBlocks = n_WeakCheckSumBytes = 0;
    n_StrongCheckSumBytes = n_ConsecutiveMatchNeeded = n_CheckSumBlocksOffset = 0;
    u_TargetFileUrl.clear();
    u_ControlFileUrl.clear();
    u_TorrentFile.clear();
    p_ControlFile.reset(nullptr);
    return;
}

/* Starts an async request to the given zsync control file. */
void ZsyncRemoteControlFileParserPrivate::getControlFile(void) {
    if(u_ControlFileUrl.isEmpty() || !u_ControlFileUrl.isValid()) {
        WARNING_START LOGR " getControlFile : no zsync control file url(" LOGR u_ControlFileUrl LOGR ") is given or valid." WARNING_END;
        return;
    }

    INFO_START LOGR " getControlFile : sending get request to " LOGR u_ControlFileUrl LOGR "." INFO_END;

    QNetworkRequest request;
    request.setUrl(u_ControlFileUrl);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    auto reply = p_NManager->get(request);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished(void)), this, SLOT(handleControlFile(void)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(handleDownloadProgress(qint64, qint64)));
    return;
}

/* Replies with a signal which has a QJsonObject which can be helpful
 * to check if the given AppImage is a different version from the
 * remote version.
 * Useless if the parser is not working with AppImageUpdateInformationPrivate.
*/
void ZsyncRemoteControlFileParserPrivate::getUpdateCheckInformation(void) {
    QJsonObject result {
        { "EmbededUpdateInformation", j_UpdateInformation},
        { "RemoteTargetFileSHA1Hash", s_TargetFileSHA1 },
        { "ReleaseNotes", s_ReleaseNotes }
    };

    emit updateCheckInformation(result);
    return;
}

/*
 * This signals information needed to configure ZsyncWriterPrivate
 * class which is the main implementation of the zsync algorithm.
*/
void ZsyncRemoteControlFileParserPrivate::getZsyncInformation(void) {
    if(!p_ControlFile ||
            !p_ControlFile->isOpen() ||
            /* Atleast one block is needed to do anything. */
            p_ControlFile->size() - n_CheckSumBlocksOffset < (n_WeakCheckSumBytes + n_StrongCheckSumBytes) ||
            !n_CheckSumBlocksOffset) {
        emit error(QAppImageUpdateEnums::Error::IoReadError);
        return;
    }

    auto buffer = new QBuffer;
    QString SeedFilePath = (j_UpdateInformation["FileInformation"].toObject())["AppImageFilePath"].toString();
    buffer->open(QIODevice::WriteOnly);
    p_ControlFile->seek(n_CheckSumBlocksOffset); /* Seek to the offset of the checksum block. */

    while(!p_ControlFile->atEnd()) {
        buffer->write(p_ControlFile->read((n_WeakCheckSumBytes + n_StrongCheckSumBytes) * 16));
        QCoreApplication::processEvents();
    }
    buffer->close();
    /* leave the buffer ownership to the one who called it. */
    emit zsyncInformation(n_TargetFileBlockSize, n_TargetFileBlocks, n_WeakCheckSumBytes, n_StrongCheckSumBytes,
                          n_ConsecutiveMatchNeeded, n_TargetFileLength, SeedFilePath, s_TargetFileName,
                          s_TargetFileSHA1, u_TargetFileUrl, buffer, b_AcceptRange, u_TorrentFile);
    return;
}

/*
 * Calculates the download progress of the control file in percentage
 * and emits it.
*/
void ZsyncRemoteControlFileParserPrivate::handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    int nPercentage =
        static_cast<int>(
            (static_cast<float>
             (
                 bytesReceived
             ) * 100.0
            ) / static_cast<float>
            (
                bytesTotal
            )
        );
    emit progress(nPercentage);
    return;
}

/* This private slot parses the latest bintray package url. */
void ZsyncRemoteControlFileParserPrivate::handleBintrayRedirection(const QUrl &url) {
    INFO_START LOGR " handleBintrayRedirection : start to parse latest package url." INFO_END;
    QNetworkReply *senderReply = qobject_cast<QNetworkReply*>(QObject::sender());
    if(!senderReply)
        return;

    if(senderReply->error() != QNetworkReply::NoError) {
        senderReply->abort();
        senderReply->deleteLater();
        return;
    }

    int responseCode = senderReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    INFO_START LOGR " handleBintrayRedirection : http response code(" LOGR responseCode LOGR ")." INFO_END;
    
    /* cut all ties. */
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, &QNetworkReply::redirected, this, &ZsyncRemoteControlFileParserPrivate::handleBintrayRedirection);

    senderReply->abort();
    senderReply->deleteLater();

    INFO_START LOGR " handleBintrayRedirection : redirected url(" LOGR url LOGR ")." INFO_END;

    QStringList information = url.toString().split("/");
    s_ZsyncFileName.replace("_latestVersion", url.fileName());
    setControlFileUrl(QUrl("https://dl.bintray.com/" + information[3] + "/" + information[4] + "/" + s_ZsyncFileName));
    getControlFile();
    return;
}

void ZsyncRemoteControlFileParserPrivate::handleGithubMarkdownParsed(void) {
    INFO_START LOGR " handleGithubMarkdownParsed : starting to parse github api response." INFO_END;
    QNetworkReply *senderReply = qobject_cast<QNetworkReply*>(QObject::sender());
    if(!senderReply)
        return;

    if(senderReply->error() != QNetworkReply::NoError){
	    senderReply->deleteLater();
	    return;
    }

    int responseCode = senderReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    INFO_START LOGR " handleGithubMarkdownParsed : http response code(" LOGR responseCode LOGR ")." INFO_END;

    /* Cut all ties. */
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, &QNetworkReply::finished,
               this,
               &ZsyncRemoteControlFileParserPrivate::handleGithubMarkdownParsed);
    QByteArray html = senderReply->readAll();
    s_ReleaseNotes = QString::fromLatin1(html);
    senderReply->deleteLater();
    getControlFile(); // start the control file parsing now.
    return;
}

/* This private slot parses the github api response. */
void ZsyncRemoteControlFileParserPrivate::handleGithubAPIResponse(void) {
    INFO_START LOGR " handleGithubAPIResponse : starting to parse github api response." INFO_END;
    QNetworkReply *senderReply = qobject_cast<QNetworkReply*>(QObject::sender());
    if(!senderReply)
        return;

    if(senderReply->error() != QNetworkReply::NoError){
	    senderReply->deleteLater();
	    return;
    }

    int responseCode = senderReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    INFO_START LOGR " handleGithubAPIResponse : http response code(" LOGR responseCode LOGR ")." INFO_END;
    
    /* Cut all ties. */
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, SIGNAL(finished(void)), this, SLOT(handleGithubAPIResponse(void)));

    QJsonDocument jsonResponse = QJsonDocument::fromJson(senderReply->readAll());
    senderReply->deleteLater();

    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray assetsArray = jsonObject["assets"].toArray();
    QString version = jsonObject["tag_name"].toString();
    QVector<QJsonObject> assets;

    /* Patern matching with wildcards. */
    QRegExp rx(s_ZsyncFileName);
    rx.setPatternSyntax(QRegExp::Wildcard);

    QString torrentFileName = QFileInfo(s_ZsyncFileName).completeBaseName();
    torrentFileName += QString::fromUtf8(".torrent");
    QRegExp rx_torrent(torrentFileName);
    rx_torrent.setPatternSyntax(QRegExp::Wildcard);

    QString requiredAssetUrl;


    INFO_START " handleGithubAPIResponse : latest version is " LOGR version INFO_END;
    INFO_START " handleGithubAPIResponse : asset required is " LOGR s_ZsyncFileName INFO_END;

    foreach (const QJsonValue &value, assetsArray) {
	    auto asset = value.toObject();
	    if(rx.exactMatch(asset["name"].toString())) {
            	requiredAssetUrl = asset["browser_download_url"].toString();
	    } 
	    if(rx_torrent.exactMatch(asset["name"].toString())) {
		u_TorrentFile = QUrl(asset["browser_download_url"].toString());
	    }
	    QCoreApplication::processEvents();
    }



    if(requiredAssetUrl.isEmpty()){
    	emit error(QAppImageUpdateEnums::Error::ZsyncControlFileNotFound);
    	return;
    }

      setControlFileUrl(QUrl(requiredAssetUrl));
            /* Convert Github flavored Markdown to HTML using their own API.
            * And then call getControlFile(); */
            QNetworkRequest request;
            request.setUrl(QString::fromUtf8("https://api.github.com/markdown/raw"));
            request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
            request.setRawHeader("Content-Type", "text/plain");

            QByteArray md = (jsonObject["body"].toString()).toLocal8Bit();
            QNetworkReply *reply = p_NManager->post(request, md);
            connect(reply,
                    SIGNAL(error(QNetworkReply::NetworkError)),
                    this,
                    SLOT(handleNetworkError(QNetworkReply::NetworkError)),
                    Qt::UniqueConnection);
            connect(reply, &QNetworkReply::finished,
                    this,
                    &ZsyncRemoteControlFileParserPrivate::handleGithubMarkdownParsed,
                    Qt::UniqueConnection); 
    return;
}

/* This private slot parses the control file. */
void ZsyncRemoteControlFileParserPrivate::handleControlFile(void) {
    INFO_START LOGR " handleControlFile : starting to parse zsync control file." INFO_END;
    QNetworkReply *senderReply = qobject_cast<QNetworkReply*>(QObject::sender());
    if(!senderReply)
        return;

    if(senderReply->error() != QNetworkReply::NoError) {
	    senderReply->deleteLater();
	    return;
    }

    int responseCode = senderReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    INFO_START LOGR " handleControlFile : http response code(" LOGR responseCode LOGR ")." INFO_END;


    /* Check if the server supports Range requests.
     * Note:
     * 	Just this check cannot imply that the server does not support range requests.
     * 	So later we will do a dry run on a http range request to find out the truth.
    */
    b_AcceptRange = senderReply->hasRawHeader("Accept-Ranges");
    if(b_AcceptRange == false) {
        WARNING_START " handleControlFile : it seems that the remote server does not support range requests." WARNING_END;
    }

    /*
     * Disconnect all ties before casting it as QIODevice to read.
    */
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, SIGNAL(finished(void)), this, SLOT(handleControlFile(void)));
    disconnect(senderReply, SIGNAL(downloadProgress(qint64, qint64)),
               this, SLOT(handleDownloadProgress(qint64, qint64)));

    p_ControlFile.reset(new QBuffer);
    p_ControlFile->open(QIODevice::ReadWrite);

    /*
     * Since QNetworkReply is sequential QIODevice , We cannot seek but we
     * need to seek on command for future operation so we copy everything
     * to our newly allocated QIODevice and Also find the offset on the way.
     * Since this is sequential , We cannot use *->pos() to get the current position
     * of the file pointer , Therefore we will use a temporary variable.
     */
    {
        INFO_START LOGR " handleControlFile : searching for checksum blocks offset in the zsync control file." INFO_END;
        qint64 pos = 0;
        char previousMark = 0;
        int bufferSize = 1;
        while(!senderReply->atEnd()) {
            /*
             * Read one byte at a time , Since the marker for the
             * offset of the checksum blocks is \n\n.
             *
             * Therefore,
             *
             * ZsyncHeaders = (0 , offset - 2)
             * Checksums = (offset , EOF)
            */
            QByteArray data = senderReply->read(bufferSize);
            pos += bufferSize;
            if(bufferSize < 1024 && previousMark == 10 && data.at(0) == 10) {
                /*
                 * Set the offset and increase the buffer size
                 * to finish the rest of the copy faster.
                */
                INFO_START LOGR " handleControlFile : found checksum blocks offset(" LOGR pos LOGR ") in zsync control file." INFO_END;
                n_CheckSumBlocksOffset = pos;
                bufferSize = 1024; /* Use standard size of 1024 bytes. */
            } else {
                previousMark = data.at(0);
            }
            p_ControlFile->write(data);
        }
    }
    senderReply->deleteLater();

    p_ControlFile->seek(0); /* seek to the top again. */
    if(!n_CheckSumBlocksOffset) {
        /* error , we don't know the marker and therefore it must be an invalid control file.*/
        emit error(QAppImageUpdateEnums::Error::NoMarkerFoundInControlFile);
        return;
    }

    QString ZsyncHeader(p_ControlFile->read(n_CheckSumBlocksOffset - 2)); /* avoid reading the marker. */
    QStringList ZsyncHeaderList = QString(ZsyncHeader).split("\n");
    if(ZsyncHeaderList.size() < 8) {
        emit error(QAppImageUpdateEnums::Error::InvalidZsyncHeadersNumber);
        return;
    }

    STORE_SPLIT(s_ZsyncMakeVersion, ZsyncHeaderList.at(0), "zsync: ", QAppImageUpdateEnums::Error::InvalidZsyncMakeVersion);
    INFO_START LOGR " handleControlFile : zsync make version confirmed to be " LOGR s_ZsyncMakeVersion LOGR "." INFO_END;

    STORE_SPLIT(s_TargetFileName, ZsyncHeaderList.at(1), "Filename: ", QAppImageUpdateEnums::Error::InvalidZsyncTargetFilename);
    INFO_START LOGR " handleControlFile : zsync target file name confirmed to be " LOGR s_TargetFileName LOGR "." INFO_END;

    {
        QString timeStr;
        STORE_SPLIT(timeStr, ZsyncHeaderList.at(2), "MTime: ", QAppImageUpdateEnums::Error::InvalidZsyncMtime);
        QLocale locale(QLocale::English, QLocale::UnitedStates);
        m_MTime = locale.toDateTime(timeStr, "ddd, dd MMM yyyy HH:mm:ss +zzz0");
    }

    if(!m_MTime.isValid()) {
        emit error(QAppImageUpdateEnums::Error::InvalidZsyncMtime);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file MTime confirmed to be " LOGR m_MTime LOGR "." INFO_END;

    {
        QString nStr;
        STORE_SPLIT(nStr, ZsyncHeaderList.at(3), "Blocksize: ", QAppImageUpdateEnums::Error::InvalidZsyncBlocksize);
        n_TargetFileBlockSize = nStr.toInt();
    }
    if(n_TargetFileBlockSize < 1024) {
        emit error(QAppImageUpdateEnums::Error::InvalidZsyncBlocksize);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file blocksize confirmed to be " LOGR n_TargetFileBlockSize LOGR " bytes." INFO_END;

    {
        QString nStr;
        STORE_SPLIT(nStr, ZsyncHeaderList.at(4), "Length: ", QAppImageUpdateEnums::Error::InvalidTargetFileLength);
        n_TargetFileLength =  nStr.toInt();
    }
    if(n_TargetFileLength == 0) {
        emit error(QAppImageUpdateEnums::Error::InvalidTargetFileLength);
        return;
    }
    INFO_START LOGR " handleControlFile : zysnc target file length confirmed to be " LOGR n_TargetFileLength LOGR " bytes." INFO_END;


    {
        QString HashLength;
        STORE_SPLIT(HashLength, ZsyncHeaderList.at(5), "Hash-Lengths: ", QAppImageUpdateEnums::Error::InvalidHashLengthLine);
        QStringList HashLengths = HashLength.split(',');
        if(HashLengths.size() != 3) {
            emit error(QAppImageUpdateEnums::Error::InvalidHashLengthLine);
            return;
        }

        n_ConsecutiveMatchNeeded = HashLengths.at(0).toInt();
        n_WeakCheckSumBytes = HashLengths.at(1).toInt();
        n_StrongCheckSumBytes = HashLengths.at(2).toInt();
        if(n_WeakCheckSumBytes < 1 || n_WeakCheckSumBytes > 4
                || n_StrongCheckSumBytes < 3 || n_StrongCheckSumBytes > 16
                || n_ConsecutiveMatchNeeded > 2 || n_ConsecutiveMatchNeeded < 1) {
            emit error(QAppImageUpdateEnums::Error::InvalidHashLengths);
            return;
        }
    }

    INFO_START LOGR " handleControlFile : " LOGR n_WeakCheckSumBytes LOGR " bytes of weak checksum is available." INFO_END;
    INFO_START LOGR " handleControlFile : " LOGR n_StrongCheckSumBytes LOGR " bytes of strong checksum is available." INFO_END;
    INFO_START LOGR " handleControlFile : " LOGR n_ConsecutiveMatchNeeded LOGR " consecutive matches is needed." INFO_END;

    {
        QString uStr;
        STORE_SPLIT(uStr, ZsyncHeaderList.at(6), "URL: ", QAppImageUpdateEnums::Error::InvalidTargetFileUrl);
        u_TargetFileUrl = QUrl(uStr);
    }
    if(!u_TargetFileUrl.isValid()) {
        emit error(QAppImageUpdateEnums::Error::InvalidTargetFileUrl);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file url is confirmed to be " LOGR u_TargetFileUrl LOGR "." INFO_END;

    STORE_SPLIT(s_TargetFileSHA1, ZsyncHeaderList.at(7), "SHA-1: ", QAppImageUpdateEnums::Error::InvalidTargetFileSha1);
    s_TargetFileSHA1 = s_TargetFileSHA1.toUpper();
    INFO_START LOGR " handleControlFile : zsync target file sha1 hash is confirmed to be " LOGR s_TargetFileSHA1 LOGR "." INFO_END;

    n_TargetFileBlocks = (n_TargetFileLength + n_TargetFileBlockSize - 1) / n_TargetFileBlockSize;
    INFO_START LOGR " handleControlFile : zsync target file has " LOGR n_TargetFileBlocks LOGR " number of blocks." INFO_END;

    /*
     * Check if target file host server truly supports range requests.
     *
     * If the target file url is relative then we have to construct the url from the control
     * file url which is given by the developer.
     *
     * IMPORTANT: Do not resolve the target url here since the redirected url may expire
     * anytime, This could cause a force cache clear when all the data are valid.
     * The redirected url must be checked on the downloader itself to solve this problem.
     **/
    {
        u_TargetFileUrl  = (u_TargetFileUrl.isRelative()) ?
                           QUrl(u_ControlFileUrl.toString().replace(u_ControlFileUrl.fileName(), s_TargetFileName))
                           : u_TargetFileUrl;
        QNetworkRequest request;
        /* Even if the abort does'nt work if range is assumed to be supported then the request will not
         * spend too much data.
         **/
        QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(0) + "-";
        rangeHeaderValue += QByteArray::number(200); // Just get the first 200 Bytes of data.
        request.setUrl(u_TargetFileUrl);
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        request.setRawHeader("Range", rangeHeaderValue);
        auto reply = p_NManager->get(request);
        connect(reply, &QNetworkReply::downloadProgress,
                this, &ZsyncRemoteControlFileParserPrivate::checkHeadTargetFileUrl);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    }

    /*
     * When target url request is done control file received
     * signal will be emitted.
     **/
    return;
}

void ZsyncRemoteControlFileParserPrivate::checkHeadTargetFileUrl(qint64 bytesReceived, qint64 bytesTotal) {
    Q_UNUSED(bytesReceived);
    Q_UNUSED(bytesTotal);

    auto reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if(!reply) {
        WARNING_START "invalid pointer sent to checkHeadTargetFileUrl" WARNING_END;
        return;
    }

    disconnect(reply, &QNetworkReply::downloadProgress,
               this, &ZsyncRemoteControlFileParserPrivate::checkHeadTargetFileUrl);
  
    if(reply->error() != QNetworkReply::NoError) {
	    reply->abort();
	    reply->deleteLater();
	    return;
    }

    auto replyCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    /* Check if the server supports Range requests. */
    b_AcceptRange = reply->hasRawHeader("Accept-Ranges") || replyCode == 206/*HTTP Status code 206 => partial retrival*/;
    if(b_AcceptRange == false) {
        WARNING_START
        " handleControlFile : its confirmed that the remote server does not support range requests." WARNING_END;
    }
    reply->abort();
    reply->deleteLater();
    emit receiveControlFile();
    return;
}

void ZsyncRemoteControlFileParserPrivate::handleNetworkError(QNetworkReply::NetworkError errorCode) {
    QNetworkReply *senderReply = qobject_cast<QNetworkReply*>(QObject::sender());
    if(!senderReply)
        return;

    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this,SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, SIGNAL(finished(void)), this, SLOT(handleControlFile(void)));
    disconnect(senderReply, &QNetworkReply::downloadProgress,
               this, &ZsyncRemoteControlFileParserPrivate::checkHeadTargetFileUrl);
    disconnect(senderReply, SIGNAL(downloadProgress(qint64, qint64)),
               this, SLOT(handleDownloadProgress(qint64, qint64)));
    senderReply->deleteLater();

    if(errorCode == QNetworkReply::OperationCanceledError)
        return;

    FATAL_START LOGR " handleNetworkError : " LOGR errorCode LOGR "." FATAL_END;
    /* Translate QNetworkReply::NetworkError to Zsync Remote control file error. */
    emit error(translateQNetworkReplyError(errorCode));
    return;
}

/* This slot will be called anytime error signal is emitted. */
void ZsyncRemoteControlFileParserPrivate::handleErrorSignal(short errorCode) {
    FATAL_START LOGR " error : Code " LOGR errorCode LOGR " occured.";
    clear(); // clear all data to prevent later corrupted data collisions.
    return;
}

#ifndef LOGGING_DISABLED
void ZsyncRemoteControlFileParserPrivate::handleLogMessage(QString message, QString AppImageName) {
    qInfo().noquote() << "["
                      << QDateTime::currentDateTime().toString(Qt::ISODate)
                      << "] "
                      << s_LoggerName
                      << "("
                      << AppImageName
                      << "):: "
                      << message;
    return;
}
#endif // LOGGING_DISABLED
