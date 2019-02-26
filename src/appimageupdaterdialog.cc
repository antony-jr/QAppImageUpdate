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
 * @filename    : appimageupdaterdialog.cc
 * @description : The implementation of the GUI Updater dialog.
*/
#include "../include/appimageupdaterdialog.hpp"

using namespace AppImageUpdaterBridge;

AppImageUpdaterDialog::AppImageUpdaterDialog(QPixmap img,QWidget *parent, int flags)
    : QWidget(parent),
      p_Flags(flags)
{
    if (objectName().isEmpty())
        setObjectName(QStringLiteral("AppImageUpdaterDialog"));
    resize(420, 120);
    /* Fixed window size. */
    setMinimumSize(QSize(420, 120));
    setMaximumSize(QSize(420, 120));

    p_GridLayout = new QGridLayout(this);
    p_GridLayout->setObjectName(QStringLiteral("MainGridLayout"));

    /* Cancel Update. */
    p_CancelBtn = new QPushButton(this);
    p_CancelBtn->setObjectName(QStringLiteral("CancelButton"));
    p_GridLayout->addWidget(p_CancelBtn, 2, 1, 1, 1);

    /* Update Status. */
    p_StatusLbl = new QLabel(this);
    p_StatusLbl->setObjectName(QStringLiteral("StatusLabel"));
    p_GridLayout->addWidget(p_StatusLbl, 1, 1, 1, 1);

    /* Update Progress. */
    p_ProgressBar = new QProgressBar(this);
    p_ProgressBar->setObjectName(QStringLiteral("ProgressBar"));
    p_ProgressBar->setValue(0);
    p_GridLayout->addWidget(p_ProgressBar, 0, 1, 1, 1);

    /* Set AppImage icon if given. */
    if(!img.isNull()) {
        p_IconLbl = new QLabel(this);
        p_IconLbl->setObjectName(QStringLiteral("IconLabel"));
        p_IconLbl->setMinimumSize(QSize(100, 100));
        p_IconLbl->setMaximumSize(QSize(100, 100));
        p_IconLbl->setScaledContents(true);
        p_IconLbl->setAlignment(Qt::AlignCenter);
        p_GridLayout->addWidget(p_IconLbl, 0, 0, 3, 1);
        p_IconLbl->setPixmap(img);
        p_AppImageIcon = img.scaled(100, 100, Qt::KeepAspectRatio);
        setWindowIcon(img);
    }

    /* Delta Revisioner. */
    p_DRevisioner = new AppImageDeltaRevisioner(/*single threaded=*/false, /*parent=*/this);

    /* Translations. */
    setWindowTitle(QString::fromUtf8("Updating... "));
    p_CancelBtn->setText(QString::fromUtf8("Cancel"));

    /* Program Logic. */
    connect(p_CancelBtn, &QPushButton::pressed, p_DRevisioner, &AppImageDeltaRevisioner::cancel);
    connect(p_DRevisioner, &AppImageDeltaRevisioner::canceled, this, &QDialog::hide);
    connect(p_DRevisioner, &AppImageDeltaRevisioner::canceled, this, &AppImageUpdaterDialog::canceled, Qt::DirectConnection);
    connect(p_DRevisioner, &AppImageDeltaRevisioner::updateAvailable, this, &AppImageUpdaterDialog::handleUpdateAvailable);
    connect(p_DRevisioner, &AppImageDeltaRevisioner::error, this, &AppImageUpdaterDialog::handleError);
    connect(p_DRevisioner, &AppImageDeltaRevisioner::started, this, &AppImageUpdaterDialog::started, Qt::DirectConnection);
    connect(p_DRevisioner, &AppImageDeltaRevisioner::finished, this, &AppImageUpdaterDialog::handleFinished);
    connect(p_DRevisioner, &AppImageDeltaRevisioner::progress, this, &AppImageUpdaterDialog::handleProgress);
    return;
}

AppImageUpdaterDialog::AppImageUpdaterDialog(const QString &AppImagePath, QPixmap img, QWidget *parent, int p_Flags)
    : AppImageUpdaterDialog(img, parent, p_Flags)
{
    p_DRevisioner->setAppImage(AppImagePath);
}

AppImageUpdaterDialog::AppImageUpdaterDialog(QFile *AppImage,QPixmap img,QWidget *parent, int p_Flags)
    : AppImageUpdaterDialog(img, parent, p_Flags)
{
    p_DRevisioner->setAppImage(AppImage);
}

AppImageUpdaterDialog::~AppImageUpdaterDialog()
{
    return;
}

