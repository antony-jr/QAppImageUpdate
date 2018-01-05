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
 * -----------------------------------------------------------------------------
 *  @filename           : AIUpdaterBridge.hpp
 *  @description        : Header for AppImage Updater Bridge.
 *  			  Class Declaration and Prototypes.
 *  			  This defines how the library is designed.
 * -----------------------------------------------------------------------------
*/
#if !defined(AIUPDATER_BRIDGE_HPP_INCLUDED)
#define AIUPDATER_BRIDGE_HPP_INCLUDED
#include <QtCore>
#include <QAIUpdateInformation.hpp>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <zsglobal.h>
extern "C" {
#include <zsync.h>
#include <zlib.h>
}

/*
 * Class AIUpdaterBridge  <- Inherits QObject.
 * ---------------------
 *
 *  This is the main class that handles AppImage Updates like
 *  a pro.
 *
 *  Public Functions:
 *      explicit AIUpdaterBridge(QObject *parent = NULL , QNetworkAccessManager *toUseManager = NULL)   - Only Construct the class.
 *      explicit AIUpdaterBridge(const QString&) - Extract Update information from AppImage and Constructs the class.
 *      explicit AIUpdaterBridge(const QJsonObject&) - Extract Update information directly from json and Construct the class.
 *
 *      void doDebug(bool)  - Set Debuging.
 *
*/
class AIUpdaterBridge : public QThread // START CLASS AIUpdaterBridge
{
    Q_OBJECT
public:
    /*
     * Error codes.
     * -----------
    */
    enum {
        UNABLE_TO_GET_APPIMAGE_INFORMATION,
        APPIMAGE_PATH_NOT_GIVEN,
        TRANSPORT_NOT_GIVEN,
        URL_NOT_GIVEN,
        INVALID_UPD_INFO_PARAMENTERS,
        INVALID_TRANSPORT_GIVEN,
        NOT_IMPLEMENTED_YET,
        NETWORK_ERROR,
        CANNOT_FIND_GITHUB_ASSET,
        ZSYNC_HEADER_INVALID,
        APPIMAGE_NOT_FOUND
    };

    explicit AIUpdaterBridge(QNetworkAccessManager *toUseManager = NULL)
    {
        // According to the Qt Docs , A Single QNetworkAccessManager is capable
        // of doing multiple request with less footprint as possible , it is also recommended
        // that a single QNetworkAccessManager is more than enough for a Qt App ,
        // So lets have a option to use this advantage.
        _pManager = (toUseManager == NULL) ? new QNetworkAccessManager(this) : toUseManager;
        _pManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        return;
    }
    explicit AIUpdaterBridge(const QString&); // get info from appimage
    explicit AIUpdaterBridge(const QJsonObject&); // get info from json
    void doDebug(bool);

    void run(void) override;

    void setAppImageUpdateInformation(const QString&);
    void setAppImageUpdateInformation(const QJsonObject&);

    ~AIUpdaterBridge()
    {
        _pManager->deleteLater();
    }
private slots:
    void handleAppImageUpdateInformation(const QString&, const QJsonObject&);
    void handleAppImageUpdateError(const QString&, short );
    void getGitHubReleases(const QUrl&);
    void handleGitHubReleases(const QString&);
    void handleZsyncHeader(qint64, qint64);
    void handleRedirects(const QUrl&);
    void resolveUrlAndEmitUpdatesAvailable(const QUrl&);
    void handleNetworkErrors(QNetworkReply::NetworkError);

    void checkForUpdates(void);
signals:
    void updatesAvailable(const QString&, const QString&);
    void noUpdatesAvailable(const QString&, const QString&);
    void progress(double); // in percentage
    void error(const QString&, short );
private:
    /*
     * configuration  <- Json
     * -------------
     *
     * Just define the transport type and type the config , the
     * bridge will automatically search for the required fields.
     *
     * Using generic zsync.
     *
     * {
     *     "appImagePath"  : "something/path/appImage.AppImage",
     *     "transport"     : "zsync",
     *     "url"           : "http://server.domain/path/Application-latest-x86_64.AppImage.zsync",
     * }
     *
     * Using gh-releases-zsync
     *
     * {
     *     "appImagePath" : "some/path/appImage.AppImage",
     *     "transport" : "gh-releases-zsync",
     *     "username"  : "antony-jr",
     *     "repo"      : "appImage",
     *     "tag"       : "latest",
     *     "filename"  : "file.AppImage*.zsync"
     * }
     *
     * Using bintray-zsync
     *
     * {
     *     "appImagePath" : "some/path/appImage.AppImage",
     *     "transport" : "bintray-zsync",
     *     "username"  : "antony-jr",
     *     "repo"      : "appImage",
     *     "packageName"   : "AppImageUpdater",
     *     "filename"      : "file.AppImage_latest_.zsync"
     * }
     *
    */

    /*
     * The end resultant from configuration
    */
    QString appImage,
            zsyncHeader,
            zsyncFileName;
    QJsonObject zsyncHeaderJson; // clean zsync header
    QUrl zsyncURL,
         fileURL;

    QAIUpdateInformation AppImageInformer;
    QString currentWorkingDirectory;
    struct zsync_state *zsyncFile; // zsync legacy
    off_t remoteFileSizeCache;
    bool debug = false;
    QEventLoop Pause;

    /*
     * Networking Support by Qt
    */
    QNetworkAccessManager    *_pManager = NULL;
    QNetworkRequest           _CurrentRequest;
    QNetworkReply            *_pCurrentReply = NULL;
};// END CLASS AIUpdaterBridge
#endif // AIUPDATER_BRIDGE_HPP_INCLUDED
