#ifndef APPIMAGE_UPDATER_BRIDGE_PRIVATE_HPP_INCLUDED
#define APPIMAGE_UPDATER_BRIDGE_PRIVATE_HPP_INCLUDED
#ifdef BUILD_AS_PLUGIN
#include <QtPlugin>
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
			//void setAppImage(QFile*);
			void setShowLog(bool);
			void setOutputDirectory(const QString&);
			void setProxy(const QNetworkProxy&);
			void checkForUpdate();
			void clear();
		Q_SIGNALS:
			void started();
			void canceled();
			void finished(QJsonObject, QString);
			void updateAvailable(bool, QJsonObject);
			void error(short);
			void progress(int, qint64, qint64, double, QString);
			void logger(QString, QString);
		private:
			AppImageDeltaRevisioner *m_Updater;
	};
}
#endif // BUILD_AS_PLUGIN
#endif // APPIMAGE_UPDATER_BRIDGE_PRIVATE_HPP_INCLUDED
