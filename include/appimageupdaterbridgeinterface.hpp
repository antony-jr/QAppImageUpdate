#ifndef APPIMAGE_UPDATER_BRIDGE_INTERFACE_HPP_INCLUDED 
#define APPIMAGE_UPDATER_BRIDGE_INTERFACE_HPP_INCLUDED 
#include <QtPlugin>
#include <QString>
#include <QFile>
#include <QNetworkProxy>
#include <QJsonObject>
#include <QDialog>
#include <QWidget>
#include <QPixmap>

class AppImageUpdaterBridgeInterface {
public Q_SLOTS:
	virtual void start() = 0;
	virtual void cancel() = 0;
	virtual void setAppImage(const QString&) = 0;
	virtual void setShowLog(bool) = 0;
	virtual void setOutputDirectory(const QString&) = 0;
	virtual void setProxy(const QNetworkProxy&) = 0; 
	virtual void checkForUpdate() = 0;
	virtual void clear() = 0;

	virtual void setGUIIcon(QPixmap) = 0;
	virtual void setGUIApplicationName(const QString&) = 0;
	virtual void setGUIFlags(int) = 0;
	virtual void setGUIDelay(int) = 0;	
	virtual QDialog *initGUI() = 0;
	virtual int getGUIFlag(const QString&) = 0;
Q_SIGNALS:
	virtual void requiresAuthorization(QString, short, QString) = 0;
	virtual void guiStarted() = 0;
	virtual void guiCanceled() = 0;	
	virtual void guiFinished(QJsonObject) = 0;
	virtual void guiError(QString, short) = 0;
	virtual void quit() = 0;
	virtual void started() = 0;
	virtual void canceled() = 0;
	virtual void finished(QJsonObject, QString) = 0;
	virtual void updateAvailable(bool, QJsonObject) = 0;
	virtual void error(short) = 0;
	virtual void progress(int, qint64, qint64, double, QString) = 0;
	virtual void logger(QString, QString) = 0;
};

#ifndef AppImageUpdaterBridgeInterface_iid
#define AppImageUpdaterBridgeInterface_iid "com.antony-jr.AppImageUpdaterBridge"
#endif 

Q_DECLARE_INTERFACE(AppImageUpdaterBridgeInterface , AppImageUpdaterBridgeInterface_iid);

#endif // APPIMAGE_UPDATER_BRIDGE_INTERFACE_HPP_INCLUDED