void AppImageUpdaterDialog::init(void)
{
    n_MegaBytesTotal = 0;
    p_StatusLbl->setText(QString::fromUtf8("Checking for Update... "));
    p_DRevisioner->checkForUpdate();
    p_DRevisioner->setShowLog(true);
    if(p_Flags & ShowBeforeProgress) {
        showWidget();
    }
    return;
}

void AppImageUpdaterDialog::setAppImage(const QString &AppImagePath)
{
    p_DRevisioner->setAppImage(AppImagePath);
}

void AppImageUpdaterDialog::setAppImage(QFile *AppImage)
{
    p_DRevisioner->setAppImage(AppImage);
}

void AppImageUpdaterDialog::setShowLog(bool c)
{
    p_DRevisioner->setShowLog(c);
}

void AppImageUpdaterDialog::showWidget(void)
{
    if(!(p_Flags & ShowProgressDialog)) {
        return;
    }
    show();
    return;
}

void AppImageUpdaterDialog::handleUpdateAvailable(bool isUpdateAvailable, QJsonObject CurrentAppImageInfo)
{
    bool confirmed = true;
    bool showUpdateDialog = p_Flags & ShowUpdateConfirmationDialog;
    bool showNoUpdateDialog = p_Flags & NotifyWhenNoUpdateIsAvailable;
    QMessageBox box(this);
    setWindowTitle(QString::fromUtf8("Updating ") +
                         QFileInfo(CurrentAppImageInfo["AppImageFilePath"].toString()).baseName() +
                         QString::fromUtf8("... "));

    s_CurrentAppImagePath = CurrentAppImageInfo["AppImageFilePath"].toString();

    if(isUpdateAvailable) {
        if(showUpdateDialog) {
            QString currentAppImageName = QFileInfo(CurrentAppImageInfo["AppImageFilePath"].toString()).fileName();
            QMessageBox box(this);
            box.setWindowTitle(QString::fromUtf8("Update Available!"));
            box.setText(QString::fromUtf8("A new version of ") +
                        currentAppImageName +
                        QString::fromUtf8(" is available , Do you want to update ?"));
            box.addButton(QMessageBox::Yes);
            box.addButton(QMessageBox::No);
            confirmed = (box.exec() == QMessageBox::Yes);
        }
    } else {
        if(showNoUpdateDialog) {
            QString currentAppImageName = QFileInfo(CurrentAppImageInfo["AppImageFilePath"].toString()).fileName();
            QMessageBox box(this);
            box.setWindowTitle(QString::fromUtf8("No Updates Available!"));
            box.setText(QString::fromUtf8("You are currently using the lastest version of ") +
                        currentAppImageName +
                        QString::fromUtf8("."));
            box.exec();
        }
        confirmed = false;
        emit finished(QJsonObject());
    }

    /*
     * If confirmed to update then start the delta revisioner.
    */
    if(confirmed) {
        p_DRevisioner->start();
        showWidget();
    } else {
        emit finished(QJsonObject());
    }
    return;
}

