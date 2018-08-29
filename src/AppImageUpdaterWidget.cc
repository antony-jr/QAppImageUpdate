#include <AppImageUpdaterWidget.hpp>

#define THREAD_SAFE_AREA(code , m) if(!m.tryLock()) { \
					return; \
				  } \
				  code \
				  m.unlock();

#define CODE_BLOCK(code) { code }

using namespace AppImageUpdaterBridge;

AppImageUpdaterWidget::AppImageUpdaterWidget(int idleSeconds , QWidget *parent)
	: QWidget(parent)
{
        if (this->objectName().isEmpty())
            this->setObjectName(QStringLiteral("AppImageUpdaterWidget"));
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
        _pIconLbl->setScaledContents(true);
        _pIconLbl->setAlignment(Qt::AlignCenter);
        _pGridLayout->addWidget(_pIconLbl, 0, 0, 3, 1);

	/* Delta Revisioner. */
	_pDRevisioner = new AppImageDeltaRevisioner(/*single threaded=*/false , /*parent=*/this);

	/* Idle Timer. */
	_pIdleTimer.setInterval(idleSeconds * 1000/*mili seconds.*/);
	_pIdleTimer.setSingleShot(true);
	connect(&_pIdleTimer , &QTimer::timeout , this , &AppImageUpdaterWidget::handleIdleTimerTimeout);

	/* Translations. */
        this->setWindowTitle(QString::fromUtf8("Updating... "));
        _pCancelBtn->setText(QString::fromUtf8("Cancel"));

	/* Program Logic. */
	connect(_pCancelBtn , &QPushButton::pressed , _pDRevisioner , &AppImageDeltaRevisioner::cancel , Qt::QueuedConnection);
	connect(_pDRevisioner , &AppImageDeltaRevisioner::canceled , this , &QWidget::hide , Qt::QueuedConnection);
	connect(_pDRevisioner , &AppImageDeltaRevisioner::canceled , this , &AppImageUpdaterWidget::canceled , Qt::DirectConnection);
	connect(_pDRevisioner , &AppImageDeltaRevisioner::updateAvailable , this , &AppImageUpdaterWidget::handleUpdateAvailable);
	connect(_pDRevisioner , &AppImageDeltaRevisioner::error , this , &AppImageUpdaterWidget::handleError);
	connect(_pDRevisioner , &AppImageDeltaRevisioner::started , this , &AppImageUpdaterWidget::started , Qt::DirectConnection);
	connect(_pDRevisioner , &AppImageDeltaRevisioner::finished , this , &AppImageUpdaterWidget::handleFinished);
	connect(_pDRevisioner , &AppImageDeltaRevisioner::progress , this , &AppImageUpdaterWidget::handleProgress);
	return;
}

AppImageUpdaterWidget::~AppImageUpdaterWidget()
{
	/*
	 * Thanks to Qt's parent to child deallocation ,
	 * We don't need to deallocate any QObject with a 
	 * parent.
	*/
	 return;
}

void AppImageUpdaterWidget::init(void)
{
	THREAD_SAFE_AREA(
		if(_bShowBeforeStarted){
			showWidget();
		}
		/* Start the timer. */
		_pIdleTimer.start();
		/* Set the label. */
  		_pStatusLbl->setText(QString::fromUtf8("Preparing for update... "));
	, _pMutex)
	return;
}

void AppImageUpdaterWidget::setAppImage(const QString &path)
{
	THREAD_SAFE_AREA(
		if(!path.isEmpty())
			_pDRevisioner->setAppImage(path);
	, _pMutex)
	return;
}

void AppImageUpdaterWidget::setAppImage(QFile *AppImage)
{
	THREAD_SAFE_AREA(
		if(AppImage)
			_pDRevisioner->setAppImage(AppImage);
	, _pMutex)
	return;
}

void AppImageUpdaterWidget::setShowBeforeStarted(bool doShow)
{
	THREAD_SAFE_AREA(
		_bShowBeforeStarted = doShow;
	, _pMutex)
	return;
}

void AppImageUpdaterWidget::setIconPixmap(const QPixmap &pixmap)
{
	THREAD_SAFE_AREA(
		_pIconLbl->setPixmap(pixmap);
	, _pMutex)
	return;
}

void AppImageUpdaterWidget::resetIdleTimer(void)
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

void AppImageUpdaterWidget::showWidget(void)
{
	if(_pIconLbl->pixmap() == 0){ /* check if we have any pixmap given by the user. */
	/* If not then don't show the icon label itself.*/
	_pIconLbl->setVisible(false);
	}else{
	/* If so then show it. */
	_pIconLbl->setVisible(true);
	}
	this->show();
	return;
}

void AppImageUpdaterWidget::handleIdleTimerTimeout(void)
{
	/* Check for updates when the timer calls for it. */
	_pIdleTimer.stop();
	_pStatusLbl->setText(QString::fromUtf8("Checking for Update... "));
	_pDRevisioner->checkForUpdate();
	return;
}

void AppImageUpdaterWidget::handleUpdateAvailable(bool isUpdateAvailable , QJsonObject CurrentAppImageInfo)
{
	bool confirmed = false;
	this->setWindowTitle(QString::fromUtf8("Updating ") + 
			     QFileInfo(CurrentAppImageInfo["AppImageFilePath"].toString()).baseName() + 
			     QString::fromUtf8("... "));

	if(isUpdateAvailable){
		confirmed = continueWithUpdate(CurrentAppImageInfo);
	}else{
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
	if(confirmed){
		_pDRevisioner->start();
		showWidget();
	}
	return;
}
    
void AppImageUpdaterWidget::handleError(short errorCode)
{
	emit error(QString() , errorCode);
	return;
}

void AppImageUpdaterWidget::handleFinished(QJsonObject newVersion, QString oldVersionPath)
{
	(void)oldVersionPath;
	_pStatusLbl->setText(QString::fromUtf8("Finalizing Update... "));
	if(openNewVersion(newVersion)){
	QFileInfo info(newVersion["AbsolutePath"].toString());
	if(!info.isExecutable()){
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
	connect(process , &QProcess::started , this , &AppImageUpdaterWidget::quit , Qt::DirectConnection);
	}
	emit finished(newVersion);
	return;
}
    
void AppImageUpdaterWidget::handleProgress(int percent, 
					   qint64 bytesReceived, 
					   qint64 bytesTotal, 
					   double speed, 
					   QString units)
{
	_pProgressBar->setValue(percent);
	QString statusText("Updating ");
	auto MegaBytesReceived = bytesReceived / 1048576,
	     MegaBytesTotal = bytesTotal / 1048576;
	statusText.append(QString::number(MegaBytesReceived) + 
			  QString::fromUtf8(" MiB of ") + 
			  QString::number(MegaBytesTotal) +
			  QString::fromUtf8(" MiB at "));
	statusText.append(QString::number(speed) + QString::fromUtf8(" ") + units + QString::fromUtf8("."));
	_pStatusLbl->setText(statusText);
	return;
}


