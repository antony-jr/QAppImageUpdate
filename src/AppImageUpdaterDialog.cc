#include <AppImageUpdaterDialog.hpp>

#define THREAD_SAFE_AREA(code , m) if(m.tryLock()) { \
				      code \
				      m.unlock(); \
				   }

#define CODE_BLOCK(code) { code }

using namespace AppImageUpdaterBridge;

AppImageUpdaterDialog::AppImageUpdaterDialog(int idleSeconds, QWidget *parent)
    : QDialog(parent)
{
    if (this->objectName().isEmpty())
        this->setObjectName(QStringLiteral("AppImageUpdaterDialog"));
    this->resize(420, 120);

    /* Fixed window size. */
    this->setMinimumSize(QSize(420, 120));
    this->setMaximumSize(QSize(420, 120));

    _pGridLayout = new QGridLayout(this);
    _pGridLayout->setObjectName(QStringLiteral("MainGridLayout"));

    /* Cancel Update. */
    _pCancelBtn = new QPushButton(this);
    _pCancelBtn->setObjectName(QStringLiteral("CancelButton"));
    _pGridLayout->addWidget(_pCancelBtn, 2, 1, 1, 1);

    /* Update Status. */
    _pStatusLbl = new QLabel(this);
    _pStatusLbl->setObjectName(QStringLiteral("StatusLabel"));
    _pGridLayout->addWidget(_pStatusLbl, 1, 1, 1, 1);

    /* Update Progress. */
    _pProgressBar = new QProgressBar(this);
    _pProgressBar->setObjectName(QStringLiteral("ProgressBar"));
    _pProgressBar->setValue(0);
    _pGridLayout->addWidget(_pProgressBar, 0, 1, 1, 1);

    /* AppImage Icon. */
    _pIconLbl = new QLabel(this);
    _pIconLbl->setObjectName(QStringLiteral("IconLabel"));
    _pIconLbl->setMinimumSize(QSize(100, 100));
    _pIconLbl->setMaximumSize(QSize(100, 100));
    _pIconLbl->setScaledContents(true);
    _pIconLbl->setAlignment(Qt::AlignCenter);
    _pGridLayout->addWidget(_pIconLbl, 0, 0, 3, 1);

    /* Delta Revisioner. */
    _pDRevisioner = new AppImageDeltaRevisioner(/*single threaded=*/false, /*parent=*/this);

    /* Idle Timer. */
    _pIdleTimer.setInterval(idleSeconds * 1000/*mili seconds.*/);
    _pIdleTimer.setSingleShot(true);
    connect(&_pIdleTimer, &QTimer::timeout, this, &AppImageUpdaterDialog::handleIdleTimerTimeout);

    /* Translations. */
    this->setWindowTitle(QString::fromUtf8("Updating... "));
    _pCancelBtn->setText(QString::fromUtf8("Cancel"));

    /* Program Logic. */
    connect(_pCancelBtn, &QPushButton::pressed, _pDRevisioner, &AppImageDeltaRevisioner::cancel, Qt::QueuedConnection);
    connect(_pDRevisioner, &AppImageDeltaRevisioner::canceled, this, &QDialog::hide, Qt::QueuedConnection);
    connect(_pDRevisioner, &AppImageDeltaRevisioner::canceled, this, &AppImageUpdaterDialog::canceled, Qt::DirectConnection);
    connect(_pDRevisioner, &AppImageDeltaRevisioner::updateAvailable, this, &AppImageUpdaterDialog::handleUpdateAvailable);
    connect(_pDRevisioner, &AppImageDeltaRevisioner::error, this, &AppImageUpdaterDialog::handleError);
    connect(_pDRevisioner, &AppImageDeltaRevisioner::started, this, &AppImageUpdaterDialog::started, Qt::DirectConnection);
    connect(_pDRevisioner, &AppImageDeltaRevisioner::finished, this, &AppImageUpdaterDialog::handleFinished);
    connect(_pDRevisioner, &AppImageDeltaRevisioner::progress, this, &AppImageUpdaterDialog::handleProgress);
    return;
}

AppImageUpdaterDialog::~AppImageUpdaterDialog()
{
    /*
     * Thanks to Qt's parent to child deallocation ,
     * We don't need to deallocate any QObject with a
     * parent.
    */
    return;
}

void AppImageUpdaterDialog::init(void)
{
    THREAD_SAFE_AREA(
    if(_bShowBeforeStarted) {
    showWidget();
    }
    /* Start the timer. */
    _pIdleTimer.start();
    /* Set the label. */
    _pStatusLbl->setText(QString::fromUtf8("Preparing for update... "));
    , _pMutex);
    return;
}

