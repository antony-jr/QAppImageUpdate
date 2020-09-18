#ifndef APPIMAGE_DELTA_REVISIONER_TESTS_HPP_INCLUDED
#define APPIMAGE_DELTA_REVISIONER_TESTS_HPP_INCLUDED
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QAppImageUpdate>
#include <QScopedPointer>
#include <QDebug>
#include <QStringList>

#include "SimpleDownload.hpp"

class QAppImageUpdateTests : public QObject {
    Q_OBJECT
    QScopedPointer<QTemporaryDir> m_TempDir;
    QStringList m_Available;
  private slots:
    void initTestCase(void) {
	SimpleDownload downloader;

	m_TempDir.reset(new QTemporaryDir);
	if(!m_TempDir->isValid()) {
		QFAIL("Cannot create temporary directory");
		emit finished();
		return;
	}

	/// The testable AppImages
	QStringList urls;

	/// Github based AppImage Update
	urls /* Small AppImages */
	     << "https://github.com/AppImage/AppImageKit/releases/download/10/appimagetool-x86_64.AppImage"
	     /* Slightly larger AppImage */
	     << "https://github.com/antony-jr/AppImageUpdater/releases/download/14/AppImageUpdater-9b4000e-x86_64.AppImage" 
	     /* Largest AppImage */
	     << "https://github.com/FreeCAD/FreeCAD/releases/download/0.18.2/FreeCAD_0.18-16117-Linux-Conda_Py3Qt5_glibc2.12-x86_64.AppImage"
	     ;	  

	/// Bintray based AppImage Update
	urls /* Slightly larger AppImage */
	     << "https://bintray.com/probono/AppImages/download_file?file_path=Brackets-1.6.0.16680-x86_64.AppImage"
	     << "https://bintray.com/probono/AppImages/download_file?file_path=Calibre-2.58.0.glibc2.7-x86_64.AppImage"
	     << "https://bintray.com/probono/AppImages/download_file?file_path=Audacity-2.0.5.glibc2.15-x86_64.AppImage"
	     /* Large AppImage < 200 MiB */
	     << "https://bintray.com/probono/AppImages/download_file?file_path=Blender-2.78-x86_64.AppImage"
	     << "https://bintray.com/probono/AppImages/download_file?file_path=FreeCAD-0.17.git201709021132.glibc2.17-x86_64.AppImage"
	     /* Largest AppImage (Largest I could find, update it if you found something larger */
	     << "https://bintray.com/probono/AppImages/download_file?file_path=RetroArch-1.3.6%2Br455.glibc2.17-x86_64.AppImage"
	     ;

	
	/// Direct zsync AppImage Update
	urls << "https://releases.openclonk.org/snapshots/2020-08-08T17:14:06Z-master-dc43c2b72/OpenClonk-x86_64.AppImage";


	/// AppImage Update without range request support
	/// Cannot find anything for now. 

	/// Download the required testing AppImages
	int count = 1;
	for(auto iter = urls.begin(),
		 end = urls.end();
	 	 iter != end;
		 ++iter) {
		QString path = m_TempDir->path() + "/" + QString::number(count) + ".AppImage";

		if(!downloader.download(*iter, path)) {
			m_Available << path;
		}else {
			QWARN("Download Failed");
		}

		++count;
	}

	if(m_Available.size() == 0) {
		QFAIL("No AppImages to test");
	}
        return;
    }

    void cleanupTestCase(void) {
        m_TempDir->remove();
	emit finished();
        return;
    }
  Q_SIGNALS:
    void finished(void);
};
#endif
