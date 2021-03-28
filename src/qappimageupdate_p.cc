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
*/

#include <QCoreApplication>

#include "qappimageupdate_p.hpp"
#include "helpers_p.hpp"

#ifndef NO_GUI
#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QScreen>
#include <QPixmap>

#include "softwareupdatedialog_p.hpp"
#include <ui_AppImageUpdaterDialog.h>
#endif // NOT NO_GUI


QAppImageUpdatePrivate::QAppImageUpdatePrivate(bool singleThreaded, QObject *parent)
    : QObject(parent) {
    setObjectName("QAppImageUpdatePrivate");
    if(!singleThreaded) {
        m_SharedThread.reset(new QThread);
        m_SharedThread->start();
    }


    m_SharedNetworkAccessManager.reset(new QNetworkAccessManager);
    m_UpdateInformation.reset(new AppImageUpdateInformationPrivate);
    m_DeltaWriter.reset(new ZsyncWriterPrivate(m_SharedNetworkAccessManager.data()));
#ifdef DECENTRALIZED_UPDATE_ENABLED
    m_Seeder.reset(new Seeder(m_SharedNetworkAccessManager.data()));
#endif

    if(!singleThreaded) {
        m_SharedNetworkAccessManager->moveToThread(m_SharedThread.data());
        m_UpdateInformation->moveToThread(m_SharedThread.data());
        m_DeltaWriter->moveToThread(m_SharedThread.data());
    }
    m_ControlFileParser.reset(new ZsyncRemoteControlFileParserPrivate(m_SharedNetworkAccessManager.data()));
    if(!singleThreaded) {
        m_ControlFileParser->moveToThread(m_SharedThread.data());
    }
    m_ControlFileParser->setObjectName("ZsyncRemoteControlFileParserPrivate");
    m_UpdateInformation->setObjectName("AppImageUpdateInformationPrivate");
    m_DeltaWriter->setObjectName("ZsyncWriterPrivate");

    // Set logger name
    m_UpdateInformation->setLoggerName("UpdateInformation");
    m_ControlFileParser->setLoggerName("ControlFileParser");
    m_DeltaWriter->setLoggerName("DeltaWriter");


    // Update information
    connect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::logger,
            this, &QAppImageUpdatePrivate::logger,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
    connect(m_UpdateInformation.data(), SIGNAL(operatingAppImagePath(QString)),
            this, SLOT(setCurrentAppImagePath(QString)),
            Qt::QueuedConnection);


    // Control file parsing
    connect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::logger,
            this, &QAppImageUpdatePrivate::logger,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));

    // Delta Writer and Downloader
    connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::logger,
            this, &QAppImageUpdatePrivate::logger,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
    connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::started,
            this, &QAppImageUpdatePrivate::handleUpdateStart,
   	    (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

    // Torrent Seeder
#ifdef DECENTRALIZED_UPDATE_ENABLED 
    connect(m_Seeder.data(), &Seeder::started,
            this, &QAppImageUpdatePrivate::torrentClientStarted ,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
    connect(m_Seeder.data(), &Seeder::torrentStatus,
            this, &QAppImageUpdatePrivate::torrentStatus,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
#endif

    // Torrent Downloader Specific
    connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::torrentClientStarted,
            this, &QAppImageUpdatePrivate::torrentClientStarted ,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
    connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::torrentStatus,
            this, &QAppImageUpdatePrivate::torrentStatus,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
}

QAppImageUpdatePrivate::QAppImageUpdatePrivate(const QString &AppImagePath, bool singleThreaded, QObject *parent)
    : QAppImageUpdatePrivate(singleThreaded, parent) {
    setAppImage(AppImagePath);
    return;
}

QAppImageUpdatePrivate::QAppImageUpdatePrivate(QFile *AppImage, bool singleThreaded, QObject *parent)
    : QAppImageUpdatePrivate(singleThreaded, parent) {
    setAppImage(AppImage);
    return;
}

QAppImageUpdatePrivate::~QAppImageUpdatePrivate() {
    if(b_Started || b_Running) {
        cancel();
    }

    if(!m_SharedThread.isNull()) {
        m_SharedThread->quit();
        m_SharedThread->wait();
    }
    return;
}

void QAppImageUpdatePrivate::setApplicationName(const QString &applicationName) {
    if(b_Started || b_Running) {
        return;
    }
    m_ApplicationName = applicationName;
}

void QAppImageUpdatePrivate::setIcon(QByteArray icon) {
    if(b_Started || b_Running) {
        return;
    }

    m_Icon = icon;
}

void QAppImageUpdatePrivate::setGuiFlag(int flag) {
    if(b_Started || b_Running) {
        return;
    }

    n_GuiFlag = flag;
}

void QAppImageUpdatePrivate::setAppImage(const QString &AppImagePath) {
    if(b_Started || b_Running) {
        return;
    }

    clear();
    getMethod(m_UpdateInformation.data(), "setAppImage(const QString&)")
    .invoke(m_UpdateInformation.data(),
            Qt::QueuedConnection,
            Q_ARG(QString,AppImagePath));
    return;
}

void QAppImageUpdatePrivate::setAppImage(QFile *AppImage) {
    if(b_Started || b_Running) {
        return;
    }

    clear();
    getMethod(m_UpdateInformation.data(), "setAppImage(QFile *)")
    .invoke(m_UpdateInformation.data(),
            Qt::QueuedConnection,
            Q_ARG(QFile*,AppImage));
}

void QAppImageUpdatePrivate::setShowLog(bool choice) {
    if(b_Started || b_Running) {
        return;
    }

    getMethod(m_UpdateInformation.data(), "setShowLog(bool)")
    .invoke(m_UpdateInformation.data(),
            Qt::QueuedConnection,
            Q_ARG(bool, choice));

    getMethod(m_ControlFileParser.data(), "setShowLog(bool)")
    .invoke(m_ControlFileParser.data(),
            Qt::QueuedConnection,
            Q_ARG(bool, choice));

    getMethod(m_DeltaWriter.data(), "setShowLog(bool)")
    .invoke(m_DeltaWriter.data(),
            Qt::QueuedConnection,
            Q_ARG(bool, choice));
}

void QAppImageUpdatePrivate::setOutputDirectory(const QString &dir) {
    if(b_Started || b_Running) {
        return;
    }

    getMethod(m_DeltaWriter.data(), "setOutputDirectory(const QString&)")
    .invoke(m_DeltaWriter.data(),
            Qt::QueuedConnection,
            Q_ARG(QString, dir));
    return;
}

void QAppImageUpdatePrivate::setProxy(const QNetworkProxy &proxy) {
    if(b_Started || b_Running) {
        return;
    }
    m_SharedNetworkAccessManager->setProxy(proxy);
    return;
}

void QAppImageUpdatePrivate::clear(void) {
    if(b_Started || b_Running) {
        return;
    }

    b_Started = b_Running = b_Finished = b_Canceled = b_CancelRequested = false;
    getMethod(m_UpdateInformation.data(), "clear(void)")
    .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);
    getMethod(m_ControlFileParser.data(), "clear(void)")
    .invoke(m_ControlFileParser.data(), Qt::QueuedConnection);
    return;
}


void QAppImageUpdatePrivate::start(short action, int flags, QByteArray icon) {
    if(b_Started || b_Running) {
        return;
    }

    b_Started = b_Running = true;
    b_Canceled = false;
    b_Finished = false;

    if(b_CancelRequested) {
        b_Started = b_Running = false;
        b_Canceled = true;
        b_CancelRequested = false;
        emit canceled(action);
        return;
    }

    if(flags == GuiFlag::None) {
        flags = (n_GuiFlag != GuiFlag::None) ? n_GuiFlag : GuiFlag::Default;
    }
    n_GuiFlag = flags;

    if(icon.isEmpty() && !m_Icon.isEmpty()) {
        icon = m_Icon;
    }

    if(action == Action::GetEmbeddedInfo) {
        n_CurrentAction = action;
        connect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::info,
                this, &QAppImageUpdatePrivate::redirectEmbeddedInformation,
                Qt::QueuedConnection);
        connect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::progress,
                this, &QAppImageUpdatePrivate::handleGetEmbeddedInfoProgress);
        connect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::error,
                this, &QAppImageUpdatePrivate::handleGetEmbeddedInfoError,
                Qt::QueuedConnection);

        emit started(Action::GetEmbeddedInfo);
        getMethod(m_UpdateInformation.data(), "getInfo(void)")
        .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);

    } else if(action == Action::CheckForUpdate) {
        n_CurrentAction = action;

        //// Needed to check if Bittorrent file is
        //// supported.
#ifdef DECENTRALIZED_UPDATE_ENABLED
        m_ControlFileParser->setUseBittorrent(true);
#else
        m_ControlFileParser->setUseBittorrent(false);
#endif
        connect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
                m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
                m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
                this, SLOT(redirectUpdateCheck(QJsonObject)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(progress(int)),
                this, SLOT(handleCheckForUpdateProgress(int)));

        connect(m_ControlFileParser.data(), SIGNAL(error(short)),
                this, SLOT(handleCheckForUpdateError(short)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_UpdateInformation.data(), SIGNAL(error(short)),
                this, SLOT(handleCheckForUpdateError(short)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));


        emit started(Action::CheckForUpdate);
        getMethod(m_UpdateInformation.data(), "getInfo(void)")
        .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);
    } else if(action == Action::Update || action == Action::UpdateWithTorrent) {
        n_CurrentAction = action;

        //// With respect to GDPR, It is strongly adviced
        //// to use Torrent only if the user explicitly
        //// asks for it.
#ifdef DECENTRALIZED_UPDATE_ENABLED
        m_ControlFileParser->setUseBittorrent((action == Action::UpdateWithTorrent));
#else
        m_ControlFileParser->setUseBittorrent(false);
#endif
        connect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
                m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
                m_ControlFileParser.data(), SLOT(getZsyncInformation(void)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
                m_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration,
                (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

        connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
                m_DeltaWriter.data(), &ZsyncWriterPrivate::start,
                (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

        connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finished,
                this, &QAppImageUpdatePrivate::handleUpdateFinished,
                (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

        connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::error,
                this, &QAppImageUpdatePrivate::handleUpdateError,
                (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

        connect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
                this, &QAppImageUpdatePrivate::handleUpdateError,
                (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

        connect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
                this, &QAppImageUpdatePrivate::handleUpdateError,
                (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

        connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
                this, &QAppImageUpdatePrivate::handleUpdateProgress,
                (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

        connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::canceled,
                this, &QAppImageUpdatePrivate::handleUpdateCancel,
                (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

        //// Started signal will be emitted by handleUpdateStart
        //// which is connected at the construction.
        getMethod(m_UpdateInformation.data(), "getInfo(void)")
        .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);

    } else if(action == Action::UpdateWithGUI || action == Action::UpdateWithGUIAndTorrent) {
#ifdef NO_GUI
        b_Started = b_CancelRequested = b_Finished = b_Canceled = false;
        emit error(QAppImageUpdateEnums::Error::UnsupportedActionForBuild, action);
        return;
#else
        n_CurrentAction = action;

        if(m_ApplicationName.isEmpty()) {
            m_ApplicationName = QCoreApplication::applicationName();
        }

        // Setup GUI if not constructed before
        if(!b_GuiClassesConstructed) {
            m_UpdaterDialog.reset(new QDialog);
            m_UpdaterDialog->setObjectName(QString::fromUtf8("AppImageUpdaterDialog"));

            m_Ui = QSharedPointer<Ui::AppImageUpdaterDialog>(new Ui::AppImageUpdaterDialog);
            m_Ui->setupUi(m_UpdaterDialog.data());

            /*
             * Default program logic.
             */

            connect((m_Ui->updateCancelBtn), &QPushButton::clicked, this, &QAppImageUpdatePrivate::cancel, Qt::QueuedConnection);
            connect(m_UpdaterDialog.data(), &QDialog::rejected, this, &QAppImageUpdatePrivate::cancel, Qt::QueuedConnection);

            b_GuiClassesConstructed = true;
        }

        QPixmap icon;

        if(!m_Icon.isEmpty()) {
            icon.loadFromData(m_Icon);
        }

        /* Set AppImage icon if given. */
        if(!icon.isNull()) {
            (m_Ui->softwareIcon)->setPixmap(icon);
            (m_Ui->softwareIconOnUpdating)->setPixmap(icon);
            m_UpdaterDialog->setWindowIcon(icon);
        } else {
            bool found = false;
            if(QApplication::windowIcon().isNull()) {
                foreach (QWidget *widget, QApplication::allWidgets()) {
                    if(!((widget->windowIcon()).isNull())) {
                        icon = widget->windowIcon().pixmap(100, 100);
                        (m_Ui->softwareIcon)->setPixmap(icon);
                        (m_Ui->softwareIconOnUpdating)->setPixmap(icon);
                        m_UpdaterDialog->setWindowIcon(icon);
                        found = true;
                        break;
                    }
                    QCoreApplication::processEvents();
                }
            } else {
                icon = QApplication::windowIcon().pixmap(100, 100);
                found = true;
            }

            if(!found) {
                (m_Ui->softwareIcon)->setVisible(false);
                (m_Ui->softwareIconOnUpdating)->setVisible(false);
            }
        }



        m_ConfirmationDialog = QSharedPointer<SoftwareUpdateDialog>(new SoftwareUpdateDialog(nullptr, icon, n_GuiFlag));

#ifdef DECENTRALIZED_UPDATE_ENABLED
        m_ControlFileParser->setUseBittorrent((action == Action::UpdateWithGUIAndTorrent));
#else
        m_ControlFileParser->setUseBittorrent(false);
#endif

        connect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
                m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
                m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
                this, SLOT(handleGUIUpdateCheck(QJsonObject)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(progress(int)),
                this, SLOT(handleGUIUpdateCheckProgress(int)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(error(short)),
                this, SLOT(handleGUIUpdateCheckError(short)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_UpdateInformation.data(), SIGNAL(error(short)),
                this, SLOT(handleGUIUpdateCheckError(short)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        /// First Check for Update
        getMethod(m_UpdateInformation.data(), "getInfo(void)")
        .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);

        if(n_GuiFlag & GuiFlag::ShowBeforeProgress) {
            (m_Ui->mainStack)->setCurrentIndex(0);
            showWidget();
        }
#endif // NO_GUI
    } else if(action == Action::Seed) {
#ifndef DECENTRALIZED_UPDATE_ENABLED
	b_Started = b_CancelRequested = b_Finished = b_Canceled = false;
        emit error(QAppImageUpdateEnums::Error::UnsupportedActionForBuild, action);
        return;
#else
	n_CurrentAction = action;

        m_ControlFileParser->setUseBittorrent(true);

	connect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
                m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
                m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
                m_Seeder.data(), SLOT(start(QJsonObject)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_Seeder.data(), SIGNAL(canceled()),
                this, SLOT(handleSeedCancel()),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_Seeder.data(), SIGNAL(error(short)),
                this, SLOT(handleSeedError(short)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_ControlFileParser.data(), SIGNAL(error(short)),
                this, SLOT(handleSeedError(short)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        connect(m_UpdateInformation.data(), SIGNAL(error(short)),
                this, SLOT(handleSeedError(short)),
                (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

        emit started(Action::Seed);
        getMethod(m_UpdateInformation.data(), "getInfo(void)")
        .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);

#endif // DECENTRALIZED_UPDATE_ENABLED
    } else {
        n_CurrentAction = Action::None;
        b_Started = b_Running = b_Canceled = false;
        emit error(QAppImageUpdateEnums::Error::InvalidAction, n_CurrentAction);
    }
    return;
}

void QAppImageUpdatePrivate::cancel(void) {
    if(!b_Started && !b_Running) {
        return;
    }

    b_CancelRequested = true;
    getMethod(m_DeltaWriter.data(),"cancel()")
    .invoke(m_DeltaWriter.data(), Qt::QueuedConnection);
    
    return;
}


/// * * *
/// Private Slots
void QAppImageUpdatePrivate::setCurrentAppImagePath(QString path) {
    m_CurrentAppImagePath = path;
}

void QAppImageUpdatePrivate::handleUpdateProgress(int percentage,
        qint64 bytesReceived,
        qint64 bytesTotal,
        double speed,
        QString units) {
    emit progress(percentage, bytesReceived, bytesTotal, speed, units, n_CurrentAction);
}

void QAppImageUpdatePrivate::handleGetEmbeddedInfoProgress(int percentage) {
    emit progress(percentage, 1, 1, 0, QString(), n_CurrentAction);
}

void QAppImageUpdatePrivate::handleCheckForUpdateProgress(int percentage) {
    emit progress(percentage, 1, 1, 0, QString(), n_CurrentAction);
}

void QAppImageUpdatePrivate::handleGetEmbeddedInfoError(short code) {
    b_Canceled = b_Started = b_Running = false;
    b_Finished = false;
    b_CancelRequested = false;
    disconnect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::error,
               this, &QAppImageUpdatePrivate::handleGetEmbeddedInfoError);
    disconnect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::info,
               this, &QAppImageUpdatePrivate::redirectEmbeddedInformation);
    disconnect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::progress,
               this, &QAppImageUpdatePrivate::handleGetEmbeddedInfoProgress);
    emit error(code, n_CurrentAction);
}

void QAppImageUpdatePrivate::redirectEmbeddedInformation(QJsonObject info) {
    b_Canceled = b_Started = b_Running = false;
    b_Finished = true;
    disconnect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::error,
               this, &QAppImageUpdatePrivate::handleGetEmbeddedInfoError);
    disconnect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::info,
               this, &QAppImageUpdatePrivate::redirectEmbeddedInformation);
    disconnect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::progress,
               this, &QAppImageUpdatePrivate::handleGetEmbeddedInfoProgress);

    if(b_CancelRequested) {
        b_CancelRequested = false;
        b_Canceled = true;
        emit canceled(n_CurrentAction);
        return;
    }
    emit finished(info, n_CurrentAction);
}


void QAppImageUpdatePrivate::handleCheckForUpdateError(short code) {
    b_Canceled = b_Started = b_Running = false;
    b_Finished = false;
    b_CancelRequested = false;

    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
               this, SLOT(redirectUpdateCheck(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(error(short)),
               this, SLOT(handleCheckForUpdateError(short)));
    disconnect(m_UpdateInformation.data(), SIGNAL(error(short)),
               this, SLOT(handleCheckForUpdateError(short)));
    disconnect(m_ControlFileParser.data(), SIGNAL(progress(int)),
               this, SLOT(handleCheckForUpdateProgress(int)));

    emit error(code, n_CurrentAction);
}

void QAppImageUpdatePrivate::handleSeedError(short code) {
    b_Canceled = b_Started = b_Running = false;
    b_Finished = false;
    b_CancelRequested = false;

    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
                m_Seeder.data(), SLOT(start(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(error(short)),
               this, SLOT(handleSeedError(short)));
    disconnect(m_UpdateInformation.data(), SIGNAL(error(short)),
               this, SLOT(handleSeedError(short)));
    disconnect(m_Seeder.data(), SIGNAL(canceled()),
                this, SLOT(handleSeedCancel()));
    disconnect(m_Seeder.data(), SIGNAL(error(short)),
                this, SLOT(handleSeedError(short)));

    emit error(code, n_CurrentAction);
}

void QAppImageUpdatePrivate::handleSeedCancel() {
    b_Canceled = b_Started = b_Running = false;
    b_Finished = false;
    b_CancelRequested = false;

    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
                m_Seeder.data(), SLOT(start(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(error(short)),
               this, SLOT(handleSeedError(short)));
    disconnect(m_UpdateInformation.data(), SIGNAL(error(short)),
               this, SLOT(handleSeedError(short)));
    disconnect(m_Seeder.data(), SIGNAL(canceled()),
                this, SLOT(handleSeedCancel()));
    disconnect(m_Seeder.data(), SIGNAL(error(short)),
                this, SLOT(handleSeedError(short)));

    b_Finished = true;

    QJsonObject r { };
    emit finished(r, n_CurrentAction);
}

void QAppImageUpdatePrivate::redirectUpdateCheck(QJsonObject info) {
    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
               this, SLOT(redirectUpdateCheck(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(error(short)),
               this, SLOT(handleCheckForUpdateError(short)));
    disconnect(m_UpdateInformation.data(), SIGNAL(error(short)),
               this, SLOT(handleCheckForUpdateError(short)));
    disconnect(m_ControlFileParser.data(), SIGNAL(progress(int)),
               this, SLOT(handleCheckForUpdateProgress(int)));


    // Can this happen without an error?
    if(info.isEmpty()) {
        return;
    }

    auto releaseNotes = info["ReleaseNotes"].toString();
    auto embeddedUpdateInformation = info["EmbededUpdateInformation"].toObject();
    auto oldVersionInformation = embeddedUpdateInformation["FileInformation"].toObject();

    QString remoteTargetFileSHA1Hash = info["RemoteTargetFileSHA1Hash"].toString(),
            localAppImageSHA1Hash = oldVersionInformation["AppImageSHA1Hash"].toString(),
            localAppImagePath = oldVersionInformation["AppImageFilePath"].toString();

    bool torrentSupported = info["TorrentSupported"].toBool();
    auto torrentFileUrl = info["TorrentFileUrl"].toString();
    auto targetFileName = info["RemoteTargetFileName"].toString();

    QJsonObject updateinfo {
        { "UpdateAvailable", localAppImageSHA1Hash != remoteTargetFileSHA1Hash},
        { "AbsolutePath", localAppImagePath},
	{ "RemoteTargetFileName", targetFileName}, 
	{ "LocalSha1Hash",  localAppImageSHA1Hash },
        { "RemoteSha1Hash", remoteTargetFileSHA1Hash},
        { "ReleaseNotes", releaseNotes},
        { "TorrentSupported", torrentSupported},
	{ "TorrentFileUrl", torrentFileUrl }
    };

    b_Started = b_Running = false;
    b_Finished = true;
    b_Canceled = false;


    if(b_CancelRequested) {
        b_CancelRequested = false;
        b_Canceled = true;
        emit canceled(n_CurrentAction);
        return;
    }
    emit finished(updateinfo, n_CurrentAction);
}

void QAppImageUpdatePrivate::handleUpdateStart() {
    emit started(n_CurrentAction);
}

void QAppImageUpdatePrivate::handleUpdateCancel() {
    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getZsyncInformation(void)));
    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::start);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finished,
               this, &QAppImageUpdatePrivate::handleUpdateFinished);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::error,
               this, &QAppImageUpdatePrivate::handleUpdateError);
    disconnect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
               this, &QAppImageUpdatePrivate::handleUpdateError);
    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
               this, &QAppImageUpdatePrivate::handleUpdateError);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
               this, &QAppImageUpdatePrivate::handleUpdateProgress);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::canceled,
               this, &QAppImageUpdatePrivate::handleUpdateCancel);

    b_Started = b_Running = b_Finished = b_CancelRequested = false;
    b_Canceled = true;

    emit canceled(n_CurrentAction);

}

void QAppImageUpdatePrivate::handleUpdateError(short ecode) {
    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getZsyncInformation(void)));
    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::start);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finished,
               this, &QAppImageUpdatePrivate::handleUpdateFinished);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::error,
               this, &QAppImageUpdatePrivate::handleUpdateError);
    disconnect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
               this, &QAppImageUpdatePrivate::handleUpdateError);
    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
               this, &QAppImageUpdatePrivate::handleUpdateError);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
               this, &QAppImageUpdatePrivate::handleUpdateProgress);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::canceled,
               this, &QAppImageUpdatePrivate::handleUpdateCancel);


    b_Started = b_Running = b_Finished = false;
    b_Canceled = false;

    if(b_CancelRequested) {
        b_CancelRequested = false;
        b_Canceled = true;
        emit canceled(n_CurrentAction);
        return;
    }

    emit error(ecode, n_CurrentAction);
}

