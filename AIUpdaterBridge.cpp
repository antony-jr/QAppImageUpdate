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

AIUpdaterBridge::AIUpdaterBridge(const QString& appImage)
{
    _pManager = new QNetworkAccessManager(this);
    _pManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    setAppImageUpdateInformation(appImage);
    return;
}

AIUpdaterBridge::AIUpdaterBridge(const QJsonObject& config)
{
    _pManager = new QNetworkAccessManager(this);
    _pManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    setAppImageUpdateInformation(config);
    return;
}

void AIUpdaterBridge::doDebug(bool ch)
{
    debug = ch;
    return;
}


void AIUpdaterBridge::setAppImageUpdateInformation(const QString& appImage)
{
    connect(&AppImageInformer, SIGNAL(updateInformation(const QString&, const QJsonObject&)),
            this, SLOT(handleAppImageUpdateInformation(const QString&, const QJsonObject&)));
    connect(&AppImageInformer, SIGNAL(error(const QString&, short)),
            this, SLOT(handleAppImageUpdateError(const QString&, short)));
    this->appImage = appImage;
    AppImageInformer.setAppImage(appImage);
    AppImageInformer.doDebug(debug);
    AppImageInformer.start();
    return;
}

void AIUpdaterBridge::setAppImageUpdateInformation(const QJsonObject& config)
{
    // Since this is from the user the data can have a lot
    // of errors. Must check before use

    if(!config["appImagePath"].isString() || config["appImagePath"].isNull()) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: 'appImagePath' entry missing in the given json ::" << config;
        }
        emit error(QString("NONE"), APPIMAGE_PATH_NOT_GIVEN);
        return;
    } else {
        // set our appImage
        this->appImage = config["appImagePath"].toString();
    }

    if(!config["transport"].isString() || config["transport"].isNull()) {
        if(debug) {
            qDebug() << "AIUpdaterBridge:: 'transport' entry missing in the given json ::" << config;
        }
        emit error(appImage, TRANSPORT_NOT_GIVEN);
        return;
    } else {
        if(config["transport"].toString() == "zsync") {
            if(!config["url"].isString() || config["url"].isNull()) {
                if(debug) {
                    qDebug() << "AIUpdaterBridge:: 'url' entry missing in the given json ::" << config;
                }
                emit error(appImage, URL_NOT_GIVEN);
                return;
            } else {
                this->zsyncURL = QUrl(config["url"].toString());
                if(debug) {
                    qDebug() << "AIUpdaterBridge:: zsyncURL ::" << zsyncURL << " :: " << appImage;
                }
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
                emit error(appImage, INVALID_UPD_INFO_PARAMENTERS);
                return;
            }
            if(debug) {
                qDebug() << "AIUpdaterBridge:: sorry but this is not implemented yet!";
            }
            emit error(appImage, NOT_IMPLEMENTED_YET);
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
            emit error(appImage, INVALID_TRANSPORT_GIVEN);
            return;
        }
    }
    return;
}

// Private Slots

void AIUpdaterBridge::handleAppImageUpdateInformation(const QString& appImage, const QJsonObject& config)
{
    if(config["transport"].toString() == "zsync") {
        this->zsyncURL = QUrl(config["url"].toString());
        if(debug) {
            qDebug() << "AIUpdaterBridge:: zsyncURL ::" << zsyncURL << " :: " << appImage;
        }
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
        if(debug) {
            qDebug() << "AIUpdaterBridge:: sorry but this is not implemented yet!";
        }
        emit error(appImage, NOT_IMPLEMENTED_YET);
        return;
    }
    // There should be no errors at this stage.
    return;
}

void AIUpdaterBridge::handleAppImageUpdateError(const QString& appImage, short errorCode)
{
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

void AIUpdaterBridge::handleNetworkErrors(QNetworkReply::NetworkError code)
{
    // avoid operation cancel errors.
    if(code == QNetworkReply::OperationCanceledError) {
        return;
    }
    if(debug) {
        qDebug() << "AIUpdaterBridge:: network error :: " << code;
    }
    emit error(appImage,  NETWORK_ERROR);
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
        if(debug) {
            qDebug() << "AIUpdaterBrdige:: zsync header :: " << zsyncHeader;
        }
        // compare the headers SHA1 and the local files SHA1
        // to confirm if we need to update or not!
        return;
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


// This is where everthing runs seperate from current main thread
// to avoid blocking of the gui thread , which may cause lags
// in the GUI.
void AIUpdaterBridge::run(void)
{
    if(zsyncURL.isEmpty()) {
        return;
    }
    if(debug) {
        qDebug() << "AIUpdaterBridge:: got everything :: ready for update!";
    }
    return;
}
