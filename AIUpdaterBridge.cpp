/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2017, Antony jr
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
 * ------------------------------------------------------------------------------
 *  @filename           : AIUpdaterBridge.cpp
 *  @description        : This is the source file which fills in the code
 *  			  with reference to the prototype given in the
 *  			  AIUpdaterBridge.hpp.
 *  			  A simple bridge to AppImage's Updating Mechanism
 *  			  writen in C++ using Qt5. This small library helps
 *  			  you to create awesome AutoUpdater in Qt5 for AppImages.
 * ------------------------------------------------------------------------------
*/
#include <AIUpdaterBridge.hpp>

/*
 * Constructors
 * ------------
*/

AIUpdaterBridge::AIUpdaterBridge(const QString& appImage)
    : QObject(NULL)
{
    _pManager = new QNetworkAccessManager(this);
    _pManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    setAppImageUpdateInformation(appImage);
    return;
}

AIUpdaterBridge::AIUpdaterBridge(const QJsonObject& config)
    : QObject(NULL)
{
    _pManager = new QNetworkAccessManager(this);
    _pManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    setAppImageUpdateInformation(config);
    return;
}

/* --------------------------------- */

/*
 *  Public Functions
 * ------------------
*/
void AIUpdaterBridge::doDebug(bool ch)
{
    debug = ch;
    return;
}


void AIUpdaterBridge::setAppImageUpdateInformation(const QString& appImage)
{
    if(!mutex.tryLock()) {
        return;
    }

    clear();

    connect(&AppImageInformer, SIGNAL(updateInformation(const QString&, const QJsonObject&)),
            this, SLOT(handleAppImageUpdateInformation(const QString&, const QJsonObject&)));
    connect(&AppImageInformer, SIGNAL(error(const QString&, short)),
            this, SLOT(handleAppImageUpdateError(const QString&, short)));
    this->appImage = appImage; // do not need to check this because QAIUpdateInformation will
    // take care of it.
    AppImageInformer.setAppImage(appImage);
    AppImageInformer.doDebug(debug);
    AppImageInformer.start();
    return;
}