void AppImageUpdaterDialog::handleError(short errorCode)
{
    bool show = p_Flags & ShowErrorDialog,
         alert = p_Flags &  AlertWhenAuthorizationIsRequired,
         doAlert = false;
    QString path = s_CurrentAppImagePath,
            errorString;

    switch(errorCode) {
    case ConnectionRefusedError:
        errorString = QString::fromUtf8("the update server is not accepting requests.");
        break;
    case RemoteHostClosedError:
        errorString = QString::fromUtf8("the remote server closed the connection prematurely, ");
        errorString += QString::fromUtf8("before the entire reply was received and processed.");
        break;
    case HostNotFoundError:
        errorString = QString::fromUtf8("the remote host name was not found (invalid hostname).");
        break;
    case TimeoutError:
        errorString = QString::fromUtf8("the connection to the remote server timed out.");
        break;
    case SslHandshakeFailedError:
        errorString = QString::fromUtf8("the SSL/TLS handshake failed and the encrypted channel ");
        errorString += QString::fromUtf8("could not be established.");
        break;
    case TemporaryNetworkFailureError:
        errorString = QString::fromUtf8("the connection was broken due to disconnection from the network,");
        break;
    case NetworkSessionFailedError:
        errorString = QString::fromUtf8("the connection was broken due to disconnection from the network ");
        errorString += QString::fromUtf8("or failure to start the network.");
        break;
    case BackgroundRequestNotAllowedError:
        errorString = QString::fromUtf8("the background request is not currently allowed due to platform policy.");
        break;
    case TooManyRedirectsError:
        errorString = QString::fromUtf8("while following redirects, the maximum limit was reached. ");
        break;
    case InsecureRedirectError:
        errorString = QString::fromUtf8("while following redirects, the network access API detected a redirect ");
        errorString += QString::fromUtf8("from a encrypted protocol (https) to an unencrypted one (http).");
        break;
    case ContentAccessDenied:
        errorString = QString::fromUtf8("the access to the remote content was denied (HTTP error 403).");
        break;
    case ContentOperationNotPermittedError:
        errorString = QString::fromUtf8("the operation requested on the remote content is not permitted.");
        break;
    case ContentNotFoundError:
        errorString = QString::fromUtf8("the remote content was not found at the server (HTTP error 404)");
        break;
    case AuthenticationRequiredError:
        errorString = QString::fromUtf8("the remote server requires authentication to serve the content ");
        errorString += QString::fromUtf8("but the credentials provided were not accepted or given. ");
        break;
    case ContentConflictError:
        errorString = QString::fromUtf8("the request could not be completed due to a conflict with the ");
        errorString += QString::fromUtf8("current state of the resource.");
        break;
    case ContentGoneError:
        errorString = QString::fromUtf8("the requested resource is no longer available at the server.");
        break;
    case InternalServerError:
        errorString = QString::fromUtf8("the server encountered an unexpected condition which prevented ");
        errorString += QString::fromUtf8("it from fulfilling the request.");
        break;
    case OperationNotImplementedError:
        errorString = QString::fromUtf8("the server does not support the functionality required to fulfill the request.");
        break;
    case ServiceUnavailableError:
        errorString = QString::fromUtf8("the server is unable to handle the request at this time.");
        break;
    case ProtocolUnknownError:
        errorString = QString::fromUtf8("the Network Access API cannot honor the request because the protocol");
        errorString += QString::fromUtf8(" is not known.");
        break;
    case ProtocolInvalidOperationError:
        errorString = QString::fromUtf8("the requested operation is invalid for this protocol.");
        break;
    case UnknownNetworkError:
        errorString = QString::fromUtf8("an unknown network-related error was detected.");
        break;
    case UnknownContentError:
        errorString = QString::fromUtf8("an unknown error related to the remote content was detected.");
        break;
    case ProtocolFailure:
        errorString = QString::fromUtf8("a breakdown in protocol was detected ");
        errorString += QString::fromUtf8("(parsing error, invalid or unexpected responses, etc.)");
        break;
    case UnknownServerError:
        errorString = QString::fromUtf8("an unknown error related to the server response was detected.");
        break;
    case NoAppimagePathGiven:
        errorString = QString::fromUtf8("no appimage was given.");
        break;
    case AppimageNotReadable:
        errorString = QString::fromUtf8("it is not readable.");
        break;
    case NoReadPermission:
        errorString = QString::fromUtf8("you don't have the permission to read it.");
        show = (alert) ? false : show;
        doAlert = alert;
        break;
    case AppimageNotFound:
        errorString = QString::fromUtf8("it does not exists.");
        break;
    case CannotOpenAppimage:
        errorString = QString::fromUtf8("it cannot be opened.");
        break;
    case EmptyUpdateInformation:
        errorString = QString::fromUtf8("the author of this AppImage did not include any update information.");
        break;
    case InvalidAppimageType:
        errorString = QString::fromUtf8("it is an unknown AppImage type.");
        break;
    case InvalidMagicBytes:
        errorString = QString::fromUtf8("it is not a AppImage , Make sure to give valid AppImage.");
        break;
    case InvalidUpdateInformation:
        errorString = QString::fromUtf8("the author of this AppImage included invalid update information.");
        break;
    case NotEnoughMemory:
        errorString = QString::fromUtf8("there is no enough memory left in your ram.");
        break;
    case SectionHeaderNotFound:
        errorString = QString::fromUtf8("the author of this AppImage did not embed the update information ");
        errorString += QString::fromUtf8("at a valid section header.");
        break;
    case UnsupportedElfFormat:
        errorString = QString::fromUtf8("the given AppImage is not in supported elf format.");
        break;
    case UnsupportedTransport:
        errorString = QString::fromUtf8("the author of this included an unsupported transport in update information.");
        break;
    case IoReadError:
        errorString = QString::fromUtf8("there was a unknown IO read error.");
        break;
    case GithubApiRateLimitReached:
        errorString = QString::fromUtf8("github api rate limit was reached , Please try again after an hour.");
        break;
    case ErrorResponseCode:
        errorString = QString::fromUtf8("some request returned a bad server response code , Please try again.");
        break;
    case NoMarkerFoundInControlFile:
    case InvalidZsyncHeadersNumber:
    case InvalidZsyncMakeVersion:
    case InvalidZsyncTargetFilename:
    case InvalidZsyncMtime:
    case InvalidZsyncBlocksize:
    case InvalidTargetFileLength:
    case InvalidHashLengthLine:
    case InvalidHashLengths:
    case InvalidTargetFileUrl:
    case InvalidTargetFileSha1:
    case HashTableNotAllocated:
    case InvalidTargetFileChecksumBlocks:
    case CannotOpenTargetFileChecksumBlocks:
    case CannotConstructHashTable:
    case QbufferIoReadError:
        errorString = QString::fromUtf8("the author did not produce the zsync meta file properly , ");
        errorString += QString::fromUtf8("Please notify the author or else try again because it can be a ");
        errorString += QString::fromUtf8("false positive.");
        break;
    case SourceFileNotFound:
        errorString = QString::fromUtf8("the current AppImage is not found , maybe deleted while updating ?");
        break;
    case NoPermissionToReadSourceFile:
        errorString = QString::fromUtf8("you have no permissions to read the current AppImage.");
        show = (alert) ? false : show;
        doAlert = alert;
        break;
    case CannotOpenSourceFile:
        errorString = QString::fromUtf8("the current AppImage cannot be opened.");
        break;
    case NoPermissionToReadWriteTargetFile:
        errorString = QString::fromUtf8("you have no write or read permissions to read and write the new version.");
        show = (alert) ? false : show;
        doAlert = alert;
        break;
    case CannotOpenTargetFile:
        errorString = QString::fromUtf8("the new version cannot be opened to write or read.");
        break;
    case TargetFileSha1HashMismatch:
        errorString = QString::fromUtf8("the newly construct AppImage failed to prove integrity , Try again.");
        break;
    default:
        errorString = QString::fromUtf8("there occured an uncaught error.");
        break;
    }

    hide();

    if(show) {
        QMessageBox box(this);
        box.setWindowTitle(QString::fromUtf8("Update Failed!"));
        box.setIcon(QMessageBox::Critical);
        box.setText(QString::fromUtf8("Update failed for '") +
                    path +
                    QString::fromUtf8("' , Because ") +
                    errorString);
        box.exec();
    }

    if(doAlert) {
        emit requiresAuthorization(errorString, errorCode, path);
    } else {
        emit error(errorString, errorCode);
    }
    return;
}

