#ifndef APPIMAGE_DELTA_REVISIONER_TESTS_HPP_INCLUDED
#define APPIMAGE_DELTA_REVISIONER_TESTS_HPP_INCLUDED
#include <QTest>
#include <QSignalSpy>
#include "../include/appimagedeltarevisioner.hpp"

/*
 * Get the official appimage tool to test it with
 * our library.
*/
#define APPIMAGE_TOOL_RELATIVE_PATH QString("test_cases/appimagetool-x86_64.AppImage")

class AppImageDeltaRevisioner : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase(void)
    {
        QFileInfo file(APPIMAGE_TOOL_RELATIVE_PATH);
        if(!file.exists()) {
            QFAIL("required test cases does not exist!");
            emit finished();
        }
        return;
    }

    void getAppImageEmbededInformation(void)
    {
        using AppImageUpdaterBridge::AppImageDeltaRevisioner;
        AppImageDeltaRevisioner AIDeltaRev;
        AIDeltaRev.setAppImage(APPIMAGE_TOOL_RELATIVE_PATH);

        QSignalSpy spyInfo(&AIDeltaRev, SIGNAL(embededInformation(QJsonObject)));
        AIDeltaRev.getAppImageEmbededInformation();

	QVERIFY(spyInfo.count() || spyInfo.wait());

        /* Get resultant QJsonObject and Compare. */
        auto result = spyInfo.takeFirst().at(0).toJsonObject();

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
  
    void checkForUpdates(void){
        using AppImageUpdaterBridge::AppImageDeltaRevisioner;
        AppImageDeltaRevisioner AIDeltaRev;
        AIDeltaRev.setAppImage(APPIMAGE_TOOL_RELATIVE_PATH);

        QSignalSpy spyInfo(&AIDeltaRev, SIGNAL(updateAvailable(bool,QJsonObject)));
        AIDeltaRev.checkForUpdate();

	QVERIFY(spyInfo.count() || spyInfo.wait(30 * 1000));

    }

    void checkForUpdateAndStartDoesNotCollide(void){
        using AppImageUpdaterBridge::AppImageDeltaRevisioner;
        AppImageDeltaRevisioner AIDeltaRev;
        AIDeltaRev.setAppImage(APPIMAGE_TOOL_RELATIVE_PATH);

        QSignalSpy spyInfo(&AIDeltaRev, SIGNAL(updateAvailable(bool,QJsonObject)));
        AIDeltaRev.checkForUpdate();
	AIDeltaRev.start(); // should not collide, this call should be ignored.

	QVERIFY(spyInfo.count() || spyInfo.wait(30 * 1000));
    }

    void noCallsCollide(void){
        using AppImageUpdaterBridge::AppImageDeltaRevisioner;
        AppImageDeltaRevisioner AIDeltaRev;
        AIDeltaRev.setAppImage(APPIMAGE_TOOL_RELATIVE_PATH);

        QSignalSpy spyInfo(&AIDeltaRev, SIGNAL(updateAvailable(bool,QJsonObject)));
        AIDeltaRev.checkForUpdate();
	AIDeltaRev.getAppImageEmbededInformation();
	AIDeltaRev.start(); // should not collide, this call should be ignored.
	AIDeltaRev.getAppImageEmbededInformation();

	QVERIFY(spyInfo.count() || spyInfo.wait(30 * 1000));
    }

    void updateShouldSucceed(void){
        using AppImageUpdaterBridge::AppImageDeltaRevisioner;
        AppImageDeltaRevisioner AIDeltaRev;
        AIDeltaRev.setAppImage(APPIMAGE_TOOL_RELATIVE_PATH);

        QSignalSpy spyInfo(&AIDeltaRev, SIGNAL(finished(QJsonObject , QString)));
        AIDeltaRev.start();
	
	QVERIFY(spyInfo.count() || spyInfo.wait(50 * 1000));
    }

    void checkErrorSignal(void)
    {
        using AppImageUpdaterBridge::AppImageDeltaRevisioner;
        AppImageDeltaRevisioner AIDeltaRev;
        QSignalSpy spyInfo(&AIDeltaRev, SIGNAL(error(short)));
        AIDeltaRev.start();
        
	QVERIFY(spyInfo.count() || spyInfo.wait());
        return;
    }

    void cleanupTestCase(void)
    {
        emit finished();
        return;
    }
Q_SIGNALS:
    void finished(void);
};
#endif