void QAppImageUpdatePrivate::handleUpdateFinished(QJsonObject info, QString oldVersionPath) {
    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getZsyncInformation(void)));
    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::start);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finished,
               this, &QAppImageUpdatePrivate::handleUpdateFinished);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::error,
               this, &QAppImageUpdatePrivate::handleUpdateError);
    disconnect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
               this, &QAppImageUpdatePrivate::handleUpdateError);
    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
               this, &QAppImageUpdatePrivate::handleUpdateError);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
               this, &QAppImageUpdatePrivate::handleUpdateProgress);
    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::canceled,
               this, &QAppImageUpdatePrivate::handleUpdateCancel);


    QJsonObject result {
        {"OldVersionPath", oldVersionPath},
        {"NewVersionPath", info["AbsolutePath"].toString()},
        {"NewVersionSha1Hash", info["Sha1Hash"].toString()},
        {"UsedTorrent", info["UsedTorrent"].toBool()},
	{"TorrentFileUrl", info["TorrentFileUrl"].toString()}
    };
    b_Started = b_Running = false;
    b_Finished = true;
    b_Canceled = false;

    if(b_CancelRequested) {
        b_CancelRequested = false;
        b_Canceled = true;
        emit canceled(n_CurrentAction);
        return;
    }

    emit finished(result, n_CurrentAction);
}