void AIUpdaterBridge::setAppImageUpdateInformation(const QJsonObject& config)
{
    // Since this is from the user the data can have a lot
    // of errors. Must check before use
    if(!mutex.tryLock()) {
        return;
    }

    clear();

    if(!config["appImagePath"].isString() || config["appImagePath"].isNull()) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: 'appImagePath' entry missing in the given json ::" << config;
        }
        mutex.unlock();
        emit error(QString("NONE"), APPIMAGE_PATH_NOT_GIVEN);
        return;
    } else {
        // set our appImage but check if it exists
        QFileInfo check_file(config["appImagePath"].toString());
        if (check_file.exists() && check_file.isFile()) {
            this->appImage = config["appImagePath"].toString();
        } else {
            if(debug) {
                qDebug() << "AIUpdaterBridge:: file not found :: " << config["appImagePath"].toString();
            }
            mutex.unlock();
            emit error(config["appImagePath"].toString(), APPIMAGE_NOT_FOUND);
            return;
        }
    }

    if(!config["transport"].isString() || config["transport"].isNull()) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: 'transport' entry missing in the given json ::" << config;
        }
        mutex.unlock();
        emit error(appImage, TRANSPORT_NOT_GIVEN);
        return;
    } else {
        if(config["transport"].toString() == "zsync") {
            if(!config["url"].isString() || config["url"].isNull()) {
                if(debug) {
                    qDebug() << "AIUpdaterBridge:: 'url' entry missing in the given json ::" << config;
                }
                mutex.unlock();
                emit error(appImage, URL_NOT_GIVEN);
                return;
            } else {
                this->zsyncURL = QUrl(config["url"].toString());
                if(debug) {
                    qDebug() << "AIUpdaterBridge:: zsyncURL ::" << zsyncURL << " :: " << appImage;
                }
                checkForUpdates();
            }
        } else if(config["transport"].toString() == "gh-releases-zsync") {
            // handle github releases zsync.
            if(
                (!config["username"].isString() || config["username"].isNull()) ||
                (!config["repo"].isString() || config["repo"].isNull()) ||
                (!config["tag"].isString() || config["tag"].isNull()) ||
                (!config["filename"].isString() || config["filename"].isNull())
            ) {
                if(debug) {
                    qDebug() << "AIUpdaterBridge:: invalid number of parameters ::" << config;
                }
                mutex.unlock();
                emit error(appImage, INVALID_UPD_INFO_PARAMENTERS);
                return;
            }
            QUrl releaseLink;
            releaseLink = QUrl("https://api.github.com/repos/" + config["username"].toString() +
                               "/"  + config["repo"].toString() + "/releases/");
            if(config["tag"].toString() == "latest") {
                releaseLink = QUrl(releaseLink.toString() + config["tag"].toString());
            } else {
                releaseLink = QUrl(releaseLink.toString() + "tags/" + config["tag"].toString());
            }
            if(debug) {
                qDebug() << "AIUpdaterBridge:: github release link ::" << releaseLink;
            }
            this->zsyncFileName = config["filename"].toString();
            getGitHubReleases(releaseLink);
        } else if(config["transport"] == "bintray-zsync") {
            // handle bintray zsync.
            if(
                (!config["username"].isString() || config["username"].isNull()) ||
                (!config["repo"].isString() || config["repo"].isNull()) ||
                (!config["packageName"].isString() || config["packageName"].isNull()) ||
                (!config["filename"].isString() || config["filename"].isNull())
            ) {
                if(debug) {
                    qDebug() << "AIUpdaterBridge:: invalid number of parameters ::" << config;
                }
                mutex.unlock();
                emit error(appImage, INVALID_UPD_INFO_PARAMENTERS);
                return;
            }
            QUrl latestLink;
            latestLink = QUrl("https://bintray.com/" + config["username"].toString() +
                              "/" + config["repo"].toString() + "/" + config["packageName"].toString() + "/_latestVersion");
            this->zsyncFileName = config["filename"].toString();
            getBintrayLatestPackage(latestLink);
            return;
        } else {
            // invalid transport given by the user
            if(debug) {
                qDebug() << "AIUpdaterBridge:: 'transport' entry invalid in the given json ::" << config["transport"].toString();
                // lets help the user a little bit.
                QString hint = config["transport"].toString();
                if(hint.contains("github")
                   || hint.contains("GITHUB")
                   || hint.contains("gh-zsync")
                   || hint.contains("gh")
                   || hint.contains("GH")) {
                    qDebug() << "AIUpdaterBridge:: did you mean 'gh-releases-zsync' ?";
                } else if(hint.contains("bintray")
                          || hint.contains("BINTRAY")
                         ) {
                    qDebug() << "AIUpdaterBridge:: did you mean 'bintray-zsync' ?";
                } else {
                    qDebug() << "AIUpdaterBridge:: valid transport mechanisms are:: 'gh-releases-zsync' , 'zsync' , 'bintray-zsync'";
                }
            }
            mutex.unlock();
            emit error(appImage, INVALID_TRANSPORT_GIVEN);
            return;
        }
    }
    return;
}


/* ------------------------------- */

/*
 * Public Slots
 * ------------
 * This is what the users get to use.
*/

bool AIUpdaterBridge::isRunning(void)
{
    if(mutex.tryLock()) {
        mutex.unlock();
        return false;
    }
    return true;
}

void AIUpdaterBridge::startUpdating(void)
{
    if(!mutex.tryLock()) {
        return;
    }
    stopUpdate = false; // reset in case.
    Promise = new QFuture<void>;
    *Promise = QtConcurrent::run(this, &AIUpdaterBridge::doUpdate);
    return;
}

void AIUpdaterBridge::stopUpdating(void)
{
    if(isRunning()) {
        stopUpdate = true;
    }
    return;
}

void AIUpdaterBridge::clear(void)
{
    appImage.clear();
    zsyncHeader.clear();
    zsyncFileName.clear();
    zsyncHeaderJson = QJsonObject(); // clean zsync header
    zsyncURL.clear();
    fileURL.clear();

    if(zsyncFile != NULL) {
        zsyncFile = NULL;
    }
    stopUpdate = false;
    return;
}

