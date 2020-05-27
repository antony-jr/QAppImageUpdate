#include <QCoreApplication>

#include "../include/appimageupdaterbridge.hpp"
#include "../include/appimageupdaterbridge_p.hpp"
#include "../include/appimageupdaterdialog.hpp"

using AppImageUpdaterBridge::AppImageDeltaRevisioner;
using AppImageUpdaterBridge::AppImageUpdaterDialog;

// This definition makes the class implementation sane since we are using the 
// same name for the namespace and class name.
typedef AppImageUpdaterBridge::AppImageUpdaterBridge ClassAppImageUpdaterBridge;

ClassAppImageUpdaterBridge::AppImageUpdaterBridge(QObject *parent)
		: QObject(parent),
		  m_Updater(new AppImageDeltaRevisioner(true, this))
{	
	connect(m_Updater, &AppImageDeltaRevisioner::started,
	[&](){
		emit started();
		return;
	});
	
	connect(m_Updater, &AppImageDeltaRevisioner::canceled,
	[&](){
		emit canceled();
		return;
	});
	connect(m_Updater, &AppImageDeltaRevisioner::finished,
	[&](QJsonObject a, QString b){
		emit finished(a, b);
		return;
	});
	connect(m_Updater, &AppImageDeltaRevisioner::updateAvailable,
	[&](bool a, QJsonObject b){
		emit updateAvailable(a, b);
		return;
	});
	connect(m_Updater, &AppImageDeltaRevisioner::error,
	[&](short a){
		emit error(a);
		return;
	});
	connect(m_Updater, &AppImageDeltaRevisioner::progress,
	[&](int a, qint64 b, qint64 c, double d, QString e){
		emit progress(a,b,c,d,e);
		return;
	});
	connect(m_Updater, &AppImageDeltaRevisioner::logger,
	[&](QString a, QString b){
		emit logger(a,b);	
		return;
	});
}

ClassAppImageUpdaterBridge::~AppImageUpdaterBridge() {
	m_Updater->clear();
	m_Updater->deleteLater();
	if(m_PrevDialog)
		m_PrevDialog->deleteLater();
}

// Public Slots
void ClassAppImageUpdaterBridge::start() {
        m_Mutex.lock();
	if(b_DeactivateHeadless){
		m_Mutex.unlock();
		return;
	}	
	m_Mutex.unlock();
	m_Updater->start(); 
}
void ClassAppImageUpdaterBridge::cancel(){ 
	m_Mutex.lock();
	if(b_DeactivateHeadless){
		m_Mutex.unlock();
		return;
	}
	m_Mutex.unlock();
	m_Updater->cancel(); 
}
void ClassAppImageUpdaterBridge::setAppImage(const QString &appimage){ 
	m_Mutex.lock();
	if(b_DeactivateHeadless){
		m_Mutex.unlock();
		return;
	}		
	m_Mutex.unlock();
	m_Updater->setAppImage(appimage); 
}
void ClassAppImageUpdaterBridge::setShowLog(bool v) {	
	m_Updater->setShowLog(v); 
}
void ClassAppImageUpdaterBridge::setOutputDirectory(const QString &dir){ 
	m_Mutex.lock();
	if(b_DeactivateHeadless){
		m_Mutex.unlock();
		return;
	}	
	m_Mutex.unlock();
	m_Updater->setOutputDirectory(dir); 
}
void ClassAppImageUpdaterBridge::setProxy(const QNetworkProxy &proxy){ 
	 m_Mutex.lock();
	if(b_DeactivateHeadless){
		m_Mutex.unlock();
		return;
	}
	m_Mutex.unlock();
	m_Updater->setProxy(proxy); 
}
void ClassAppImageUpdaterBridge::checkForUpdate() { 
	m_Mutex.lock();
	if(b_DeactivateHeadless){
		m_Mutex.unlock();
		return;
	}	
	m_Mutex.unlock();
	m_Updater->checkForUpdate(); 
}

void ClassAppImageUpdaterBridge::clear(){ 	
	m_Mutex.lock();
	s_ApplicationName.clear();
	n_Flags = n_Delay = -1;
	b_DeactivateHeadless = false;
	m_Updater->cancel();
	m_Updater->clear();
	if(m_PrevDialog){
		m_PrevDialog->deleteLater();
		m_PrevDialog = nullptr;
	}
	m_Mutex.unlock();
}


// GUI Api
void ClassAppImageUpdaterBridge::setGUIIcon(QPixmap icon){
	m_Mutex.lock();
	m_Icon = icon;
	m_Mutex.unlock();
}

void ClassAppImageUpdaterBridge::setGUIApplicationName(const QString &applicationName){
	m_Mutex.lock();
	s_ApplicationName = applicationName;
	m_Mutex.unlock();
}

void ClassAppImageUpdaterBridge::setGUIFlags(int flags){
	m_Mutex.lock();
	n_Flags = flags;
	m_Mutex.unlock();
}	

void ClassAppImageUpdaterBridge::setGUIDelay(int delay){
	m_Mutex.lock();
	n_Delay = delay;
	m_Mutex.unlock();
}

QDialog *ClassAppImageUpdaterBridge::initGUI() {
	m_Mutex.lock();
	if(n_Flags == -1)
		n_Flags = AppImageUpdaterDialog::Default;
	if(n_Delay == -1)
		n_Delay = 10;
	if(s_ApplicationName.isEmpty())
		s_ApplicationName = QCoreApplication::applicationName();
	AppImageUpdaterDialog *r = new AppImageUpdaterDialog(m_Icon, nullptr, n_Flags, n_Delay);
	m_Updater->disconnect();
	m_Updater->cancel();

	b_DeactivateHeadless = true;
	if(m_PrevDialog){
		m_PrevDialog->deleteLater();
		m_PrevDialog = nullptr;
	}
	r->init(m_Updater, s_ApplicationName);
	m_PrevDialog = r;
	m_Mutex.unlock();
	return static_cast<QDialog *>(r);
}

int ClassAppImageUpdaterBridge::getGUIFlag(const QString &flagName){
	if(flagName == QString::fromUtf8("Default")){
		return AppImageUpdaterDialog::Default;	
	}
	if(flagName == QString::fromUtf8("ShowProgressDialog")){
		return AppImageUpdaterDialog::ShowProgressDialog;
	}

	if(flagName == QString::fromUtf8("ShowUpdateConfirmationDialog")){	
		return AppImageUpdaterDialog::ShowUpdateConfirmationDialog;
	}
        if(flagName == QString::fromUtf8("ShowFinishedDialog")){	
		return AppImageUpdaterDialog::ShowFinishedDialog;
	}
        if(flagName == QString::fromUtf8("ShowErrorDialog")){	
		return AppImageUpdaterDialog::ShowErrorDialog;
	}
        if(flagName == QString::fromUtf8("AlertWhenAuthorizationRequired")){	
		return AppImageUpdaterDialog::AlertWhenAuthorizationIsRequired;
	}
	if(flagName == QString::fromUtf8("NotifyWhenNoUpdateIsAvailable")){	
		return AppImageUpdaterDialog::NotifyWhenNoUpdateIsAvailable;
	}
	if(flagName == QString::fromUtf8("NoRemindMeLaterButton")){	
		return AppImageUpdaterDialog::NoRemindMeLaterButton;
	}
	if(flagName == QString::fromUtf8("NoSkipThisVersionButton")){	
		return AppImageUpdaterDialog::NoSkipThisVersionButton;
	}
	return -1;
}	