/// GUI Update routines.

#ifndef NO_GUI
void QAppImageUpdatePrivate::showWidget() {
    if(m_Ui.isNull() || m_UpdaterDialog.isNull()) {
        return;
    }
    if(!(n_GuiFlag & GuiFlag::ShowProgressDialog) &&
            (m_Ui->mainStack)->currentIndex() != 0) {
        return;
    }
    m_UpdaterDialog->show();
    return;
}

void QAppImageUpdatePrivate::handleGUIConfirmationRejected() {
    handleGUIUpdateCancel();
}

void QAppImageUpdatePrivate::handleGUIConfirmationAccepted() {
    doGUIUpdate();
}

void QAppImageUpdatePrivate::doGUIUpdate() {
    connect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
            m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)),
            (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

    connect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
            m_ControlFileParser.data(), SLOT(getZsyncInformation(void)),
            (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

    connect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
            m_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

    connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
            m_DeltaWriter.data(), &ZsyncWriterPrivate::start,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

    connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finished,
            this, &QAppImageUpdatePrivate::handleGUIUpdateFinished,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

    connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::error,
            this, &QAppImageUpdatePrivate::handleGUIUpdateError,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

    connect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
            this, &QAppImageUpdatePrivate::handleGUIUpdateError,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

    connect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
            this, &QAppImageUpdatePrivate::handleGUIUpdateError,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

    connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
            this, &QAppImageUpdatePrivate::handleGUIUpdateProgress,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

    connect(m_DeltaWriter.data(), &ZsyncWriterPrivate::canceled,
            this, &QAppImageUpdatePrivate::handleGUIUpdateCancel,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

    getMethod(m_UpdateInformation.data(), "getInfo(void)")
    .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);


    showWidget();
}

void QAppImageUpdatePrivate::handleGUIUpdateCheck(QJsonObject info) {
    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));

    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
               this, SLOT(handleGUIUpdateCheck(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(progress(int)),
               this, SLOT(handleGUIUpdateCheckProgress(int)));
    disconnect(m_ControlFileParser.data(), SIGNAL(error(short)),
               this, SLOT(handleGUIUpdateCheckError(short)));
    disconnect(m_UpdateInformation.data(), SIGNAL(error(short)),
               this, SLOT(handleGUIUpdateCheckError(short)));


    /// Connect confirmation buttons
    connect(m_ConfirmationDialog.data(), &SoftwareUpdateDialog::rejected,
            this, &QAppImageUpdatePrivate::handleGUIConfirmationRejected, Qt::QueuedConnection);
    connect(m_ConfirmationDialog.data(), &SoftwareUpdateDialog::accepted,
            this, &QAppImageUpdatePrivate::handleGUIConfirmationAccepted, Qt::QueuedConnection);



    if(b_CancelRequested) {
        b_Started = b_Running = b_Finished = false;
        b_Canceled = false;
        b_CancelRequested = false;
        b_Canceled = true;
        emit canceled(n_CurrentAction);
        return;
    }



    m_UpdaterDialog->hide();
    m_UpdaterDialog->move(QGuiApplication::primaryScreen()->geometry().center() - m_UpdaterDialog->rect().center());
    (m_Ui->mainStack)->setCurrentIndex(1);

    auto notes = info["ReleaseNotes"].toString();
    auto embeddedUpdateInformation = info["EmbededUpdateInformation"].toObject();
    auto oldVersionInformation = embeddedUpdateInformation["FileInformation"].toObject();

    QString remoteTargetFileSHA1Hash = info["RemoteTargetFileSHA1Hash"].toString(),
            localAppImageSHA1Hash = oldVersionInformation["AppImageSHA1Hash"].toString(),
            localAppImagePath = oldVersionInformation["AppImageFilePath"].toString();

    bool torrentSupported = info["TorrentSupported"].toBool();

    bool showUpdateDialog = n_GuiFlag & GuiFlag::ShowUpdateConfirmationDialog;
    bool showNoUpdateDialog = n_GuiFlag & GuiFlag::NotifyWhenNoUpdateIsAvailable;
    bool noConfirmTorrentUsage = n_GuiFlag & GuiFlag::NoConfirmTorrentUsage;
    m_UpdaterDialog->setWindowTitle(QString::fromUtf8("Updating ") +
		    		   (m_ApplicationName.isEmpty() ? 
				    QFileInfo(localAppImagePath).baseName() : 
				    m_ApplicationName));

    bool isUpdateAvailable = (localAppImageSHA1Hash != remoteTargetFileSHA1Hash);


    if(isUpdateAvailable) {
        if(torrentSupported) {
            bool permission = true;
            if(!noConfirmTorrentUsage) {
                QMessageBox box(m_UpdaterDialog.data());
                box.setWindowTitle(QString::fromUtf8("Torrent Usage Permission"));
                box.setIcon(QMessageBox::Information);
                box.setText(
                    QString::fromUtf8(
                        "It seems that the author of ") +
                    (m_ApplicationName.isEmpty() ? QFileInfo(localAppImagePath).baseName() : 
		     m_ApplicationName) +
                    QString::fromUtf8(" supports decentralized update via Bittorrent.") +
                    QString::fromUtf8(
                        " Do you agree to <b>use Bittorrent for decentralized update?</b> This is completely optional.") +
                    QString::fromUtf8(
                        " <b>Please click 'No' if you don't understand the question</b>.")
                );
                box.addButton(QMessageBox::Yes);
                box.addButton(QMessageBox::No);
                permission = (box.exec() == QMessageBox::Yes);

            }

            if(!permission) {
                m_ControlFileParser->setUseBittorrent(false);
            }
        }
        if(showUpdateDialog) {
            QString oldSha = localAppImageSHA1Hash,
                    newSha = remoteTargetFileSHA1Hash;
            /* we don't need the full sha to show the diff. */
            oldSha.resize(7);
            newSha.resize(7);

            oldSha = oldSha.toLower();
            newSha = newSha.toLower();

            m_ConfirmationDialog->init( m_ApplicationName, oldSha, newSha, notes);
        } else {
            // Auto confirm if we are not showing the confirm dialog.
            doGUIUpdate();
        }
    } else {
        if(showNoUpdateDialog) {
            QMessageBox box(m_UpdaterDialog.data());
            box.setWindowTitle(QString::fromUtf8("No Updates Available!"));
            box.setText(
                QString::fromUtf8("You are currently using the lastest version of ") +\
		(m_ApplicationName.isEmpty() ? QFileInfo(localAppImagePath).fileName()
		 : m_ApplicationName ) +
                QString::fromUtf8("."));
            box.exec();
        }

        QJsonObject result {
            {"OldVersionPath", localAppImagePath},
            {"NewVersionPath", localAppImagePath},
            {"NewVersionSha1Hash", localAppImageSHA1Hash},
            {"UsedTorrent", false}
        };
        b_Started = b_Running = false;
        b_Finished = true;
        b_Canceled = false;

        emit finished(result, n_CurrentAction);
    }

}