/* ------------------------------- */

/*
 * Private Slots
 * -------------
*/

void AIUpdaterBridge::handleAppImageUpdateInformation(const QString& appImage, const QJsonObject& config)
{
    /*
     * Disconnect with confidence!
    */

    disconnect(&AppImageInformer, SIGNAL(updateInformation(const QString&, const QJsonObject&)),
               this, SLOT(handleAppImageUpdateInformation(const QString&, const QJsonObject&)));
    disconnect(&AppImageInformer, SIGNAL(error(const QString&, short)),
               this, SLOT(handleAppImageUpdateError(const QString&, short)));

    if(config["transport"].toString() == "zsync") {
        this->zsyncURL = QUrl(config["zsyncUrl"].toString());
        if(debug) {
            qDebug() << "AIUpdaterBridge:: zsyncURL ::" << zsyncURL << " :: " << appImage;
        }
        checkForUpdates();
    } else if(config["transport"].toString() == "gh-releases-zsync") {
        // handle github releases zsync.
        QUrl releaseLink;
        releaseLink = QUrl("https://api.github.com/repos/" + config["username"].toString() +
                           "/"  + config["repo"].toString() + "/releases/");
        if(config["tag"].toString() == "latest") {
            releaseLink = QUrl(releaseLink.toString() + config["tag"].toString());
        } else {
            releaseLink = QUrl(releaseLink.toString() + "tags/" + config["tag"].toString());
        }
        if(debug) {
            qDebug() << "AIUpdaterBridge:: github release link ::" << releaseLink;
        }
        this->zsyncFileName = config["filename"].toString();
        getGitHubReleases(releaseLink);
    } else {
        // if its not github releases zsync or generic zsync
        // then it must be bintray-zsync
        // Note: Since QAIUpdateInformation can handle errors
        // we don't really have to check for integrity now.

        // handle bintray zsync.
        QUrl latestLink;
        latestLink = QUrl("https://bintray.com/" + config["username"].toString() +
                          "/" + config["repo"].toString() + "/" + config["packageName"].toString() + "/_latestVersion");
        this->zsyncFileName = config["filename"].toString();
        getBintrayLatestPackage(latestLink);
    }
    // There should be no errors at this stage.
    return;
}

void AIUpdaterBridge::handleAppImageUpdateError(const QString& appImage, short errorCode)
{
    if(!mutex.tryLock()) {
        mutex.unlock();
    } else {
        mutex.unlock();
    }
    emit error(appImage, UNABLE_TO_GET_APPIMAGE_INFORMATION);
    return;
}


void AIUpdaterBridge::getGitHubReleases(const QUrl& url)
{
    _CurrentRequest = QNetworkRequest(url);
    _pCurrentReply = _pManager->get(_CurrentRequest);

    connect(_pCurrentReply, &QNetworkReply::finished,
    [&]() {
        if(_pCurrentReply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() >= 400) {
            return;
        }

        QString Response(_pCurrentReply->readAll());
        _pCurrentReply->deleteLater(); // this will free the memory as soon as this object is not used.
        // by anyone or anything.
        if(debug) {
            qDebug() << "AIUpdaterBridge::GET::" << _CurrentRequest.url() << " :: success!";
        }
        handleGitHubReleases(Response);
        return;

    });
    connect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleNetworkErrors(QNetworkReply::NetworkError)));
    return;
}

void AIUpdaterBridge::getBintrayLatestPackage(const QUrl& url)
{
    _CurrentRequest = QNetworkRequest(url);
    _pCurrentReply = _pManager->head(_CurrentRequest);

    connect(_pCurrentReply, SIGNAL(redirected(const QUrl&)), this, SLOT(handleBintrayLatestPackage(const QUrl&)));
    connect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleNetworkErrors(QNetworkReply::NetworkError)));
    return;
}


