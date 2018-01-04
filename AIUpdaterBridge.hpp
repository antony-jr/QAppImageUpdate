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
 *  Constructors:
 *      explicit AIUpdaterBridge(QObject *parent = NULL)   - Only Construct the class.
*/
class AIUpdaterBridge : public QObject // START CLASS AIUpdaterBridge
{
    Q_OBJECT
public: 
    /*
     * Error codes.
     * -----------
    */
    enum {
	UNABLE_TO_GET_APPIMAGE_INFORMATION
    };

    explicit AIUpdaterBridge(QObject *parent = NULL)
        : QObject(parent)
    {
	    return;
    }
    explicit AIUpdaterBridge(const QString&); // get info from appimage
    explicit AIUpdaterBridge(const QJsonObject&); // get info from json
    void doDebug(bool);

    ~AIUpdaterBridge() { }
public slots:
    void setAppImageUpateInformation(const QString&);
private slots:
    void handleAppImageUpdateInformation(const QString& , const QJsonObject&);
    void handleAppImageUpdateError(const QString& , short );
signals:
    void error(const QString& , short );
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
     *     "transport"     : "zsync",
     *     "url"           : "http://server.domain/path/Application-latest-x86_64.AppImage.zsync",
     *     "cacheFiles"    : false
     * }
     *
     * Using gh-releases-zsync
     *
     * {
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
    QString filename; // if we got github or bintray info!
    QUrl zsyncURL;
    
    QAIUpdateInformation AppImageInformer;
    QString currentWorkingDirectory;
    struct zsync_state *zsyncFile; // legacy
    off_t remoteFileSizeCache;
    bool debug = false;
};// END CLASS AIUpdaterBridge
#endif // AIUPDATER_BRIDGE_HPP_INCLUDED