void QAppImageUpdatePrivate::handleGUIUpdateCheckError(short ecode) {
    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));

    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)));
    disconnect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
               this, SLOT(handleGUIUpdateCheck(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(progress(int)),
               this, SLOT(handleGUIUpdateCheckProgress(int)));
    disconnect(m_ControlFileParser.data(), SIGNAL(error(short)),
               this, SLOT(handleGUIUpdateCheckError(short)));
    disconnect(m_UpdateInformation.data(), SIGNAL(error(short)),
               this, SLOT(handleGUIUpdateCheckError(short)));

    if(b_CancelRequested) {
        b_Started = b_Running = b_Finished = false;
        b_Canceled = false;
        b_CancelRequested = false;
        b_Canceled = true;
        emit canceled(n_CurrentAction);
        return;
    }

    bool show = n_GuiFlag & GuiFlag::ShowErrorDialog;


    QString errorString = QAppImageUpdatePrivate::errorCodeToDescriptionString(ecode);
    if(ecode == QAppImageUpdateEnums::Error::NoReadPermission ||
            ecode == QAppImageUpdateEnums::Error::NoPermissionToReadSourceFile ||
            ecode == QAppImageUpdateEnums::Error::NoPermissionToReadWriteTargetFile) {
        if(n_GuiFlag & GuiFlag::NoShowErrorDialogOnPermissionErrors) {
            show = false;
        }
    }

    if(show) {
        QMessageBox box(m_UpdaterDialog.data());
        box.setWindowTitle(QString::fromUtf8("Update Failed"));
        box.setIcon(QMessageBox::Critical);
        box.setText(QString::fromUtf8("Update failed for '") +
                    QFileInfo(m_CurrentAppImagePath).fileName() +
                    QString::fromUtf8("': ") + errorString);
        box.exec();
    }


    emit error(ecode, n_CurrentAction);
    m_UpdaterDialog->hide();
    return;
}

