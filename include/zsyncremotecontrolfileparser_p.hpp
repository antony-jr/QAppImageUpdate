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
 * @filename    : zsyncremotecontrolfileparser_p.hpp
 * @description : This is where the ZsyncRemoteControlFileParserPrivate in described.
 * This class is responsible to parse the remote zsync control file from the given
 * embeded appimage information.
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
#include <QScopedPointer>
#include <QTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "appimageupdaterbridge_enums.hpp"
#include "zsyncinternalstructures_p.hpp"

namespace AppImageUpdaterBridge
{
class ZsyncRemoteControlFileParserPrivate : public QObject
{
    Q_OBJECT
public:
    ZsyncRemoteControlFileParserPrivate(QNetworkAccessManager*);
    ~ZsyncRemoteControlFileParserPrivate();
public Q_SLOTS:
    void clear(void);
    void setControlFileUrl(const QUrl&);
    void setControlFileUrl(QJsonObject);
    void setLoggerName(const QString&);
    void setShowLog(bool);
    void getControlFile(void);
    void getUpdateCheckInformation(void);
    void getZsyncInformation(void);
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
    void zsyncInformation(qint32,qint32,qint32,
                          qint32,qint32,qint32,
                          QString,QString,QString,
                          QUrl,QBuffer*,bool);
    void updateCheckInformation(QJsonObject);
    void receiveControlFile(void);
    void progress(int);
    void error(short);
    void statusChanged(short);
    void logger(QString, QString);
private:
    bool b_AcceptRange = false,
         b_Busy = false;
    QJsonObject j_UpdateInformation;
    QString s_ZsyncMakeVersion,
            s_ZsyncFileName, /* only used for github transport. */
            s_TargetFileName,
            s_AppImagePath,
            s_TargetFileSHA1
#ifndef LOGGING_DISABLED
            ,s_LoggerName,
            s_LogBuffer;
#else
            ;
#endif // LOGGING_DISABLED
    QDateTime m_MTime;
    qint32 n_TargetFileBlockSize = 0,
           n_TargetFileLength = 0,
           n_TargetFileBlocks = 0;
    qint32 n_WeakCheckSumBytes = 0,
           n_StrongCheckSumBytes = 0,
           n_ConsecutiveMatchNeeded = 0;
    qint64 n_CheckSumBlocksOffset = 0;
    QUrl u_TargetFileUrl,
         u_ControlFileUrl;

#ifndef LOGGING_DISABLED
    QScopedPointer<QDebug> p_Logger;
#endif // LOGGING_DISABLED
    QScopedPointer<QBuffer> p_ControlFile;
    QNetworkAccessManager *p_NManager = nullptr;
};
}

#endif //ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
