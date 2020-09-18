#include <QCoreApplication>
#include <QAppImageUpdateTests.hpp>

int main(int ac, char **av) {
    QCoreApplication app(ac, av);
    QAppImageUpdateTests tests;
    QObject::connect(&tests, &QAppImageUpdateTests::finished, &app, &QCoreApplication::quit,
                     Qt::QueuedConnection);
    QTest::qExec(&tests);
    return app.exec();
}