void QAppImageUpdatePrivate::handleGUIUpdateCheckProgress(int percentage) {
	Q_UNUSED(percentage);
}


void QAppImageUpdatePrivate::handleGUIUpdateProgress(int percentage,
        qint64 bytesReceived,
        qint64 bytesTotal,
        double speed,
        QString units) {
    //// Show that we are canceling if cancel was requested.
    if(b_CancelRequested) {
        (m_Ui->updateSpeedLbl)->setText(QString::fromUtf8("Rolling back changes, Please wait... "));
        return;
    }

    (m_Ui->progressBar)->setValue(percentage);
    double MegaBytesReceived = bytesReceived / 1048576;
    double MegaBytesTotal = bytesTotal / 1048576;
    const QString progressTemplate = QString::fromUtf8("Updating %1 MiB of %2 MiB at %3 %4...");
    QString statusText = progressTemplate.arg(MegaBytesReceived).arg(MegaBytesTotal).arg(speed).arg(units);
    (m_Ui->updateSpeedLbl)->setText(statusText);
}


void QAppImageUpdatePrivate::handleGUIUpdateStart() {
    emit started(n_CurrentAction);
}

void QAppImageUpdatePrivate::handleGUIUpdateCancel() {
    m_UpdaterDialog->hide();

    disconnect(m_ConfirmationDialog.data(), &SoftwareUpdateDialog::rejected,
               this, &QAppImageUpdatePrivate::handleGUIConfirmationRejected);
    disconnect(m_ConfirmationDialog.data(), &SoftwareUpdateDialog::accepted,
               this, &QAppImageUpdatePrivate::handleGUIConfirmationAccepted);

    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));

    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getZsyncInformation(void)));

    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::start);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finished,
               this, &QAppImageUpdatePrivate::handleGUIUpdateFinished);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::error,
               this, &QAppImageUpdatePrivate::handleGUIUpdateError);

    disconnect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
               this, &QAppImageUpdatePrivate::handleGUIUpdateError);

    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
               this, &QAppImageUpdatePrivate::handleGUIUpdateError);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
               this, &QAppImageUpdatePrivate::handleGUIUpdateProgress);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::canceled,
               this, &QAppImageUpdatePrivate::handleGUIUpdateCancel);

    b_Started = b_Running = b_Finished = b_CancelRequested = false;
    b_Canceled = true;

    emit canceled(n_CurrentAction);
}

void QAppImageUpdatePrivate::handleGUIUpdateError(short ecode) {
    disconnect(m_ConfirmationDialog.data(), &SoftwareUpdateDialog::rejected,
               this, &QAppImageUpdatePrivate::handleGUIConfirmationRejected);
    disconnect(m_ConfirmationDialog.data(), &SoftwareUpdateDialog::accepted,
               this, &QAppImageUpdatePrivate::handleGUIConfirmationAccepted);

    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));

    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getZsyncInformation(void)));

    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::start);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finished,
               this, &QAppImageUpdatePrivate::handleGUIUpdateFinished);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::error,
               this, &QAppImageUpdatePrivate::handleGUIUpdateError);

    disconnect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
               this, &QAppImageUpdatePrivate::handleGUIUpdateError);

    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
               this, &QAppImageUpdatePrivate::handleGUIUpdateError);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
               this, &QAppImageUpdatePrivate::handleGUIUpdateProgress);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::canceled,
               this, &QAppImageUpdatePrivate::handleGUIUpdateCancel);


    b_Started = b_Running = b_Finished = false;
    b_Canceled = false;

    if(b_CancelRequested) {
        b_CancelRequested = false;
        b_Canceled = true;
        emit canceled(n_CurrentAction);
        return;
    }

    bool show = n_GuiFlag & GuiFlag::ShowErrorDialog;


    QString errorString = QAppImageUpdatePrivate::errorCodeToDescriptionString(ecode);
    if(ecode == QAppImageUpdateEnums::Error::NoReadPermission ||
            ecode == QAppImageUpdateEnums::Error::NoPermissionToReadSourceFile ||
            ecode == QAppImageUpdateEnums::Error::NoPermissionToReadWriteTargetFile) {
        if(n_GuiFlag & GuiFlag::NoShowErrorDialogOnPermissionErrors) {
            show = false;
        }
    }

    if(show) {
        QMessageBox box(m_UpdaterDialog.data());
        box.setWindowTitle(QString::fromUtf8("Update Failed"));
        box.setIcon(QMessageBox::Critical);
        box.setText(QString::fromUtf8("Update failed for '") +
                    (m_ApplicationName.isEmpty() ? QFileInfo(m_CurrentAppImagePath).fileName() : m_ApplicationName) +
                    QString::fromUtf8("': ") + errorString);
        box.exec();
    }


    emit error(ecode, n_CurrentAction);
    m_UpdaterDialog->hide();
    return;
}

