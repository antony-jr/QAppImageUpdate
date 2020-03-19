#include "../include/appimageupdaterbridge.hpp"

using AppImageUpdaterBridge::AppImageDeltaRevisioner;

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
	m_Updater->cancel();
	m_Updater->deleteLater();
}

// Public Slots
void ClassAppImageUpdaterBridge::start() { m_Updater->start(); }
void ClassAppImageUpdaterBridge::cancel(){ m_Updater->cancel(); }
void ClassAppImageUpdaterBridge::setAppImage(const QString &appimage){ m_Updater->setAppImage(appimage); }
//void ClassAppImageUpdaterBridge::setAppImage(QFile *appimage){ m_Updater->setAppImage(appimage); }	
void ClassAppImageUpdaterBridge::setShowLog(bool v) { m_Updater->setShowLog(v); }
void ClassAppImageUpdaterBridge::setOutputDirectory(const QString &dir){ m_Updater->setOutputDirectory(dir); }
void ClassAppImageUpdaterBridge::setProxy(const QNetworkProxy &proxy){ m_Updater->setProxy(proxy); }
void ClassAppImageUpdaterBridge::checkForUpdate() { m_Updater->checkForUpdate(); }
void ClassAppImageUpdaterBridge::clear(){ m_Updater->clear(); }
	