void AppImageUpdaterDialog::setAppImage(const QString &path)
{
    THREAD_SAFE_AREA(
        if(!path.isEmpty())
        _pDRevisioner->setAppImage(path);
        _sCurrentAppImagePath = QFileInfo(path).absolutePath();
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::setAppImage(QFile *AppImage)
{
    THREAD_SAFE_AREA(
        if(AppImage)
        _pDRevisioner->setAppImage(AppImage);
        _sCurrentAppImagePath = QFileInfo(AppImage->fileName()).absolutePath();
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::setMovePoint(const QPoint &pt)
{
    THREAD_SAFE_AREA(
        _pMovePoint = pt;
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::setShowUpdateConfirmationDialog(bool doShow)
{
    THREAD_SAFE_AREA(
        _bShowUpdateConfirmationDialog = doShow;
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::setShowNoUpdateDialog(bool doShow)
{
    THREAD_SAFE_AREA(
        _bShowNoUpdateDialog = doShow;
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::setShowFinishDialog(bool doShow)
{
    THREAD_SAFE_AREA(
        _bShowFinishDialog = doShow;
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::setShowErrorDialog(bool doShow)
{
    THREAD_SAFE_AREA(
        _bShowErrorDialog = doShow;
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::setShowBeforeStarted(bool doShow)
{
    THREAD_SAFE_AREA(
        _bShowBeforeStarted = doShow;
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::setShowLog(bool choice)
{
    THREAD_SAFE_AREA(
        _pDRevisioner->setShowLog(choice);
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::setIconPixmap(const QPixmap &pixmap)
{
    THREAD_SAFE_AREA(
        _pIconLbl->setPixmap(pixmap);
        _pAppImageIcon = pixmap.scaled(100, 100, Qt::KeepAspectRatio);
        , _pMutex);
    return;
}

void AppImageUpdaterDialog::resetIdleTimer(void)
{
    /*
     * Start the timer only if it was active in the first
     * place.
    */
    if(_pIdleTimer.isActive())
        _pIdleTimer.start();
    return;
}

/*
 * Note: No need to use mutex inside private
 * slots.
*/

void AppImageUpdaterDialog::showWidget(void)
{
    if(_pIconLbl->pixmap() == 0) { /* check if we have any pixmap given by the user. */
        /* If not then don't show the icon label itself.*/
        _pIconLbl->setVisible(false);
    } else {
        /* If so then show it. */
        _pIconLbl->setVisible(true);
    }
    if(!_pMovePoint.isNull()) {
        this->move(_pMovePoint);
    }
    this->show();
    return;
}

void AppImageUpdaterDialog::handleIdleTimerTimeout(void)
{
    /* Check for updates when the timer calls for it. */
    _pIdleTimer.stop();
    _pStatusLbl->setText(QString::fromUtf8("Checking for Update... "));
    _pDRevisioner->checkForUpdate();
    return;
}

void AppImageUpdaterDialog::handleUpdateAvailable(bool isUpdateAvailable, QJsonObject CurrentAppImageInfo)
{
    bool confirmed = true;
    bool showUpdateDialog = false;
    bool showNoUpdateDialog = false;
    QMessageBox box(this);
    this->setWindowTitle(QString::fromUtf8("Updating ") +
                         QFileInfo(CurrentAppImageInfo["AppImageFilePath"].toString()).baseName() +
                         QString::fromUtf8("... "));

    THREAD_SAFE_AREA(
        showUpdateDialog = _bShowUpdateConfirmationDialog;
        showNoUpdateDialog = _bShowNoUpdateDialog;
        , _pMutex);

    if(isUpdateAvailable) {
        if(showUpdateDialog) {
            QString currentAppImageName = QFileInfo(CurrentAppImageInfo["AppImageFilePath"].toString()).fileName();
            QMessageBox box(this);
            box.setWindowTitle(QString::fromUtf8("Update Available!"));
            box.setIconPixmap(_pAppImageIcon);
            box.setText(QString::fromUtf8("A new version of ") +
                        currentAppImageName +
                        QString::fromUtf8(" is available , Do you want to update ?"));
            box.addButton(QMessageBox::Yes);
            box.addButton(QMessageBox::No);
            if(!_pMovePoint.isNull()) {
                box.move(_pMovePoint);
            }
            confirmed = (box.exec() == QMessageBox::Yes);
        }
    } else {
        if(showNoUpdateDialog) {
            QString currentAppImageName = QFileInfo(CurrentAppImageInfo["AppImageFilePath"].toString()).fileName();
            QMessageBox box(this);
            box.setWindowTitle(QString::fromUtf8("No Updates Available!"));
            box.setIconPixmap(_pAppImageIcon);
            box.setText(QString::fromUtf8("You are currently using the lastest version of ") +
                        currentAppImageName +
                        QString::fromUtf8("."));
            if(!_pMovePoint.isNull()) {
                box.move(_pMovePoint);
            }
            box.exec();
        }
        confirmed = false;
        emit finished(QJsonObject());
    }

    /*
     * If confirmed to update then start the
     * delta revisioner.
     *
     * Note: With the virtual method continueWithUpdate will always return
     * true unless or until the user overrides it to do something with it.
     * Like showing a message box to the user to confirm update.
    */
    if(confirmed) {
        _pDRevisioner->start();
        showWidget();
    } else {
        emit finished(QJsonObject());
    }
    return;
}

void AppImageUpdaterDialog::handleError(short errorCode)
{
    bool show = false;
    QString path,
            errorString;
    THREAD_SAFE_AREA(
        show = _bShowErrorDialog;
        path = _sCurrentAppImagePath;
        , _pMutex);

    /* Convert the error code to human readable code. */
    if(errorCode == AppImageUpdaterBridge::UNKNOWN_NETWORK_ERROR) {
        /* Expand network error even more. */
        switch(errorCode) {
        default:
            errorString = QString::fromUtf8("there was an unknown network error.");
            break;
        }
    } else {
        switch(errorCode) {
        case AppImageUpdaterBridge::APPIMAGE_NOT_READABLE:
            errorString = QString::fromUtf8("it is not readable.");
            break;
        case AppImageUpdaterBridge::NO_READ_PERMISSION:
            errorString = QString::fromUtf8("you don't have the permission to read it.");
            break;
        case AppImageUpdaterBridge::APPIMAGE_NOT_FOUND:
            errorString = QString::fromUtf8("it does not exists.");
            break;
        case AppImageUpdaterBridge::CANNOT_OPEN_APPIMAGE:
            errorString = QString::fromUtf8("it cannot be opened.");
            break;
        case AppImageUpdaterBridge::EMPTY_UPDATE_INFORMATION:
            errorString = QString::fromUtf8("the author of this AppImage did not include any update information.");
            break;
        case AppImageUpdaterBridge::INVALID_APPIMAGE_TYPE:
            errorString = QString::fromUtf8("it is an unknown AppImage type.");
            break;
        case AppImageUpdaterBridge::INVALID_MAGIC_BYTES:
            errorString = QString::fromUtf8("it is not a AppImage , Make sure to give valid AppImage.");
            break;
        case AppImageUpdaterBridge::INVALID_UPDATE_INFORMATION:
            errorString = QString::fromUtf8("the author of this AppImage included invalid update information.");
            break;
        case AppImageUpdaterBridge::NOT_ENOUGH_MEMORY:
            errorString = QString::fromUtf8("there is no enough memory left in your ram.");
            break;
        case AppImageUpdaterBridge::SECTION_HEADER_NOT_FOUND:
            errorString = QString::fromUtf8("the author of this AppImage did not embed the update information ");
            errorString += QString::fromUtf8("at a valid section header.");
            break;
        case AppImageUpdaterBridge::UNSUPPORTED_ELF_FORMAT:
            errorString = QString::fromUtf8("the given AppImage is not in supported elf format.");
            break;
        case AppImageUpdaterBridge::UNSUPPORTED_TRANSPORT:
            errorString = QString::fromUtf8("the author of this included an unsupported transport in update information.");
            break;
        case AppImageUpdaterBridge::IO_READ_ERROR:
            errorString = QString::fromUtf8("there was a unknown IO read error.");
            break;
        case AppImageUpdaterBridge::GITHUB_API_RATE_LIMIT_REACHED:
            errorString = QString::fromUtf8("github api rate limit was reached , Please try again after an hour.");
            break;
        case AppImageUpdaterBridge::ERROR_RESPONSE_CODE:
            errorString = QString::fromUtf8("some request returned a bad server response code , Please try again.");
            break;
        case AppImageUpdaterBridge::NO_MARKER_FOUND_IN_CONTROL_FILE:
        case AppImageUpdaterBridge::INVALID_ZSYNC_HEADERS_NUMBER:
        case AppImageUpdaterBridge::INVALID_ZSYNC_MAKE_VERSION:
        case AppImageUpdaterBridge::INVALID_ZSYNC_TARGET_FILENAME:
        case AppImageUpdaterBridge::INVALID_ZSYNC_MTIME:
        case AppImageUpdaterBridge::INVALID_ZSYNC_BLOCKSIZE:
        case AppImageUpdaterBridge::INVALID_TARGET_FILE_LENGTH:
        case AppImageUpdaterBridge::INVALID_HASH_LENGTH_LINE:
        case AppImageUpdaterBridge::INVALID_HASH_LENGTHS:
        case AppImageUpdaterBridge::INVALID_TARGET_FILE_URL:
        case AppImageUpdaterBridge::INVALID_TARGET_FILE_SHA1:
        case AppImageUpdaterBridge::HASH_TABLE_NOT_ALLOCATED:
        case AppImageUpdaterBridge::INVALID_TARGET_FILE_CHECKSUM_BLOCKS:
        case AppImageUpdaterBridge::CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS:
        case AppImageUpdaterBridge::CANNOT_CONSTRUCT_HASH_TABLE:
        case AppImageUpdaterBridge::QBUFFER_IO_READ_ERROR:
            errorString = QString::fromUtf8("the author did not produce the zsync meta file properly , ");
            errorString += QString::fromUtf8("Please notify the author or else try again because it can be a ");
            errorString += QString::fromUtf8("false positive.");
            break;
        case AppImageUpdaterBridge::SOURCE_FILE_NOT_FOUND:
            errorString = QString::fromUtf8("the current AppImage is not found , maybe deleted while updating ?");
            break;
        case AppImageUpdaterBridge::NO_PERMISSION_TO_READ_SOURCE_FILE:
            errorString = QString::fromUtf8("you have no permissions to read the current AppImage.");
            break;
        case AppImageUpdaterBridge::CANNOT_OPEN_SOURCE_FILE:
            errorString = QString::fromUtf8("the current AppImage cannot be opened.");
            break;
        case AppImageUpdaterBridge::NO_PERMISSION_TO_READ_WRITE_TARGET_FILE:
            errorString = QString::fromUtf8("you have no write or read permissions to read and write the new version.");
            break;
        case AppImageUpdaterBridge::CANNOT_OPEN_TARGET_FILE:
            errorString = QString::fromUtf8("the new version cannot be opened to write or read.");
            break;
        case AppImageUpdaterBridge::TARGET_FILE_SHA1_HASH_MISMATCH:
            errorString = QString::fromUtf8("the newly construct AppImage failed to prove integrity , Try again.");
            break;
        default:
            errorString = QString::fromUtf8("there occured an uncaught error.");
            break;
        }
    }

    if(show) {
        QMessageBox box(this);
        box.setWindowTitle(QString::fromUtf8("Update Failed!"));
        box.setIcon(QMessageBox::Critical);
        box.setText(QString::fromUtf8("Update failed for '") +
                    path +
                    QString::fromUtf8("' , Because ") +
                    errorString);
        if(!_pMovePoint.isNull()) {
            box.move(_pMovePoint);
        }
        box.exec();
    }
    emit error(errorString, errorCode);
    return;
}

void AppImageUpdaterDialog::handleFinished(QJsonObject newVersion, QString oldVersionPath)
{
    (void)oldVersionPath;
    _pStatusLbl->setText(QString::fromUtf8("Finalizing Update... "));

    bool execute = false;
    bool show = false;
    THREAD_SAFE_AREA (
        show = _bShowFinishDialog;
        , _pMutex);

    if(show) {
        QString currentAppImageName = QFileInfo(oldVersionPath).fileName();
        QMessageBox box(this);
        box.setWindowTitle(QString::fromUtf8("Update Completed!"));
        box.setIconPixmap(_pAppImageIcon);
        box.setText(QString::fromUtf8("Update Completed successfully for ") +
                    currentAppImageName +
                    QString::fromUtf8(" , the new version is saved at '") +
                    newVersion["AbsolutePath"].toString() +
                    QString::fromUtf8("' , Do you want to open it ?"));
        box.addButton(QMessageBox::Yes);
        box.addButton(QMessageBox::No);
        if(!_pMovePoint.isNull()) {
            box.move(_pMovePoint);
        }
        execute = (box.exec() == QMessageBox::Yes);

    }

    if(execute) {
        QFileInfo info(newVersion["AbsolutePath"].toString());
        if(!info.isExecutable()) {
            CODE_BLOCK(
                QFile file(newVersion["AbsolutePath"].toString());
                file.setPermissions(QFileDevice::ExeUser |
                                    QFileDevice::ExeOther|
                                    QFileDevice::ExeGroup|
                                    info.permissions());
            )
        }
        QProcess *process = new QProcess(this);
        process->startDetached(newVersion["AbsolutePath"].toString());
        connect(process, &QProcess::started, this, &AppImageUpdaterDialog::quit, Qt::DirectConnection);
    }
    emit finished(newVersion);
    return;
}

void AppImageUpdaterDialog::handleProgress(int percent,
        qint64 bytesReceived,
        qint64 bytesTotal,
        double speed,
        QString units)
{
    _pProgressBar->setValue(percent);
    double MegaBytesReceived = bytesReceived / 1048576;
    if(!_nMegaBytesTotal) {
        _nMegaBytesTotal = bytesTotal / 1048576;
    }
    const QString progressTemplate = QString::fromUtf8("Updating %1 MiB of %2 MiB at %3 %4...");
    QString statusText = progressTemplate.arg(MegaBytesReceived).arg(_nMegaBytesTotal).arg(speed).arg(units);
    _pStatusLbl->setText(statusText);
    return;
}