void AIUpdaterBridge::handleNetworkErrors(QNetworkReply::NetworkError code)
{
    // avoid operation cancel errors.
    if(code == QNetworkReply::OperationCanceledError) {
        return;
    }
    if(debug) {
        qDebug() << "AIUpdaterBridge:: network error :: " << code;
    }
    if(!mutex.tryLock()) {
        mutex.unlock();
    } else {
        mutex.unlock();
    }
    emit error(appImage,  NETWORK_ERROR);
    return;
}

void AIUpdaterBridge::handleBintrayLatestPackage(const QUrl& url)
{
    disconnect(_pCurrentReply, SIGNAL(redirected(const QUrl&)), this, SLOT(handleBintrayLatestPackage(const QUrl&)));
    _pCurrentReply->abort();
    _pCurrentReply->deleteLater();
    _pCurrentReply = NULL;
    QString latestVersion = url.fileName();
    QStringList information = url.toString().split("/");
    zsyncFileName.replace("_latestVersion", latestVersion);
    zsyncURL = QUrl("https://dl.bintray.com/" + information[3] + "/" + information[4] + "/" + zsyncFileName);

    if(debug) {
        qDebug() << "AIUpdaterBridge:: bintray url :: " << zsyncURL;
    }

    if(zsyncURL.isEmpty()) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: cannot find zsync file ::" << zsyncFileName;
        }
        if(!mutex.tryLock()) {
            mutex.unlock();
        } else {
            mutex.unlock();
        }
        emit error(appImage, CANNOT_FIND_BINTRAY_PACKAGE);
    } else {
        // Got the zsync url so proceed to check for changes.
        checkForUpdates();
    }
    return;
}

void AIUpdaterBridge::handleGitHubReleases(const QString& content)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray assetsArray = jsonObject["assets"].toArray();
    QString version = jsonObject["tag_name"].toString();
    QVector<QJsonObject> assets;
    QRegExp rx(zsyncFileName); // Patern Matching with wildcards!
    rx.setPatternSyntax(QRegExp::Wildcard);

    if(debug) {
        qDebug() << "AIUpdaterBridge::Latest Version:: " << version;
        qDebug() << "AIUpdaterBridge::Asset Required:: " << zsyncFileName;
    }

    // Parse the array in the assets vector!
    foreach (const QJsonValue &value, assetsArray) {
        assets.push_back(value.toObject());
    }

    for(int i = 0; i < assets.size(); ++i) {
        if(debug) {
            qDebug() << "AIUpdaterBridge::Checking Asset::" << assets.at(i)["name"].toString();
        }
        if(rx.exactMatch(assets.at(i)["name"].toString())) {
            zsyncURL = assets.at(i)["browser_download_url"].toString();
            if(debug) {
                qDebug() << "AIUpdaterBridge::Latest Package::" << zsyncURL;
            }
            break;
        }
    }
    if(zsyncURL.isEmpty()) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: cannot find zsync file ::" << zsyncFileName;
        }
        if(!mutex.tryLock()) {
            mutex.unlock();
        } else {
            mutex.unlock();
        }
        emit error(appImage, CANNOT_FIND_GITHUB_ASSET);
    } else {
        // Got the zsync url so proceed to check for changes.
        checkForUpdates();
    }
    return;
}

