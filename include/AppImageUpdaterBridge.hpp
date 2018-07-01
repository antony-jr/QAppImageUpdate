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
 * @filename           : AIUpdaterBridge.hpp
 *
*/
#if !defined(AIUPDATER_BRIDGE_HPP_INCLUDED)
#define AIUPDATER_BRIDGE_HPP_INCLUDED
#include <QtCore>
#include <QtConcurrentRun>
#include <QFuture>
#include <QAIUpdateInformation>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace AppImageUpdaterBridge {
class AppImageUpdaterBridge : public QObject {
	Q_OBJECT
public:
    enum : short {

    } error_code;

    enum : short {
    
    } status_code;

    explicit AppImageUpdaterBridge(QObject *parent = nullptr);
    explicit AppImageUpdaterBridge(const QString& , QObject *parent = nullptr);
    explicit AppImageUpdaterBridge(QFile * , QObject *parent = nullptr);
    AppImageUpdaterBridge &setAppImage(const QString&);
    AppImageUpdaterBridge &setAppImage(QFile *);
    AppImageUpdaterBridge &setShowLog(bool);
    ~AppImageUpdaterBridge();

public Q_SLOTS:
    AppImageUpdaterBridge &waitForFinished(void);
    AppImageUpdaterBridge &start(void);
    AppImageUpdaterBridge &cancel(void);
    AppImageUpdaterBridge &pause(void);
    AppImageUpdaterBridge &resume(void);

    bool isRunning() const;
    bool isStarted() const;
    bool isCanceled() const;
    bool isPaused() const;
    bool isFinished() const;

Q_SIGNALS:
    void started(void);
    void finished(void);
    void canceled(void);
    void paused(void);
    void resumed(void);
    void progress(int);
    void logger(QString);
    void error(short);
private:
    QAIUpdateInformation _pAIUInformation;
};
}
#endif // AIUPDATER_BRIDGE_HPP_INCLUDED