void QAppImageUpdatePrivate::handleGUIUpdateFinished(QJsonObject info, QString oldVersionPath) {
    disconnect(m_ConfirmationDialog.data(), &SoftwareUpdateDialog::rejected,
               this, &QAppImageUpdatePrivate::handleGUIConfirmationRejected);
    disconnect(m_ConfirmationDialog.data(), &SoftwareUpdateDialog::accepted,
               this, &QAppImageUpdatePrivate::handleGUIConfirmationAccepted);

    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
               m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)));

    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
               m_ControlFileParser.data(), SLOT(getZsyncInformation(void)));

    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
               m_DeltaWriter.data(), &ZsyncWriterPrivate::start);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::finished,
               this, &QAppImageUpdatePrivate::handleGUIUpdateFinished);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::error,
               this, &QAppImageUpdatePrivate::handleGUIUpdateError);

    disconnect(m_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
               this, &QAppImageUpdatePrivate::handleGUIUpdateError);

    disconnect(m_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
               this, &QAppImageUpdatePrivate::handleGUIUpdateError);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
               this, &QAppImageUpdatePrivate::handleGUIUpdateProgress);

    disconnect(m_DeltaWriter.data(), &ZsyncWriterPrivate::canceled,
               this, &QAppImageUpdatePrivate::handleGUIUpdateCancel);



    QJsonObject result {
        {"OldVersionPath", oldVersionPath},
        {"NewVersionPath", info["AbsolutePath"].toString()},
        {"NewVersionSha1Hash", info["Sha1Hash"].toString()},
        {"UsedTorrent", info["UsedTorrent"].toBool()}
    };
    b_Started = b_Running = false;
    b_Finished = true;
    b_Canceled = false;

    if(b_CancelRequested) {
        QFile::remove(info["AbsolutePath"].toString());

        b_CancelRequested = false;
        b_Canceled = true;
        emit canceled(n_CurrentAction);
        return;
    }

    (m_Ui->updateSpeedLbl)->setText(QString::fromUtf8("Finalizing Update... "));

    bool execute = false;
    bool show = n_GuiFlag & GuiFlag::ShowFinishedDialog;

    if(show) {
        auto curinfo = QFileInfo(oldVersionPath);
        QString currentAppImageName = curinfo.fileName();
        QString currentAppImageAbsPath = curinfo.absolutePath();

        QMessageBox box(m_UpdaterDialog.data());
        box.setWindowTitle(QString::fromUtf8("Update Completed"));
        box.setIcon(QMessageBox::Information);
        box.setDetailedText(
            QString::fromUtf8("Old version is at: ") +
            currentAppImageAbsPath + QString::fromUtf8("/") + currentAppImageName +
            QString::fromUtf8("\n\n") +
            QString::fromUtf8("New version is saved at: ") +
            result["NewVersionPath"].toString());
        box.setText(QString::fromUtf8("Update completed successfully for <b>") +
                    (m_ApplicationName.isEmpty() ? currentAppImageName : m_ApplicationName) +
                    QString::fromUtf8("</b>, do you want to launch the new version? View details for more information."));
        box.addButton(QMessageBox::Yes);
        box.addButton(QMessageBox::No);
        execute = (box.exec() == QMessageBox::Yes);
    }

    if(execute) {
        QFileInfo info(result["NewVersionPath"].toString());
        if(!info.isExecutable()) {
            {
                QFile file(result["NewVersionPath"].toString());
                file.setPermissions(QFileDevice::ExeUser |
                                    QFileDevice::ExeOther|
                                    QFileDevice::ExeGroup|
                                    info.permissions());
            }
        }


        //// The delay is set to 5 seconds
        QProcess::startDetached("sh",
                                QStringList()
                                << "-c"
                                << (
                                    QString::fromUtf8("sleep ") +
                                    QString::number(5) +
                                    QString::fromUtf8("; ") +
                                    result["NewVersionPath"].toString()
                                ));
        m_UpdaterDialog->hide();
        emit quit();
    }
    m_UpdaterDialog->hide();
    emit finished(result, n_CurrentAction);
    return;
}
#endif // NOT NO_GUI

//// Static Methods
QString QAppImageUpdatePrivate::errorCodeToString(short errorCode) {
    QString ret = "QAppImageUpdate::Error::";
    switch(errorCode) {
    case QAppImageUpdateEnums::Error::NoAppimagePathGiven:
        ret += "NoAppImagePathGiven";
        break;
    case QAppImageUpdateEnums::Error::AppimageNotReadable:
        ret += "AppImageNotReadable";
        break;
    case QAppImageUpdateEnums::Error::NoReadPermission:
        ret += "NoReadPermission";
        break;
    case QAppImageUpdateEnums::Error::AppimageNotFound:
        ret += "AppimageNotFound";
        break;
    case QAppImageUpdateEnums::Error::CannotOpenAppimage:
        ret += "CannnotOpenAppimage";
        break;
    case QAppImageUpdateEnums::Error::EmptyUpdateInformation:
        ret += "EmptyUpdateInformation";
        break;
    case QAppImageUpdateEnums::Error::InvalidAppimageType:
        ret += "InvalidAppimageType";
        break;
    case QAppImageUpdateEnums::Error::InvalidMagicBytes:
        ret += "InvalidMagicBytes";
        break;
    case QAppImageUpdateEnums::Error::InvalidUpdateInformation:
        ret += "InvalidUpdateInformation";
        break;
    case QAppImageUpdateEnums::Error::NotEnoughMemory:
        ret += "NotEnoughMemory";
        break;
    case QAppImageUpdateEnums::Error::SectionHeaderNotFound:
        ret += "SectionHeaderNotFound";
        break;
    case QAppImageUpdateEnums::Error::UnsupportedElfFormat:
        ret += "UnsupportedElfFormat";
        break;
    case QAppImageUpdateEnums::Error::UnsupportedTransport:
        ret += "UnsupportedTransport";
        break;
    case QAppImageUpdateEnums::Error::UnknownNetworkError:
        ret += "UnknownNetworkError";
        break;
    case QAppImageUpdateEnums::Error::IoReadError:
        ret += "IoReadError";
        break;
    case QAppImageUpdateEnums::Error::ErrorResponseCode:
        ret += "ErrorResponseCode";
        break;
    case QAppImageUpdateEnums::Error::GithubApiRateLimitReached:
        ret += "GithubApiRateLimitReached";
        break;
    case QAppImageUpdateEnums::Error::NoMarkerFoundInControlFile:
        ret += "NoMarkerFoundInControlFile";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncHeadersNumber:
        ret += "InvalidZsyncHeadersNumber";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncMakeVersion:
        ret += "InvalidZsyncMakeVersion";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncTargetFilename:
        ret += "InvalidZsyncTargetFilename";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncMtime:
        ret += "InvalidZsyncMtime";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncBlocksize:
        ret += "InvalidZsyncBlocksize";
        break;
    case QAppImageUpdateEnums::Error::InvalidTargetFileLength:
        ret += "InvalidTargetFileLength";
        break;
    case QAppImageUpdateEnums::Error::InvalidHashLengthLine:
        ret += "InvalidHashLengthLine";
        break;
    case QAppImageUpdateEnums::Error::InvalidHashLengths:
        ret += "InvalidHashLengths";
        break;
    case QAppImageUpdateEnums::Error::InvalidTargetFileUrl:
        ret += "InvalidTargetFileUrl";
        break;
    case QAppImageUpdateEnums::Error::InvalidTargetFileSha1:
        ret += "InvalidTargetFileSha1";
        break;
    case QAppImageUpdateEnums::Error::ConnectionRefusedError:
        ret += "ConnectionRefusedError";
        break;
    case QAppImageUpdateEnums::Error::RemoteHostClosedError:
        ret += "RemoteHostClosedError";
        break;
    case QAppImageUpdateEnums::Error::HostNotFoundError:
        ret += "HostNotFoundError";
        break;
    case QAppImageUpdateEnums::Error::TimeoutError:
        ret += "TimeoutError";
        break;
    case QAppImageUpdateEnums::Error::OperationCanceledError:
        ret += "OperationCanceledError";
        break;
    case QAppImageUpdateEnums::Error::SslHandshakeFailedError:
        ret += "SslHandshakeFailedError";
        break;
    case QAppImageUpdateEnums::Error::TemporaryNetworkFailureError:
        ret += "TemporaryNetworkFailureError";
        break;
    case QAppImageUpdateEnums::Error::NetworkSessionFailedError:
        ret += "NetworkSessionFailedError";
        break;
    case QAppImageUpdateEnums::Error::BackgroundRequestNotAllowedError:
        ret += "BackgroundRequestNotAllowedError";
        break;
    case QAppImageUpdateEnums::Error::TooManyRedirectsError:
        ret += "TooManyRedirectsError";
        break;
    case QAppImageUpdateEnums::Error::InsecureRedirectError:
        ret += "InsecureRedirectError";
        break;
    case QAppImageUpdateEnums::Error::ProxyConnectionRefusedError:
        ret += "ProxyConnectionRefusedError";
        break;
    case QAppImageUpdateEnums::Error::ProxyConnectionClosedError:
        ret += "ProxyConnectionClosedError";
        break;
    case QAppImageUpdateEnums::Error::ProxyNotFoundError:
        ret += "ProxyNotFoundError";
        break;
    case QAppImageUpdateEnums::Error::ProxyTimeoutError:
        ret += "ProxyTimeoutError";
        break;
    case QAppImageUpdateEnums::Error::ProxyAuthenticationRequiredError:
        ret += "ProxyAuthenticationRequiredError";
        break;
    case QAppImageUpdateEnums::Error::ContentAccessDenied:
        ret += "ContentAccessDenied";
        break;
    case QAppImageUpdateEnums::Error::ContentOperationNotPermittedError:
        ret += "ContentOperationNotPermittedError";
        break;
    case QAppImageUpdateEnums::Error::ContentNotFoundError:
        ret += "ContentNotFoundError";
        break;
    case QAppImageUpdateEnums::Error::AuthenticationRequiredError:
        ret += "AuthenticationRequiredError";
        break;
    case QAppImageUpdateEnums::Error::ContentReSendError:
        ret += "ContentReSendError";
        break;
    case QAppImageUpdateEnums::Error::ContentConflictError:
        ret += "ContentConflictError";
        break;
    case QAppImageUpdateEnums::Error::ContentGoneError:
        ret += "ContentGoneError";
        break;
    case QAppImageUpdateEnums::Error::InternalServerError:
        ret += "InternalServerError";
        break;
    case QAppImageUpdateEnums::Error::OperationNotImplementedError:
        ret += "OperationNotImplementedError";
        break;
    case QAppImageUpdateEnums::Error::ServiceUnavailableError:
        ret += "ServiceUnavailableError";
        break;
    case QAppImageUpdateEnums::Error::ProtocolUnknownError:
        ret += "ProtocolUnknownError";
        break;
    case QAppImageUpdateEnums::Error::ProtocolInvalidOperationError:
        ret += "ProtocolInvalidOperationError";
        break;
    case QAppImageUpdateEnums::Error::UnknownProxyError:
        ret += "UnknownProxyError";
        break;
    case QAppImageUpdateEnums::Error::UnknownContentError:
        ret += "UnknownContentError";
        break;
    case QAppImageUpdateEnums::Error::ProtocolFailure:
        ret += "ProtocolFailure";
        break;
    case QAppImageUpdateEnums::Error::UnknownServerError:
        ret += "UnknownServerError";
        break;
    case QAppImageUpdateEnums::Error::ZsyncControlFileNotFound:
        ret += "ZsyncControlFileNotFound";
        break;
    case QAppImageUpdateEnums::Error::HashTableNotAllocated:
        ret += "HashTableNotAllocated";
        break;
    case QAppImageUpdateEnums::Error::InvalidTargetFileChecksumBlocks:
        ret += "InvalidTargetFileChecksumBlocks";
        break;
    case QAppImageUpdateEnums::Error::CannotConstructHashTable:
        ret += "CannotConstructHashTable";
        break;
    case QAppImageUpdateEnums::Error::CannotOpenTargetFileChecksumBlocks:
        ret += "CannotOpenTargetFileChecksumBlocks";
        break;
    case QAppImageUpdateEnums::Error::QbufferIoReadError:
        ret += "QbufferIoReadError";
        break;
    case QAppImageUpdateEnums::Error::SourceFileNotFound:
        ret += "SourceFileNotFound";
        break;
    case QAppImageUpdateEnums::Error::NoPermissionToReadSourceFile:
        ret += "NoPermissionToReadSourceFile";
        break;
    case QAppImageUpdateEnums::Error::CannotOpenSourceFile:
        ret += "CannotOpenSourceFile";
        break;
    case QAppImageUpdateEnums::Error::NoPermissionToReadWriteTargetFile:
        ret += "NoPermissionToReadWriteTargetFile";
        break;
    case QAppImageUpdateEnums::Error::CannotOpenTargetFile:
        ret += "CannotOpenTargetFile";
        break;
    case QAppImageUpdateEnums::Error::TargetFileSha1HashMismatch:
        ret += "TargetFileSha1HashMismatch";
        break;
    case QAppImageUpdateEnums::Error::TorrentNotSupported:
	ret += "TorrentNotSupported";
	break;
    case QAppImageUpdateEnums::Error::TorrentSeedFailed:
	ret += "TorrentSeedFailed";
	break;
    case QAppImageUpdateEnums::Error::OutdatedAppImageForSeed:
	ret += "OutdatedAppImageForSeed";
	break;
    case QAppImageUpdateEnums::Error::IncompleteAppImageForSeed:
	ret += "IncompleteAppImageForSeed";
	break;
    case QAppImageUpdateEnums::Error::UnsupportedActionForBuild:
        ret += "UnsupportedActionForBuild";
        break;
    case QAppImageUpdateEnums::Error::InvalidAction:
        ret += "InvalidAction";
        break;
    default:
        ret += "Unknown";
        break;
    }
    return ret;
}