void AIUpdaterBridge::handleZsyncHeader(qint64 bytesRecived, qint64 bytesTotal)
{
    // Since we don't need any progress with this
    (void)bytesTotal;

    if(_pCurrentReply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() >= 400) {
        return;
    }

    zsyncHeader += QString(_pCurrentReply->readAll());
    // Stop the request after we see two newlines or return feed.
    if((zsyncHeader.contains("\n\n") || zsyncHeader.contains("\n\n\r")) && _pCurrentReply != NULL) {
        disconnect(_pCurrentReply, SIGNAL(downloadProgress(qint64, qint64)),
                   this, SLOT(handleZsyncHeader(qint64, qint64)));
        _pCurrentReply->abort(); // stop the request.
        _pCurrentReply->deleteLater();
        _pCurrentReply = NULL;
        // clean our header first
        zsyncHeader = zsyncHeader.split("\n\n")[0];

        // Parse to json for more cleaner usage.
        QStringList zsyncHeaders = zsyncHeader.split("\n");
        if(zsyncHeaders.size() == 8) {
            zsyncHeaderJson = QJsonObject {
                {"zsync", zsyncHeaders.at(0).split("zsync: ")[1]},
                {"Filename", zsyncHeaders.at(1).split("Filename: ")[1]},
                {"MTime", zsyncHeaders.at(2).split("MTime: ")[1]},
                {"Blocksize", zsyncHeaders.at(3).split("Blocksize: ")[1]},
                {"Length", zsyncHeaders.at(4).split("Length: ")[1]},
                {"Hash-Lengths", zsyncHeaders.at(5).split("Hash-Lengths: ")[1]},
                {"URL" 	, zsyncHeaders.at(6).split("URL: ")[1]},
                {"SHA-1", zsyncHeaders.at(7).split("SHA-1: ")[1]}
            };
        } else {
            if(debug) {
                qDebug() << "AIUpdaterBridge:: zsync header invalid parameters:: " << zsyncHeaders.size();
            }
            if(!mutex.tryLock()) {
                mutex.unlock();
            } else {
                mutex.unlock();
            }
            emit error(appImage, ZSYNC_HEADER_INVALID);
            return;
        }
        if(debug) {
            qDebug() << "AIUpdaterBridge::GOT:: zsync headers :: success!";
        }
        // compare the headers SHA1 and the local files SHA1
        // to confirm if we need to update or not!
        QFile AppImage(appImage);
        if(!AppImage.open(QIODevice::ReadOnly)) {
            if(debug) {
                qDebug() << "AIUpdaterBridge:: file not found :: " << appImage;
            }
            if(!mutex.tryLock()) {
                mutex.unlock();
            } else {
                mutex.unlock();
            }
            emit error(appImage, APPIMAGE_NOT_FOUND);
            return;
        }
        QString RemoteFileName = zsyncHeaderJson["Filename"].toString();
        QString LocalFileName  = QFileInfo(appImage).fileName();
        QString RemoteSHA1 = zsyncHeaderJson["SHA-1"].toString();
        QString LocalSHA1(QCryptographicHash::hash(AppImage.readAll(), QCryptographicHash::Sha1).toHex());

        if(RemoteSHA1 != LocalSHA1) {
            if(debug) {
                qDebug() << "AIUpdaterBridge:: your version of appimage is older:: " << LocalSHA1;
            }
            // Lets prepare for the update before we tell the user.
            QUrl alphaFile(zsyncHeaderJson["Filename"].toString());
            if(!alphaFile.isValid() || alphaFile.isRelative()) {
                // then it must be relative.
                QUrl betaFile(zsyncURL);
                betaFile = betaFile.adjusted(QUrl::RemoveFilename);
                betaFile = QUrl(betaFile.toString() + alphaFile.toString());
                alphaFile = betaFile;
            }
            // emit updatesAvailable after we setup everything for the user.
            _CurrentRequest = QNetworkRequest(alphaFile);
            _pCurrentReply = _pManager->get(_CurrentRequest);
            connect(_pCurrentReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(resolveRedirections(qint64, qint64 )));
            connect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(handleNetworkErrors(QNetworkReply::NetworkError)));
        } else {
            if(debug) {
                qDebug() << "AIUpdaterBridge:: no new updates available :: " << RemoteSHA1;
            }
            mutex.unlock(); // remember this lock we put on setAppImageUpdateInformation
            emit noUpdatesAvailable(appImage, RemoteSHA1);
        }
        return;
    }
    return;

}

