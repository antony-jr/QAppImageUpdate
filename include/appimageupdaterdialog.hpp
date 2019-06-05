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
 * @filename    : appimageupdaterdialog.hpp
 * @description : The description of the GUI Updater dialog.
*/
#ifndef APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
#define APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
#include <QPixmap>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QMutex>
#include <QNetworkProxy>
#include <QMessageBox>
#include <QJsonObject>

#include "ui_AppImageUpdaterDialog.h"
#include "appimageupdaterbridge_enums.hpp"
#include "appimagedeltarevisioner.hpp"

namespace AppImageUpdaterBridge
{
class AppImageUpdaterDialog : public QDialog
{
    Q_OBJECT
public:
    enum {
        ShowProgressDialog = 0x1,
        ShowBeforeProgress = 0x2,
        ShowUpdateConfirmationDialog = 0x4,
        ShowFinishedDialog = 0x8,
        ShowErrorDialog = 0x10,
        AlertWhenAuthorizationIsRequired = 0x20,
        NotifyWhenNoUpdateIsAvailable = 0x40,
        Default = ShowBeforeProgress | 
		  ShowProgressDialog |
                  ShowUpdateConfirmationDialog |
                  ShowFinishedDialog   |
                  ShowErrorDialog |
                  NotifyWhenNoUpdateIsAvailable
    };

    AppImageUpdaterDialog(QPixmap img = QPixmap(),
                          QWidget *parent = nullptr, int flags = Default,
			  AppImageDeltaRevisioner *revisioner = nullptr);

    AppImageUpdaterDialog(const QString&,
                          QPixmap img = QPixmap(),
                          QWidget *parent = nullptr, int flags = Default,
			  AppImageDeltaRevisioner *revisioner = nullptr);
    AppImageUpdaterDialog(QFile*,
                          QPixmap img = QPixmap(),
                          QWidget *parent = nullptr, int flags = Default,
			  AppImageDeltaRevisioner *revisioner = nullptr);
    ~AppImageUpdaterDialog();

public Q_SLOTS:
    void init();
    void setCustomUpdateConfirmationDialog(QDialog*);
    void setAppImage(const QString&);
    void setAppImage(QFile *);
    void setShowLog(bool);
    void setProxy(const QNetworkProxy&);

private Q_SLOTS:
    void showWidget(void);
    void handleCustomConfirmUpdate();
    void handleCustomConfirmNoUpdate();
    void handleCustomUpdateAvailable(bool , QJsonObject);
    void handleUpdateAvailable(bool, QJsonObject);
    void handleError(short);
    void handleFinished(QJsonObject, QString);
    void handleProgress(int, qint64, qint64, double, QString);

Q_SIGNALS:
    void quit(void);
    void started(void);
    void canceled(void);
    void error(QString, short);
    void finished(QJsonObject);
    void requiresAuthorization(QString, short, QString);
    void updateAvailable(bool , QJsonObject);

private:
    int p_Flags = 0;
    QPixmap p_AppImageIcon;
    QString s_CurrentAppImagePath; /* Used only for error dialog box. */
    AppImageDeltaRevisioner *p_DRevisioner = nullptr;
    double n_MegaBytesTotal = 0;
    const QString progressTemplate = QString::fromUtf8("Updating %1 MiB of %2 MiB at %3 %4...");

    Ui::AppImageUpdaterDialog m_Ui;
};
}

#endif // APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