void AppImageUpdaterDialog::handleFinished(QJsonObject newVersion, QString oldVersionPath)
{
    (void)oldVersionPath;
    p_StatusLbl->setText(QString::fromUtf8("Finalizing Update... "));

    bool execute = false;
    bool show = p_Flags & ShowFinishedDialog;

    if(show) {
        QString currentAppImageName = QFileInfo(oldVersionPath).fileName();
        QMessageBox box(this);
        box.setWindowTitle(QString::fromUtf8("Update Completed!"));
        box.setIconPixmap(p_AppImageIcon);
        box.setText(QString::fromUtf8("Update Completed successfully for ") +
                    currentAppImageName +
                    QString::fromUtf8(" , the new version is saved at '") +
                    newVersion["AbsolutePath"].toString() +
                    QString::fromUtf8("' , Do you want to open it ?"));
        box.addButton(QMessageBox::Yes);
        box.addButton(QMessageBox::No);
        execute = (box.exec() == QMessageBox::Yes);
    }

    if(execute) {
        QFileInfo info(newVersion["AbsolutePath"].toString());
        if(!info.isExecutable()) {
            {
                QFile file(newVersion["AbsolutePath"].toString());
                file.setPermissions(QFileDevice::ExeUser |
                                    QFileDevice::ExeOther|
                                    QFileDevice::ExeGroup|
                                    info.permissions());
            }
        }
        QProcess::startDetached(newVersion["AbsolutePath"].toString());
        emit quit();
    }
    this->hide();
    emit finished(newVersion);
    return;
}

void AppImageUpdaterDialog::handleProgress(int percent,
        qint64 bytesReceived,
        qint64 bytesTotal,
        double speed,
        QString units)
{
    p_ProgressBar->setValue(percent);
    double MegaBytesReceived = bytesReceived / 1048576;
    if(!n_MegaBytesTotal) {
        n_MegaBytesTotal = bytesTotal / 1048576;
    }
    const QString progressTemplate = QString::fromUtf8("Updating %1 MiB of %2 MiB at %3 %4...");
    QString statusText = progressTemplate.arg(MegaBytesReceived).arg(n_MegaBytesTotal).arg(speed).arg(units);
    p_StatusLbl->setText(statusText);
    return;
}


