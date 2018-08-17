#ifndef APPIMAGE_UPDATE_INFORMATION_TESTS_HPP_INCLUDED
#define APPIMAGE_UPDATE_INFORMATION_TESTS_HPP_INCLUDED
#include <QTest>
#include <QSignalSpy>
#include <AppImageUpdateInformation_p.hpp>

/*
 * Get the official appimage tool to test it with 
 * our library.
*/
#define APPIMAGE_TOOL_RELATIVE_PATH QString("test_cases/appimagetool-x86_64.AppImage")

class AppImageUpdateInformation : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase(void)
    {
	QFileInfo file(APPIMAGE_TOOL_RELATIVE_PATH);
	if(!file.exists()){
		QFAIL("required test cases does not exist!");
		emit finished();
	}
	return;
    }
    
    void benchmark(void)
    {
	using AppImageUpdaterBridge::AppImageUpdateInformationPrivate;
	QBENCHMARK {
	AppImageUpdateInformationPrivate AIUpdateInformation;
	AIUpdateInformation.setAppImage(APPIMAGE_TOOL_RELATIVE_PATH);

	QSignalSpy spyInfo(&AIUpdateInformation , SIGNAL(info(QJsonObject)));
	AIUpdateInformation.getInfo();
	
	/*  Must emit exactly one signal. */
	QCOMPARE(spyInfo.count() , 1);
	
	}
	return;		
    }

    void resultingJsonObjectisUniformAndValid(void)
    {
	using AppImageUpdaterBridge::AppImageUpdateInformationPrivate;
	AppImageUpdateInformationPrivate AIUpdateInformation;
	AIUpdateInformation.setAppImage(APPIMAGE_TOOL_RELATIVE_PATH);

	QSignalSpy spyInfo(&AIUpdateInformation , SIGNAL(info(QJsonObject)));
	AIUpdateInformation.getInfo();
	
	QCOMPARE(spyInfo.count() , 1);

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

    void checkErrorSignal(void)
    {
	using AppImageUpdaterBridge::AppImageUpdateInformationPrivate;
	AppImageUpdateInformationPrivate AIUpdateInformation;
	QSignalSpy spyInfo(&AIUpdateInformation , SIGNAL(error(short)));
	AIUpdateInformation.getInfo();
	QCOMPARE(spyInfo.count() , 1);
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
