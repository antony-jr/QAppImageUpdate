#ifndef ZSYNC_REMOTE_CONTROL_FILE_PARSER_TESTS_HPP_INCLUDED
#define ZSYNC_REMOTE_CONTROL_FILE_PARSER_TESTS_HPP_INCLUDED
#include <QTest>
#include <QSignalSpy>
#include <QNetworkAccessManager>
#include "../include/zsyncremotecontrolfileparser_p.hpp"
/*
 * Get the official appimage tool to test it with
 * our library.
*/
#define APPIMAGE_TOOL_CONTROL_FILE_URL QUrl("https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage.zsync")

class ZsyncRemoteControlFileParser : public QObject
{
    Q_OBJECT
private slots:

    void basicZsyncControlFileParsing(void)
    {
        using AppImageUpdaterBridge::ZsyncRemoteControlFileParserPrivate;
        ZsyncRemoteControlFileParserPrivate CFParser(&_pManager);
        CFParser.setControlFileUrl(APPIMAGE_TOOL_CONTROL_FILE_URL);
        QSignalSpy spyReceiveControlFile(&CFParser, SIGNAL(receiveControlFile(void)));
        CFParser.getControlFile();

        /*
         * Since this is done over the network , lets wait
         * atmost 10 seconds.
         * If we don't get any signals inside that time frame then
         * ZsyncRemoteControlFileParserPrivate did not work properly.
         */

        QVERIFY(spyReceiveControlFile.wait(10000) == true);
        return;
    }

    void verifyZsyncControlFileParserData(void)
    {
        using AppImageUpdaterBridge::ZsyncRemoteControlFileParserPrivate;
        ZsyncRemoteControlFileParserPrivate CFParser(&_pManager);
        CFParser.setControlFileUrl(APPIMAGE_TOOL_CONTROL_FILE_URL);
        QSignalSpy spyReceiveControlFile(&CFParser, SIGNAL(receiveControlFile(void)));
	QSignalSpy spyInfo(&CFParser , SIGNAL(zsyncInformation(qint32,qint32,qint32,
                          		      qint32,qint32,qint32,
                          		      QString,QString,QString,
                          		      QUrl,QBuffer*,bool)));
	CFParser.getControlFile();

        /* Check if we received the control file */
        QVERIFY(spyReceiveControlFile.wait(10000) == true);
        return;
    }

    void checkErrorSignal(void)
    {
        using AppImageUpdaterBridge::ZsyncRemoteControlFileParserPrivate;
        ZsyncRemoteControlFileParserPrivate CFParser(&_pManager);
        CFParser.setControlFileUrl(QUrl("https://example.com/somecontrolfile.zsync"));
        QSignalSpy spyError(&CFParser, SIGNAL(error(short)));
        CFParser.getControlFile();

        /* Check if we received a error*/
        QVERIFY(spyError.wait() == true);
        return;
    }

    void cleanupTestCase(void)
    {
        emit finished();
        return;
    }
Q_SIGNALS:
    void finished(void);
private:
    QNetworkAccessManager _pManager;
};
#endif