QString QAppImageUpdatePrivate::errorCodeToDescriptionString(short errorCode) {
    QString errorString;
    switch(errorCode) {
    case QAppImageUpdateEnums::Error::ConnectionRefusedError:
        errorString = QString::fromUtf8("The update server is not accepting requests.");
        break;
    case QAppImageUpdateEnums::Error::RemoteHostClosedError:
        errorString = QString::fromUtf8("The remote server closed the connection prematurely, ");
        errorString += QString::fromUtf8("before the entire reply was received and processed.");
        break;
    case QAppImageUpdateEnums::Error::HostNotFoundError:
        errorString = QString::fromUtf8("The remote host name was not found (invalid hostname).");
        break;
    case QAppImageUpdateEnums::Error::TimeoutError:
        errorString = QString::fromUtf8("The connection to the remote server timed out.");
        break;
    case QAppImageUpdateEnums::Error::SslHandshakeFailedError:
        errorString = QString::fromUtf8("The SSL/TLS handshake failed and the encrypted channel ");
        errorString += QString::fromUtf8("could not be established.");
        break;
    case QAppImageUpdateEnums::Error::TemporaryNetworkFailureError:
        errorString = QString::fromUtf8("The connection to the network was broken.");
        break;
    case QAppImageUpdateEnums::Error::NetworkSessionFailedError:
        errorString = QString::fromUtf8("The connection to the network was broken ");
        errorString += QString::fromUtf8("or could not be initiated.");
        break;
    case QAppImageUpdateEnums::Error::BackgroundRequestNotAllowedError:
        errorString = QString::fromUtf8("The background request is not currently allowed due to platform policy.");
        break;
    case QAppImageUpdateEnums::Error::TooManyRedirectsError:
        errorString = QString::fromUtf8("While following redirects, the maximum limit was reached.");
        break;
    case QAppImageUpdateEnums::Error::InsecureRedirectError:
        errorString = QString::fromUtf8("While following redirects, there was a redirect ");
        errorString += QString::fromUtf8("from a encrypted protocol (https) to an unencrypted one (http).");
        break;
    case QAppImageUpdateEnums::Error::ContentAccessDenied:
        errorString = QString::fromUtf8("The access to the remote content was denied (HTTP error 403).");
        break;
    case QAppImageUpdateEnums::Error::ContentOperationNotPermittedError:
        errorString = QString::fromUtf8("The operation requested on the remote content is not permitted.");
        break;
    case QAppImageUpdateEnums::Error::ContentNotFoundError:
        errorString = QString::fromUtf8("The remote content was not found at the server (HTTP error 404)");
        break;
    case QAppImageUpdateEnums::Error::AuthenticationRequiredError:
        errorString = QString::fromUtf8("The remote server requires authentication to serve the content, ");
        errorString += QString::fromUtf8("but the credentials provided were not accepted or given.");
        break;
    case QAppImageUpdateEnums::Error::ContentConflictError:
        errorString = QString::fromUtf8("The request could not be completed due to a conflict with the ");
        errorString += QString::fromUtf8("current state of the resource.");
        break;
    case QAppImageUpdateEnums::Error::ContentGoneError:
        errorString = QString::fromUtf8("The requested resource is no longer available at the server.");
        break;
    case QAppImageUpdateEnums::Error::InternalServerError:
        errorString = QString::fromUtf8("The server encountered an unexpected condition which prevented ");
        errorString += QString::fromUtf8("it from fulfilling the request.");
        break;
    case QAppImageUpdateEnums::Error::OperationNotImplementedError:
        errorString = QString::fromUtf8("The server does not support the functionality required to fulfill the request.");
        break;
    case QAppImageUpdateEnums::Error::ServiceUnavailableError:
        errorString = QString::fromUtf8("The server is unable to handle the request at this time.");
        break;
    case QAppImageUpdateEnums::Error::ProtocolUnknownError:
        errorString = QString::fromUtf8("The Network Access API cannot honor the request because the protocol");
        errorString += QString::fromUtf8(" is not known.");
        break;
    case QAppImageUpdateEnums::Error::ProtocolInvalidOperationError:
        errorString = QString::fromUtf8("The requested operation is invalid for this protocol.");
        break;
    case QAppImageUpdateEnums::Error::UnknownNetworkError:
        errorString = QString::fromUtf8("An unknown network-related error was detected.");
        break;
    case QAppImageUpdateEnums::Error::UnknownContentError:
        errorString = QString::fromUtf8("An unknown error related to the remote content was detected.");
        break;
    case QAppImageUpdateEnums::Error::ProtocolFailure:
        errorString = QString::fromUtf8("A breakdown in protocol was detected ");
        errorString += QString::fromUtf8("(parsing error, invalid or unexpected responses, etc.)");
        break;
    case QAppImageUpdateEnums::Error::UnknownServerError:
        errorString = QString::fromUtf8("An unknown error related to the server response was detected.");
        break;
    case QAppImageUpdateEnums::Error::NoAppimagePathGiven:
        errorString = QString::fromUtf8("No AppImage given.");
        break;
    case QAppImageUpdateEnums::Error::AppimageNotReadable:
        errorString = QString::fromUtf8("The AppImage is not readable.");
        break;
    case QAppImageUpdateEnums::Error::NoReadPermission:
        errorString = QString::fromUtf8("You don't have the permission to read the AppImage.");
        break;
    case QAppImageUpdateEnums::Error::AppimageNotFound:
        errorString = QString::fromUtf8("The AppImage does not exist.");
        break;
    case QAppImageUpdateEnums::Error::CannotOpenAppimage:
        errorString = QString::fromUtf8("The AppImage cannot be opened.");
        break;
    case QAppImageUpdateEnums::Error::EmptyUpdateInformation:
        errorString = QString::fromUtf8("The AppImage does not include any update information.");
        break;
    case QAppImageUpdateEnums::Error::InvalidAppimageType:
        errorString = QString::fromUtf8("The AppImage has an unknown type.");
        break;
    case QAppImageUpdateEnums::Error::InvalidMagicBytes:
        errorString = QString::fromUtf8("The AppImage is not valid.");
        break;
    case QAppImageUpdateEnums::Error::InvalidUpdateInformation:
        errorString = QString::fromUtf8("The AppImage has invalid update information.");
        break;
    case QAppImageUpdateEnums::Error::NotEnoughMemory:
        errorString = QString::fromUtf8("Not enough memory.");
        break;
    case QAppImageUpdateEnums::Error::SectionHeaderNotFound:
        errorString = QString::fromUtf8("The AppImage does not contain update information ");
        errorString += QString::fromUtf8("at a valid section header.");
        break;
    case QAppImageUpdateEnums::Error::UnsupportedElfFormat:
        errorString = QString::fromUtf8("The AppImage is not in supported ELF format.");
        break;
    case QAppImageUpdateEnums::Error::UnsupportedTransport:
        errorString = QString::fromUtf8("The AppImage specifies an unsupported update transport.");
        break;
    case QAppImageUpdateEnums::Error::IoReadError:
        errorString = QString::fromUtf8("Unknown IO read error.");
        break;
    case QAppImageUpdateEnums::Error::GithubApiRateLimitReached:
        errorString = QString::fromUtf8("GitHub API rate limit reached, please try again later.");
        break;
    case QAppImageUpdateEnums::Error::ErrorResponseCode:
        errorString = QString::fromUtf8("Bad response from the server, please try again later.");
        break;
    case QAppImageUpdateEnums::Error::NoMarkerFoundInControlFile:
    case QAppImageUpdateEnums::Error::InvalidZsyncHeadersNumber:
    case QAppImageUpdateEnums::Error::InvalidZsyncMakeVersion:
    case QAppImageUpdateEnums::Error::InvalidZsyncTargetFilename:
    case QAppImageUpdateEnums::Error::InvalidZsyncMtime:
    case QAppImageUpdateEnums::Error::InvalidZsyncBlocksize:
    case QAppImageUpdateEnums::Error::InvalidTargetFileLength:
    case QAppImageUpdateEnums::Error::InvalidHashLengthLine:
    case QAppImageUpdateEnums::Error::InvalidHashLengths:
    case QAppImageUpdateEnums::Error::InvalidTargetFileUrl:
    case QAppImageUpdateEnums::Error::InvalidTargetFileSha1:
    case QAppImageUpdateEnums::Error::HashTableNotAllocated:
    case QAppImageUpdateEnums::Error::InvalidTargetFileChecksumBlocks:
    case QAppImageUpdateEnums::Error::CannotOpenTargetFileChecksumBlocks:
    case QAppImageUpdateEnums::Error::CannotConstructHashTable:
    case QAppImageUpdateEnums::Error::QbufferIoReadError:
        errorString = QString::fromUtf8("Invalid zsync meta file.");
        break;
    case QAppImageUpdateEnums::Error::ZsyncControlFileNotFound:
        errorString = QString::fromUtf8("The zsync control file was not found in the specified location.");
        break;
    case QAppImageUpdateEnums::Error::SourceFileNotFound:
        errorString = QString::fromUtf8("The current AppImage could not be found, maybe it was deleted while updating?");
        break;
    case QAppImageUpdateEnums::Error::NoPermissionToReadSourceFile:
        errorString = QString::fromUtf8("You don't have the permission to read the current AppImage.");
        break;
    case QAppImageUpdateEnums::Error::CannotOpenSourceFile:
        errorString = QString::fromUtf8("The current AppImage cannot be opened.");
        break;
    case QAppImageUpdateEnums::Error::NoPermissionToReadWriteTargetFile:
        errorString = QString::fromUtf8("You have no write or read permissions for the new version.");
        break;
    case QAppImageUpdateEnums::Error::CannotOpenTargetFile:
        errorString = QString::fromUtf8("The new version cannot be opened to write or read.");
        break;
    case QAppImageUpdateEnums::Error::TargetFileSha1HashMismatch:
        errorString = QString::fromUtf8("The newly constructed AppImage failed the integrity check, please try again.");
        break;
    case QAppImageUpdateEnums::Error::TorrentNotSupported:
	errorString = QString::fromUtf8("The AppImage author does not support decentralized update.");
	break;
    case QAppImageUpdateEnums::Error::TorrentSeedFailed:
	errorString = QString::fromUtf8("The AppImage cannot be seeded.");
	break;
    case QAppImageUpdateEnums::Error::OutdatedAppImageForSeed:
	errorString = QString::fromUtf8("The AppImage is not the newest available for seeding.");
	break;
    case QAppImageUpdateEnums::Error::IncompleteAppImageForSeed:
	errorString = QString::fromUtf8("The AppImage is incomplete for seeding.");
	break;
    case QAppImageUpdateEnums::Error::UnsupportedActionForBuild:
        errorString = QString::fromUtf8("The current build of the core library does not support the requested action.");
        break;
    case QAppImageUpdateEnums::Error::InvalidAction:
        errorString = QString::fromUtf8("The requested action is invalid.");
        break;
    default:
        errorString = QString::fromUtf8("Unknown error.");
        break;
    }
    return errorString;
}

QString QAppImageUpdatePrivate::versionString() {
    return QString::fromUtf8("2.0.0");
}
