#include <QCoreApplication>
#include <AppImageUpdateInformation.hpp>
#include <ZsyncRemoteControlFileParser.hpp>

int main(int ac, char **av)
{
    QCoreApplication app(ac, av);
    AppImageUpdateInformation AIUITest;
    ZsyncRemoteControlFileParser ZRCFParserTest;

    auto startTests = [&](){
	/* Test AppImage Update Information. */
	QTest::qExec(&AIUITest);
	QTest::qExec(&ZRCFParserTest);
	return;
    };

    QObject::connect(&ZRCFParserTest , &ZsyncRemoteControlFileParser::finished , &app , &QCoreApplication::quit,
		    Qt::QueuedConnection);
    QTimer::singleShot(256 , startTests);
    return app.exec();
}
