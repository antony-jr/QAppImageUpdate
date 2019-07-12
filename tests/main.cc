#include <QCoreApplication>
#include <AppImageUpdateInformation.hpp>
#include <ZsyncRemoteControlFileParser.hpp>
#include <AppImageDeltaRevisioner.hpp>

int main(int ac, char **av)
{
    QCoreApplication app(ac, av);
    AppImageUpdateInformation AIUITest;
    ZsyncRemoteControlFileParser ZRCFParserTest;
    AppImageDeltaRevisioner AIDRTest;

    auto startTests = [&]() {
        /* Test AppImage Update Information. */
        QTest::qExec(&AIUITest);
        QTest::qExec(&ZRCFParserTest);
	QTest::qExec(&AIDRTest);
        return;
    };

    QObject::connect(&AIDRTest, &AppImageDeltaRevisioner::finished, &app, &QCoreApplication::quit,
                     Qt::QueuedConnection);
    startTests();
    return app.exec();
}