void AIUpdaterBridge::constructZsync()
{
    QByteArray resp(_pCurrentReply->readAll());
    QString tempFilePath(QFileInfo(appImage).fileName() + ".part");
    auto *memFile = fmemopen(resp.data(), resp.size(), "r");
    if((zsyncFile = zsync_begin(memFile, 0, QDir::currentPath().toStdString().c_str())) == nullptr) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: failed to open zsync handle.";
        }
        if(!mutex.tryLock()) {
            mutex.unlock();
        } else {
            mutex.unlock();
        }
        emit error(appImage, FAILED_TO_OPEN_ZSYNC_HANDLE);
        return;
    } else {
        // rename file.
        if(zsync_rename_file(zsyncFile, tempFilePath.toStdString().c_str()) != 0) {
            if(debug) {
                qDebug() << "AIUpdaterBridge:: failed to rename temporary file.";
            }
            if(!mutex.tryLock()) {
                mutex.unlock();
            } else {
                mutex.unlock();
            }
            emit error(appImage, FAILED_TO_RENAME_TEMPFILE);
            return;
        }
        // start update
        QString RemoteSHA1 = zsyncHeaderJson["SHA-1"].toString();
        if(debug) {
            qDebug() << "AIUpdaterBridge:: new updates available :: " << RemoteSHA1;
        }
        mutex.unlock();
        emit updatesAvailable(appImage, RemoteSHA1);
    }
    _pCurrentReply->deleteLater();
    return;
}

void AIUpdaterBridge::resolveRedirections(qint64 bytesReceived, qint64 bytesTotal)
{
    /*
     * Since we have no use of these.
    */
    (void)bytesReceived;
    (void)bytesTotal;

    QUrl redirected = _pCurrentReply->url();
    if(!redirected.isEmpty()) { // Stop the request when we get a valid redirection!
        disconnect(_pCurrentReply, SIGNAL(downloadProgress(qint64, qint64)),
                   this, SLOT(resolveRedirections(qint64, qint64)));
        _pCurrentReply->abort(); // stop the request.
        _pCurrentReply->deleteLater();
        _pCurrentReply = NULL;
        if(debug) {
            qDebug() << "AIUpdaterBridge:: redirected url :: " << redirected;
        }

        fileURL = redirected; // setURL
        QDir::setCurrent ( QDir(QFileInfo(appImage).absoluteDir()).absolutePath() ); // set current dir
        // Now we are set to construct the zsHandle.

        _CurrentRequest = QNetworkRequest(zsyncURL);
        _pCurrentReply = _pManager->get(_CurrentRequest);

        connect(_pCurrentReply, SIGNAL(finished()), this, SLOT(constructZsync()));
        connect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(handleNetworkErrors(QNetworkReply::NetworkError)));
    }
    return;
}

void AIUpdaterBridge::checkForUpdates(void)
{
    _CurrentRequest = QNetworkRequest(zsyncURL);
    // We only need the first 1KiB.
    _CurrentRequest.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    _pCurrentReply = _pManager->get(_CurrentRequest);

    connect(_pCurrentReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(handleZsyncHeader(qint64, qint64)));
    connect(_pCurrentReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleNetworkErrors(QNetworkReply::NetworkError)));
    return;
}


