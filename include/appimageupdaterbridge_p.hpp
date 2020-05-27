#ifndef APPIMAGE_UPDATER_BRIDGE_PRIVATE_HPP_INCLUDED
#define APPIMAGE_UPDATER_BRIDGE_PRIVATE_HPP_INCLUDED
#ifdef BUILD_AS_PLUGIN
#include <QtPlugin>
#include <QMutex>
#include "appimageupdaterbridgeinterface.hpp"

namespace AppImageUpdaterBridge {
	class AppImageUpdaterBridge : public QObject, AppImageUpdaterBridgeInterface {
		Q_OBJECT
		Q_PLUGIN_METADATA(IID AppImageUpdaterBridgeInterface_iid FILE "AppImageUpdaterBridge.json")
		Q_INTERFACES(AppImageUpdaterBridgeInterface)

		public:
			AppImageUpdaterBridge(QObject *parent = nullptr);
			~AppImageUpdaterBridge();
		public Q_SLOTS:
			void start();
			void cancel();
			void setAppImage(const QString&);
			void setShowLog(bool);
			void setOutputDirectory(const QString&);
			void setProxy(const QNetworkProxy&);
			void checkForUpdate();
			void clear();

			
			void setGUIIcon(QPixmap);
			void setGUIApplicationName(const QString&);
			void setGUIFlags(int);
			void setGUIDelay(int);	
			QDialog *initGUI();
			int getGUIFlag(const QString&);
		Q_SIGNALS:
			void requiresAuthorization(QString, short, QString);
			void guiStarted();
			void guiCanceled();
			void guiFinished(QJsonObject);
			void guiError(QString, short);
			void quit();
			void started();
			void canceled();
			void finished(QJsonObject, QString);
			void updateAvailable(bool, QJsonObject);
			void error(short);
			void progress(int, qint64, qint64, double, QString);
			void logger(QString, QString);
		private:
			bool b_DeactivateHeadless = false;
			QPixmap m_Icon;
			QString s_ApplicationName;
			int n_Flags = -1, n_Delay = -1;

			QMutex m_Mutex;
			QDialog *m_PrevDialog = nullptr;
			AppImageDeltaRevisioner *m_Updater;
	};
}
#endif // BUILD_AS_PLUGIN
#endif // APPIMAGE_UPDATER_BRIDGE_PRIVATE_HPP_INCLUDED
