#include <QCoreApplication>
#include <AppImageUpdateInformation.hpp>

int main(int ac, char **av)
{
    QCoreApplication app(ac, av);
    AppImageUpdateInformation AIUITest;
    
    auto startTests = [&](){
	/* Test AppImage Update Information. */
	QTest::qExec(&AIUITest);
	return;
    };

    QObject::connect(&AIUITest , &AppImageUpdateInformation::finished , &app , &QCoreApplication::quit,
		    Qt::QueuedConnection);
    QTimer::singleShot(256 , startTests);
    return app.exec();
}
