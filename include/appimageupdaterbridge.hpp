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
 * @filename    : appimageupdaterbridge.hpp
*/
#ifndef APPIMAGE_UPDATER_BRIDGE_HPP_INCLUDED
#define APPIMAGE_UPDATER_BRIDGE_HPP_INCLUDED
#include "appimageupdaterbridge_enums.hpp"
#include "appimagedeltarevisioner.hpp"

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
#endif // APPIMAGE_UPDATER_BRIDGE_HPP_INCLUDED
