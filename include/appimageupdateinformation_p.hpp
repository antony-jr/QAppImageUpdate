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
 * @filename    : appimageupdateinformation_p.hpp
 * @description : This is where the extraction of embeded update information
 * from AppImages is described.
*/
#ifndef APPIMAGE_UPDATE_INFORMATION_PRIVATE_HPP_INCLUDED
#define APPIMAGE_UPDATE_INFORMATION_PRIVATE_HPP_INCLUDED
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "appimageupdaterbridge_enums.hpp"

namespace AppImageUpdaterBridge
{
class AppImageUpdateInformationPrivate : public QObject
{
    Q_OBJECT
public:
    AppImageUpdateInformationPrivate(QObject *parent = nullptr);
    ~AppImageUpdateInformationPrivate();
public Q_SLOTS:
    void setAppImage(const QString&);
    void setAppImage(QFile *);
    void setShowLog(bool);
    void setLoggerName(const QString&);
    void getInfo(void);
    void clear(void);

#ifndef LOGGING_DISABLED
private Q_SLOTS:
    void handleLogMessage(QString, QString);
#endif // LOGGING_DISABLED

Q_SIGNALS:
    void info(QJsonObject);
    void progress(int);
    void error(short);
    void statusChanged(short);
    void logger(QString, QString);

private:
    bool b_Busy = false;
    QJsonObject m_Info;
    QString s_AppImageName, /* cache to avoid the overhead for QFileInfo. */
            s_AppImagePath,
#ifndef LOGGING_DISABLED
            s_LogBuffer,
            s_LoggerName,
#endif // LOGGING_DISABLED
            s_AppImageSHA1;
#ifndef LOGGING_DISABLED
    QScopedPointer<QDebug> p_Logger;
#endif // LOGGING_DISABLED
    QFile *p_AppImage = nullptr;
};
}
#endif // APPIMAGE_UPDATE_INFORMATION_PRIVATE_HPP_INCLUDED