void AIUpdaterBridge::doUpdate(void)
{
    if(fileURL.isEmpty() || zsyncFile == nullptr) {
        return;
    }
    QTime  downloadSpeed; // to calculate download speed.
    static const auto BUFFERSIZE = 8192;

    /*
     * Before starting the fetch , lets add the seed files first!
     * ----------------------------------------------------------
     * Since the AppImage is checked all the time over the code
     * we don't have to check it here , but still , just to be safe.
    */
    {
        FILE *MainAppImage,
             *BrokenDownloads;
        QFile AppImage(appImage);
        QFile brokenDownloads(appImage + ".part");
        if(!AppImage.open(QIODevice::ReadOnly)) {
            mutex.unlock();
            qDebug() << "AIUpdaterBridge:: cannot find appimage.";
            emit error(appImage, APPIMAGE_NOT_FOUND);
            return;
        } else {
            MainAppImage = fdopen(AppImage.handle(), "r");
        }

        if(!brokenDownloads.open(QIODevice::ReadOnly)) {
            // No broken downloads
            BrokenDownloads = NULL;
        } else {
            BrokenDownloads = fdopen(brokenDownloads.handle(), "r");
        }

        zsync_submit_source_file(zsyncFile, MainAppImage, false);
        if(BrokenDownloads != NULL) {
            zsync_submit_source_file(zsyncFile, BrokenDownloads, false);
        }
        // All Files get closed here.
    }


    if(debug) {
        qDebug() << "AIUpdaterBridge:: got everything :: Updating";
    }

    int urlType = 0,
        ret = 0;

    struct range_fetch* rf = range_fetch_start(fileURL.toEncoded().data());
    struct zsync_receiver* zr;

    if (rf == nullptr) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: range fetch failed ::" << appImage;
        }
        mutex.unlock();
        emit error(appImage, ZSYNC_RANGE_FETCH_FAILED);
        return;
    }

    zr = zsync_begin_receive(zsyncFile, urlType);
    if (zr == nullptr) {
        range_fetch_end(rf);
        if(debug) {
            qDebug() << "AIUpdaterBridge:: zsync recieve failed ::" << appImage;
        }
        mutex.unlock();
        emit error(appImage, ZSYNC_RECIEVE_FAILED);
        return;
    }

    std::vector<unsigned char> buffer;
    try {
        buffer.reserve(BUFFERSIZE);
    } catch (std::bad_alloc& e) {
        zsync_end_receive(zr);
        range_fetch_end(rf);
        if(debug) {
            qDebug() << "AIUpdaterBridge:: bad alloc.";
        }
        mutex.unlock();
        emit error(appImage, BAD_ALLOC);
        return;
    }

    {   /* Get a set of byte ranges that we need to complete the target */
        int nrange;
        auto* zbyterange = zsync_needed_byte_ranges(zsyncFile, &nrange, urlType);
        if (zbyterange == nullptr) {
            if(debug) {
                qDebug() << "AIUpdaterBridge:: zsync recieve failed ::" << appImage;
            }
            mutex.unlock();
            emit error(appImage, ZSYNC_RECIEVE_FAILED);
            return;
        }
        if (nrange == 0) {
            if(debug) {
                qDebug() << "AIUpdaterBridge:: zsync recieve failed ::" << appImage;
            }
            mutex.unlock();
            emit error(appImage, ZSYNC_RECIEVE_FAILED);
            return;

        }

        downloadSpeed.start();

        for(int i = 0; i < 2 * nrange && !stopUpdate; i++) {
            auto beginbyte = zbyterange[i];
            i++;
            auto endbyte = zbyterange[i];
            off_t single_range[2] = {beginbyte, endbyte};
            /* And give that to the range fetcher */
            /* Only one range at a time because Akamai can't handle more than one range per request */
            range_fetch_addranges(rf, single_range, 1);

            {
                int len;
                off_t zoffset;

                /* Loop while we're receiving data, until we're done or there is an error */
                while (!ret
                       && (len = get_range_block(rf, &zoffset, buffer.data(), BUFFERSIZE)) > 0 && !stopUpdate) {
                    /* Pass received data to the zsync receiver, which writes it to the
                     * appropriate location in the target file */
                    if (zsync_receive_data(zr, buffer.data(), zoffset, len) != 0)
                        ret = 1;

                    {
                        long long zgot, ztot;
                        qint64 bytesReceived, bytesTotal;
                        zsync_progress(zsyncFile, &zgot, &ztot);
                        double percentage = (double) zgot / (double) ztot;

                        // Lets calculate the speed too.
                        bytesReceived = (qint64)range_fetch_bytes_down(rf);
                        bytesTotal    = (qint64)zsyncHeaderJson["Length"].toString().toInt();
                        double speed = bytesReceived * 1000.0 / downloadSpeed.elapsed();

                        QString unit;
                        if (speed < 1024) {
                            unit = "bytes/sec";
                        } else if (speed < 1024*1024) {
                            speed /= 1024;
                            unit = "kB/s";
                        } else {
                            speed /= 1024*1024;
                            unit = "MB/s";
                        }

                        emit progress((float) percentage * 100.0f, bytesReceived, bytesTotal, speed, unit);
                    }
                    // Needed in case next call returns len=0 and we need to signal where the EOF was.
                    zoffset += len;
                }

                /* If error, we need to flag that to our caller */
                if (len < 0) {
                    if(debug) {
                        qDebug() << "AIUpdaterBridge:: zsync recieve failed ::" << appImage;
                    }
                    mutex.unlock();
                    emit error(appImage, ZSYNC_RECIEVE_FAILED);
                    return;
                } else {
                    /* Else, let the zsync receiver know that we're at EOF; there
                     *could be data in its buffer that it can use or needs to process */
                    zsync_receive_data(zr, nullptr, zoffset, 0);
                }
            }

        }

        free(zbyterange);
    }

    if(stopUpdate) {
        appImage.clear();
        zsyncHeader.clear();
        zsyncFileName.clear();
        zsyncHeaderJson = QJsonObject(); // clean zsync header
        zsyncURL.clear();
        fileURL.clear();

        if(zsyncFile != NULL) {
            free(zsyncFile);
            zsyncFile = NULL;
        }
        stopUpdate = false;
        mutex.unlock();
        emit(stopped());
        return;
    }

    emit progress(100, (qint64)zsyncHeaderJson["Length"].toString().toInt(), (qint64)zsyncHeaderJson["Length"].toString().toInt(), 0, "Kbps");
    zsync_end_receive(zr);
    range_fetch_end(rf);
    zsync_complete(zsyncFile);
    zsync_end(zsyncFile);

    // Verify temporary file and replace.
    QString tempFilePath(QFileInfo(appImage).fileName() + ".part");
    QFile AppImage(tempFilePath);
    if(!AppImage.open(QIODevice::ReadOnly)) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: temporary file not found :: " << tempFilePath;
        }
        mutex.unlock();
        emit error(appImage, APPIMAGE_NOT_FOUND);
        return;
    }
    QString RemoteSHA1 = zsyncHeaderJson["SHA-1"].toString();
    QString LocalSHA1(QCryptographicHash::hash(AppImage.readAll(), QCryptographicHash::Sha1).toHex());
    // final checksum verification with sha1 sum.
    if(RemoteSHA1 != LocalSHA1) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: failed to prove integrity :: "<< RemoteSHA1 << "!=" << LocalSHA1;
        }
        mutex.unlock();
        emit error(appImage, UPDATE_INTEGRITY_FAILED);
        return;
    } else {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: SHA1 Sum Matched -> Integrity Proved :: " << RemoteSHA1;
        }
        // remove old backups and create new backups

        auto LocalFile = QFileInfo(appImage).fileName();

        if(QFileInfo(LocalFile  + ".zs-old").exists()) {
            if(!QFile::remove(LocalFile + ".zs-old")) {
                if(debug) {
                    qDebug() << "AIUpdaterBridge:: Post installation error.";
                }
                mutex.unlock();
                emit error(LocalFile, POST_INSTALLATION_FAILED);
                return;
            }
        }

        if(!QFile::rename(LocalFile, LocalFile + ".zs-old")) {
            if(debug) {
                qDebug() << "AIUpdaterBridge:: Post installation error.";
            }
            mutex.unlock();
            emit error(LocalFile, POST_INSTALLATION_FAILED);
            return;
        }

        if(QFileInfo(zsyncHeaderJson["Filename"].toString()).exists()) {
            if(!QFile::remove(zsyncHeaderJson["Filename"].toString())) {
                if(debug) {
                    qDebug() << "AIUpdaterBridge:: Post installation error.";
                }
                mutex.unlock();
                emit error(zsyncHeaderJson["Filename"].toString(), POST_INSTALLATION_FAILED);
                return;
            }

        }

        if(
            !(
                QFile::rename(tempFilePath, zsyncHeaderJson["Filename"].toString()) &&
                QFile::setPermissions(zsyncHeaderJson["Filename"].toString(), QFile(LocalFile + ".zs-old").permissions())
            )
        ) {

        }
        emit updateFinished(appImage, RemoteSHA1);  // Yeaa , Finally Finished Gracefully!
    }
    mutex.unlock();
    return;
}

/* ----------------------- */
