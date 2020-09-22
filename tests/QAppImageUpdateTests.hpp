#ifndef APPIMAGE_DELTA_REVISIONER_TESTS_HPP_INCLUDED
#define APPIMAGE_DELTA_REVISIONER_TESTS_HPP_INCLUDED
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QAppImageUpdate>
#include <QScopedPointer>
#include <QDebug>
#include <QStringList>
#include <QCoreApplication>
#include <QJsonObject>
#include <QtConcurrent>
#include <QFuture>
#include <QEventLoop>

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
	urls /* Large AppImage < 200 MiB */
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

    void actionGetEmbeddedInfo() {
	QAppImageUpdate updater;
	updater.setAppImage(m_Available.at(0));
	connect(&updater, &QAppImageUpdate::error, this, &QAppImageUpdateTests::defaultErrorHandler);
	QSignalSpy spyInfo(&updater, SIGNAL(finished(QJsonObject, short)));
	
	updater.start(QAppImageUpdate::Action::GetEmbeddedInfo);

	spyInfo.wait(10 * 1000);

	QVERIFY(spyInfo.count() == 1);

        /* Get resultant QJsonObject and Compare. */
        auto sg = spyInfo.takeFirst();

	QJsonObject result = sg.at(0).toJsonObject();
	short action = sg.at(1).toInt();

	QVERIFY(action == QAppImageUpdate::Action::GetEmbeddedInfo);

        /* Check if the result has a json sub-object called 'FileInformation'.
         * If so then compare it with our test case file information.
         */
        auto fileInfo = result["FileInformation"].toObject();

        /* If the file info is empty then fail. */
        QVERIFY(!fileInfo.isEmpty());

        auto updateInfo = result["UpdateInformation"].toObject();

        /* if the update info is empty then fail. */
        QVERIFY(!updateInfo.isEmpty());

        /* both are not empty , Check the value for isEmpty in the resultant. */
        QVERIFY(!result["isEmpty"].toBool());
        return;
    }
    
    void actionGetEmbeddedInfoAll(void) {
	short action = 0;
	QJsonObject result,fileInfo,updateInfo;
	QList<QVariant> sg;
	QAppImageUpdate updater;
	connect(&updater, &QAppImageUpdate::error, this, &QAppImageUpdateTests::defaultErrorHandler);
	QSignalSpy spyInfo(&updater, SIGNAL(finished(QJsonObject, short)));

	for(auto iter = m_Available.begin(),
		 end = m_Available.end();
		 iter != end;
		 ++iter) {	 
	updater.setAppImage(*iter);
	qInfo().noquote() << "GetEmbeddedInfo(" << *iter << ")";

	updater.start(QAppImageUpdate::Action::GetEmbeddedInfo);
	
	spyInfo.wait(10 * 1000);
	QCOMPARE(spyInfo.count(), 1);

        /* Get resultant QJsonObject and Compare. */
	sg = spyInfo.takeFirst();
        result = sg.at(0).toJsonObject();
	action = sg.at(1).toInt();

	QVERIFY(action == QAppImageUpdate::Action::GetEmbeddedInfo);

        /* Check if the result has a json sub-object called 'FileInformation'.
         * If so then compare it with our test case file information.
         */
        fileInfo = result["FileInformation"].toObject();

        /* If the file info is empty then fail. */
        QVERIFY(!fileInfo.isEmpty());

        updateInfo = result["UpdateInformation"].toObject();

        /* if the update info is empty then fail. */
        QVERIFY(!updateInfo.isEmpty());

        /* both are not empty , Check the value for isEmpty in the resultant. */
        QVERIFY(!result["isEmpty"].toBool());
	}
    }
 
    void actionCheckForUpdate() {
        QAppImageUpdate updater;
	updater.setAppImage(m_Available.at(0));
	connect(&updater, &QAppImageUpdate::error, this, &QAppImageUpdateTests::defaultErrorHandler);

	QEventLoop loop;	
	QSignalSpy spyInfo(&updater, SIGNAL(finished(QJsonObject, short)));
	connect(&updater, &QAppImageUpdate::finished, &loop, &QEventLoop::quit, Qt::QueuedConnection);

	updater.start(QAppImageUpdate::Action::CheckForUpdate);
	loop.exec();

	QVERIFY(spyInfo.count() == 1);

	auto sg = spyInfo.takeFirst();
	QJsonObject result = sg.at(0).toJsonObject();
	short action = sg.at(1).toInt();

	QVERIFY(action == QAppImageUpdate::Action::CheckForUpdate);
	QVERIFY(result.contains("UpdateAvailable"));
    }
    
    void actionCheckForUpdateAll() {
        short action = 0;
	QJsonObject result;
	QList<QVariant> sg;
	QAppImageUpdate updater;
	connect(&updater, &QAppImageUpdate::error, this, &QAppImageUpdateTests::defaultErrorHandler);
	QSignalSpy spyInfo(&updater, SIGNAL(finished(QJsonObject, short)));
	QEventLoop loop;	
	connect(&updater, &QAppImageUpdate::finished, &loop, &QEventLoop::quit, Qt::QueuedConnection);
	
	for(auto iter = m_Available.begin(),
		 end = m_Available.end();
		 iter != end;
		 ++iter) { 	
	updater.setAppImage(*iter);
	qInfo().noquote() << "CheckForUpdate(" << *iter << ")";

	updater.start(QAppImageUpdate::Action::CheckForUpdate);
	loop.exec();

        QCOMPARE(spyInfo.count(), 1);

	sg = spyInfo.takeFirst();
	result = sg.at(0).toJsonObject();
	action = sg.at(1).toInt();

	QVERIFY(action == QAppImageUpdate::Action::CheckForUpdate);
	QVERIFY(result.contains("UpdateAvailable"));
	}
    }

    // Test the default action sequence
    // CheckForUpdate -> Update
    // Make sure that exactly the required signals are 
    // emitted.
    void actionSequenceAll() {
        short action = 0;
	QJsonObject result;
	QList<QVariant> sg;
	QAppImageUpdate updater;
	connect(&updater, &QAppImageUpdate::error, this, &QAppImageUpdateTests::defaultErrorHandler);
	QSignalSpy spyInfo(&updater, SIGNAL(finished(QJsonObject, short)));
	QEventLoop loop;	
	connect(&updater, &QAppImageUpdate::finished, &loop, &QEventLoop::quit, Qt::QueuedConnection);
	
	for(auto iter = m_Available.begin(),
		 end = m_Available.end();
		 iter != end;
		 ++iter) { 	
	updater.setAppImage(*iter);
	qInfo().noquote() << "Update(" << *iter << ")";

	updater.start(QAppImageUpdate::Action::CheckForUpdate);
	loop.exec();

        QCOMPARE(spyInfo.count(), 1);

	sg = spyInfo.takeFirst();
	result = sg.at(0).toJsonObject();
	action = sg.at(1).toInt();

	QVERIFY(action == QAppImageUpdate::Action::CheckForUpdate);
	QVERIFY(result.contains("UpdateAvailable"));

	auto remoteSha1 = result["RemoteSha1Hash"].toString();

	// Now Update	
	updater.start();
	loop.exec();

	QCOMPARE(spyInfo.count(), 1);

	sg = spyInfo.takeFirst();
	result = sg.at(0).toJsonObject();
	action = sg.at(1).toInt();

	QVERIFY(action == QAppImageUpdate::Action::Update);
	QCOMPARE(remoteSha1, result["NewVersionSha1Hash"].toString());
	}
    }

    /// I have no idea on how to test thread safety,
    //  so we are just gonna call setAppImage and
    //  start from multiple threads.
    //
    //  The expected result is,
    //
    //  The first thread to call setAppImage and start
    //  will succeed and all other threads calls will be 
    //  invalid and will not affect the object or segfaults.
    //
    //  So the QSignalSpy should have only one signal that
    //  is finished. and not multiple signals.
    void threadSafety() {
	    QAppImageUpdate updater;
	    QEventLoop loop;
	    connect(&updater, &QAppImageUpdate::error, this, &QAppImageUpdateTests::defaultErrorHandler);
	    QSignalSpy spyInfo(&updater, SIGNAL(finished(QJsonObject, short)));
	    connect(&updater, &QAppImageUpdate::finished, &loop, &QEventLoop::quit, Qt::QueuedConnection);

	    auto function = [&]() {
		    updater.setAppImage(m_Available.at(0));
		    updater.start();
	    };

	    auto future1 = new QFuture<void>;
	    auto future2 = new QFuture<void>;
	    auto future3 = new QFuture<void>;
	    auto future4 = new QFuture<void>;
	    *future1 = QtConcurrent::run(function);
	    *future2 = QtConcurrent::run(function);
	    *future3 = QtConcurrent::run(function);
	    *future4 = QtConcurrent::run(function);

	    /// Wait for all futures to end;
	    while(future1->isRunning() || future2->isRunning() ||
		  future3->isRunning() || future4->isRunning()) {
		    QCoreApplication::processEvents();
	    }

	    delete future1;
	    delete future2;
	    delete future3;
	    delete future4;

    	    if(spyInfo.count() < 1) {
		    if(!spyInfo.wait()){
		    	loop.exec();
		    }
	    }

	    QCOMPARE(spyInfo.count(), 1);

	    auto sg = spyInfo.takeFirst();
	    QJsonObject result = sg.at(0).toJsonObject();
	    short action = sg.at(1).toInt();

	    QVERIFY(action == QAppImageUpdate::Action::Update);
    }

    void cleanupTestCase(void) {
        m_TempDir->remove();
	emit finished();
        return;
    }
  protected slots:
    void defaultErrorHandler(short code, short action) {
	Q_UNUSED(action);
        auto scode = QAppImageUpdate::errorCodeToString(code);
        scode.prepend("error:: ");
        QFAIL(QTest::toString(scode));
        return;
    }
  Q_SIGNALS:
    void finished(void);
};
#endif
