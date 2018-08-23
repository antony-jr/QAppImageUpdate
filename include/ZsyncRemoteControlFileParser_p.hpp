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
 * @filename    : ZsyncRemoteControlFileParser_p.hpp
 * @description : This is where the zsync control file parser in described.
*/
#ifndef ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
#define ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
#include <QBuffer>
#include <cmath>
#include <QCoreApplication>
#include <QDebug>
#include <QtEndian>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QMetaMethod>
#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <ZsyncInternalStructures_p.hpp>

namespace AppImageUpdaterBridge
{
class ZsyncRemoteControlFileParserPrivate : public QObject
{
    Q_OBJECT
public:
    enum : short {
        UNKNOWN_NETWORK_ERROR = 50,
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
        INVALID_TARGET_FILE_SHA1
    } error_code;

    enum : short {
        INITIALIZING = 0,
        IDLE = 1,
        PARSING_APPIMAGE_EMBEDED_UPDATE_INFORMATION = 50,
        REQUESTING_GITHUB_API,
        PARSING_GITHUB_API_RESPONSE,
        REQUESTING_ZSYNC_CONTROL_FILE,
        REQUESTING_BINTRAY,
        PARSING_BINTRAY_REDIRECTED_URL_FOR_LATEST_PACKAGE_URL,
        PARSING_ZSYNC_CONTROL_FILE,
        SEARCHING_TARGET_FILE_CHECKSUM_BLOCK_OFFSET_IN_ZSYNC_CONTROL_FILE,
        STORING_ZSYNC_CONTROL_FILE_DATA_TO_MEMORY,
        FINALIZING_PARSING_ZSYNC_CONTROL_FILE
    } status_code;

    explicit ZsyncRemoteControlFileParserPrivate(QNetworkAccessManager*);
    ~ZsyncRemoteControlFileParserPrivate();

    static QString errorCodeToString(short);
    static QString statusCodeToString(short);
public Q_SLOTS:
    void clear(void);
    void setControlFileUrl(const QUrl&);
    void setControlFileUrl(QJsonObject);
    void setLoggerName(const QString&);
    void setShowLog(bool);
    void getControlFile(void);
    void getUpdateCheckInformation(void);
    void getZsyncInformation(void);
    qint32 getTargetFileBlocksCount(void);
    QUrl getTargetFileUrl(void);
    QUrl getControlFileUrl(void);
    QString getZsyncMakeVersion(void);
    QString getTargetFileName(void);
    QString getTargetFileSHA1(void);
    QDateTime getMTime(void);
    qint32 getTargetFileBlockSize(void);
    qint32 getTargetFileLength(void);
    qint32 getWeakCheckSumBytes(void);
    qint32 getStrongCheckSumBytes(void);
    qint32 getConsecutiveMatchNeeded(void);
private Q_SLOTS:
    void checkHeadTargetFileUrl(qint64, qint64);
    void handleBintrayRedirection(const QUrl&);
    void handleGithubAPIResponse(void);
    void handleDownloadProgress(qint64, qint64);
    void handleControlFile(void);
    void handleNetworkError(QNetworkReply::NetworkError);
    void handleErrorSignal(short);
#ifndef LOGGING_DISABLED
    void handleLogMessage(QString, QString);
#endif // LOGGING_DISABLED
Q_SIGNALS:
    void targetFileUrl(QUrl);
    void zsyncInformation(qint32,qint32,qint32,qint32,qint32,qint32,QString,QString,QString,QBuffer*,bool);
    void updateCheckInformation(QJsonObject);
    void receiveControlFile(void);
    void progress(int);
    void error(short);
    void networkError(QNetworkReply::NetworkError);
    void statusChanged(short);
    void logger(QString, QString);
private:
    bool _bAcceptRange = false;
    QJsonObject _jUpdateInformation;
    QString _sZsyncMakeVersion,
            _sZsyncFileName, //Only used for github and bintray API responses.
            _sTargetFileName,
            _sAppImagePath,
            _sTargetFileSHA1
#ifndef LOGGING_DISABLED
            ,_sLoggerName,
            _sLogBuffer;
#else
            ;
#endif // LOGGING_DISABLED
    QDateTime _pMTime;
    qint32 _nTargetFileBlockSize = 0,
           _nTargetFileLength = 0,
           _nTargetFileBlocks = 0;
    qint32 _nWeakCheckSumBytes = 0,
           _nStrongCheckSumBytes = 0,
           _nConsecutiveMatchNeeded = 0;
    qint64 _nCheckSumBlocksOffset = 0;
    QUrl _uTargetFileUrl,
         _uControlFileUrl;

#ifndef LOGGING_DISABLED
    QSharedPointer<QDebug> _pLogger = nullptr;
#endif // LOGGING_DISABLED
    QSharedPointer<QBuffer> _pControlFile = nullptr;
    QNetworkAccessManager *_pNManager = nullptr;
};
}

#endif //ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
