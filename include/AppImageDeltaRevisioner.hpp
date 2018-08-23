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
 * @description : This where the delta revisioner is described.
 * Delta Revisioner is the public API to manage the entire revision
 * of a AppImage. From retrival of the embeded information from the
 * AppImage to the retrival of required remaining block ranges.
 * This is a controller to all the internal mechanisms , Handled neatly
 * via Qt's signals and slots.
 *
*/
#ifndef APPIMAGE_UPDATER_BRIDGE_HPP_INCLUDED
#define APPIMAGE_UPDATER_BRIDGE_HPP_INCLUDED
#include <QFile>
#include <QtGlobal>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QThread>

namespace AppImageUpdaterBridge
{
class AppImageUpdateInformationPrivate;
class ZsyncRemoteControlFileParserPrivate;
class ZsyncWriterPrivate;
class ZsyncBlockRangeDownloaderPrivate;

class AppImageDeltaRevisioner : public QObject
{
    Q_OBJECT
public:
    enum : short {
        NO_ERROR = 0,
        APPIMAGE_NOT_READABLE = 1, //  50 < and > 0 , AppImage Update Information Error.
        NO_READ_PERMISSION,
        APPIMAGE_NOT_FOUND,
        CANNOT_OPEN_APPIMAGE,
        EMPTY_UPDATE_INFORMATION,
        INVALID_APPIMAGE_TYPE,
        INVALID_MAGIC_BYTES,
        INVALID_UPDATE_INFORMATION,
        NOT_ENOUGH_MEMORY,
        SECTION_HEADER_NOT_FOUND,
        UNSUPPORTED_ELF_FORMAT,
        UNSUPPORTED_TRANSPORT,
        UNKNOWN_NETWORK_ERROR = 50, // >= 50 , Zsync Control File Parser Error.
        IO_READ_ERROR,
        ERROR_RESPONSE_CODE,
        GITHUB_API_RATE_LIMIT_REACHED,
        NO_MARKER_FOUND_IN_CONTROL_FILE,
        INVALID_ZSYNC_HEADERS_NUMBER,
        INVALID_ZSYNC_MAKE_VERSION,
        INVALID_ZSYNC_TARGET_FILENAME,
        INVALID_ZSYNC_MTIME,
        INVALID_ZSYNC_BLOCKSIZE,
        INVALID_TARGET_FILE_LENGTH,
        INVALID_HASH_LENGTH_LINE,
        INVALID_HASH_LENGTHS,
        INVALID_TARGET_FILE_URL,
        INVALID_TARGET_FILE_SHA1,
        HASH_TABLE_NOT_ALLOCATED = 100, // >= 100 , Zsync Writer error.
        INVALID_TARGET_FILE_CHECKSUM_BLOCKS,
        CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS,
        CANNOT_CONSTRUCT_HASH_TABLE,
        QBUFFER_IO_READ_ERROR,
        SOURCE_FILE_NOT_FOUND,
        NO_PERMISSION_TO_READ_SOURCE_FILE,
        CANNOT_OPEN_SOURCE_FILE,
        NO_PERMISSION_TO_READ_WRITE_TARGET_FILE,
        CANNOT_OPEN_TARGET_FILE,
        TARGET_FILE_SHA1_HASH_MISMATCH,
        CANNOT_CONSTRUCT_TARGET_FILE
    } error_code;

    enum : short {
        INITIALIZING = 0,
        IDLE = 1,
        OPENING_APPIMAGE,
        CALCULATING_APPIMAGE_SHA1_HASH,
        READING_APPIMAGE_MAGIC_BYTES,
        FINDING_APPIMAGE_ARCHITECTURE,
        MAPPING_APPIMAGE_TO_MEMORY,
        SEARCHING_FOR_UPDATE_INFORMATION_SECTION_HEADER,
        UNMAPPING_APPIMAGE_FROM_MEMORY,
        FINALIZING_APPIMAGE_EMBEDED_UPDATE_INFORMATION,
        PARSING_APPIMAGE_EMBEDED_UPDATE_INFORMATION = 50,
        REQUESTING_GITHUB_API,
        PARSING_GITHUB_API_RESPONSE,
        REQUESTING_ZSYNC_CONTROL_FILE,
        REQUESTING_BINTRAY,
        PARSING_BINTRAY_REDIRECTED_URL_FOR_LATEST_PACKAGE_URL,
        PARSING_ZSYNC_CONTROL_FILE,
        SEARCHING_TARGET_FILE_CHECKSUM_BLOCK_OFFSET_IN_ZSYNC_CONTROL_FILE,
        STORING_ZSYNC_CONTROL_FILE_DATA_TO_MEMORY,
        FINALIZING_PARSING_ZSYNC_CONTROL_FILE,
        WRITTING_DOWNLOADED_BLOCK_RANGES = 100,
        EMITTING_REQUIRED_BLOCK_RANGES,
        CHECKING_CHECKSUMS_FOR_DOWNLOADED_BLOCK_RANGES,
        WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE,
        CALCULATING_TARGET_FILE_SHA1_HASH,
        CONSTRUCTING_TARGET_FILE
    } status_code;

    explicit AppImageDeltaRevisioner(bool singleThreaded = true, QObject *parent = nullptr);
    explicit AppImageDeltaRevisioner(const QString&, bool singleThreaded = true, QObject *parent = nullptr);
    explicit AppImageDeltaRevisioner(QFile *, bool singleThreaded = true, QObject *parent = nullptr);
    ~AppImageDeltaRevisioner();

    static QString errorCodeToString(short);
    static QString statusCodeToString(short);

public Q_SLOTS:
    AppImageDeltaRevisioner &start(void);
    AppImageDeltaRevisioner &cancel(void);
    AppImageDeltaRevisioner &setAppImage(const QString&);
    AppImageDeltaRevisioner &setAppImage(QFile*);
    AppImageDeltaRevisioner &setShowLog(bool);
    AppImageDeltaRevisioner &getAppImageEmbededInformation(void);
    AppImageDeltaRevisioner &checkForUpdate(void);
    AppImageDeltaRevisioner &clear(void);

    QNetworkReply::NetworkError getNetworkError(void);
private Q_SLOTS:
    void handleZsyncRemoteControlFileParserError(short);
    void handleNetworkError(QNetworkReply::NetworkError);
    void handleIndeterminateProgress(int);
    void handleBlockDownloaderStarted(void);
    void handleDeltaWriterFinished(bool);
    void handleUpdateAvailable(bool, QString);
    void handleUpdateCheckInformation(QJsonObject);

Q_SIGNALS:
    void started(void);
    void canceled(void);
    void finished(void);
    void embededInformation(QJsonObject);
    void updateAvailable(bool, QString);
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

#endif // APPIMAGE_DELTA_WRITER_HPP_INCLUDED
