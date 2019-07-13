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

#include "softwareupdatedialog_p.hpp"
#include "appimageupdaterbridge_enums.hpp"
#include "appimagedeltarevisioner.hpp"

#ifndef UI_APPIMAGEUPDATERDIALOG_H
#define UI_APPIMAGEUPDATERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AppImageUpdaterDialog
{
public:
    QGridLayout *gridLayout;
    QStackedWidget *mainStack;
    QWidget *checkForUpdate;
    QGridLayout *gridLayout_3;
    QGridLayout *checkForUpdateLayout;
    QLabel *softwareIcon;
    QLabel *checkForUpdateStatusLbl;
    QWidget *updating;
    QGridLayout *gridLayout_4;
    QGridLayout *updatingLayout;
    QLabel *softwareIconOnUpdating;
    QProgressBar *progressBar;
    QLabel *updateSpeedLbl;
    QPushButton *updateCancelBtn;

    void setupUi(QDialog *AppImageUpdaterDialog)
    {
        if (AppImageUpdaterDialog->objectName().isEmpty())
            AppImageUpdaterDialog->setObjectName(QString::fromUtf8("AppImageUpdaterDialog"));
        AppImageUpdaterDialog->setWindowModality(Qt::ApplicationModal);
        AppImageUpdaterDialog->resize(432, 126);
        gridLayout = new QGridLayout(AppImageUpdaterDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        mainStack = new QStackedWidget(AppImageUpdaterDialog);
        mainStack->setObjectName(QString::fromUtf8("mainStack"));
        checkForUpdate = new QWidget();
        checkForUpdate->setObjectName(QString::fromUtf8("checkForUpdate"));
        gridLayout_3 = new QGridLayout(checkForUpdate);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        checkForUpdateLayout = new QGridLayout();
        checkForUpdateLayout->setObjectName(QString::fromUtf8("checkForUpdateLayout"));
        softwareIcon = new QLabel(checkForUpdate);
        softwareIcon->setObjectName(QString::fromUtf8("softwareIcon"));
        softwareIcon->setMinimumSize(QSize(100, 100));
        softwareIcon->setMaximumSize(QSize(100, 100));
        softwareIcon->setScaledContents(true);

        checkForUpdateLayout->addWidget(softwareIcon, 0, 0, 1, 1);

        checkForUpdateStatusLbl = new QLabel(checkForUpdate);
        checkForUpdateStatusLbl->setObjectName(QString::fromUtf8("checkForUpdateStatusLbl"));
        QFont font;
        font.setPointSize(11);
        checkForUpdateStatusLbl->setFont(font);
        checkForUpdateStatusLbl->setAlignment(Qt::AlignCenter);

        checkForUpdateLayout->addWidget(checkForUpdateStatusLbl, 0, 1, 1, 1);


        gridLayout_3->addLayout(checkForUpdateLayout, 0, 0, 1, 1);

        mainStack->addWidget(checkForUpdate);
        updating = new QWidget();
        updating->setObjectName(QString::fromUtf8("updating"));
        gridLayout_4 = new QGridLayout(updating);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        updatingLayout = new QGridLayout();
        updatingLayout->setObjectName(QString::fromUtf8("updatingLayout"));
        softwareIconOnUpdating = new QLabel(updating);
        softwareIconOnUpdating->setObjectName(QString::fromUtf8("softwareIconOnUpdating"));
        softwareIconOnUpdating->setMinimumSize(QSize(100, 100));
        softwareIconOnUpdating->setMaximumSize(QSize(100, 100));
        softwareIconOnUpdating->setScaledContents(true);

        updatingLayout->addWidget(softwareIconOnUpdating, 0, 0, 3, 1);

        progressBar = new QProgressBar(updating);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(0);

        updatingLayout->addWidget(progressBar, 0, 1, 1, 1);

        updateSpeedLbl = new QLabel(updating);
        updateSpeedLbl->setObjectName(QString::fromUtf8("updateSpeedLbl"));

        updatingLayout->addWidget(updateSpeedLbl, 1, 1, 1, 1);

        updateCancelBtn = new QPushButton(updating);
        updateCancelBtn->setObjectName(QString::fromUtf8("updateCancelBtn"));

        updatingLayout->addWidget(updateCancelBtn, 2, 1, 1, 1);


        gridLayout_4->addLayout(updatingLayout, 0, 0, 1, 1);

        mainStack->addWidget(updating);

        gridLayout->addWidget(mainStack, 0, 0, 1, 1);


        retranslateUi(AppImageUpdaterDialog);

        mainStack->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(AppImageUpdaterDialog);
    } // setupUi

    void retranslateUi(QDialog *AppImageUpdaterDialog)
    {
        AppImageUpdaterDialog->setWindowTitle(QApplication::translate("AppImageUpdaterDialog", "Updating", nullptr));
        softwareIcon->setText(QString());
        checkForUpdateStatusLbl->setText(QApplication::translate("AppImageUpdaterDialog", "Checking for Update...", nullptr));
        softwareIconOnUpdating->setText(QString());
        updateSpeedLbl->setText(QString());
        updateCancelBtn->setText(QApplication::translate("AppImageUpdaterDialog", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AppImageUpdaterDialog: public Ui_AppImageUpdaterDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_APPIMAGEUPDATERDIALOG_H

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
        NoRemindMeLaterButton = 0x80,
        NoSkipThisVersionButton = 0x100,	
	Default = ShowBeforeProgress | 
		  ShowProgressDialog |
                  ShowUpdateConfirmationDialog |
                  ShowFinishedDialog   |
                  ShowErrorDialog |
                  NotifyWhenNoUpdateIsAvailable |
		  NoRemindMeLaterButton |
		  NoSkipThisVersionButton
    };

    AppImageUpdaterDialog(QPixmap img = QPixmap(),
                          QWidget *parent = nullptr, int flags = Default);
    ~AppImageUpdaterDialog();

public Q_SLOTS:
    void init(AppImageDeltaRevisioner *revisioner = nullptr ,
	      const QString &applicationName = QApplication::applicationName());
private Q_SLOTS:
    void doInit(QObject*, const QString&);

    void showWidget(void);
    void handleRejected(void);
    void doUpdate(void);
    void handleUpdateAvailable(bool, QJsonObject);
    void handleError(short);
    void handleFinished(QJsonObject, QString);
    void handleProgress(int, qint64, qint64, double, QString);
    void resetConnections();

Q_SIGNALS:
    void quit(void);
    void started(void);
    void canceled(void);
    void error(QString, short);
    void finished(QJsonObject);
    void requiresAuthorization(QString, short, QString);

private:
    bool b_Busy = false;
    int p_Flags = 0;
    QString m_ApplicationName;
    QString s_CurrentAppImagePath; /* Used only for error dialog box. */
    SoftwareUpdateDialog *m_ConfirmationDialog;
    AppImageDeltaRevisioner *p_DRevisioner = nullptr;
    double n_MegaBytesTotal = 0;
    const QString progressTemplate = QString::fromUtf8("Updating %1 MiB of %2 MiB at %3 %4...");
    Ui::AppImageUpdaterDialog m_Ui;
};
}

#endif // APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
