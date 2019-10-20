#include <QCoreApplication>
#include <QDebug>
#include <AppImageUpdaterBridge>

int main(int ac, char **av) {
    if(ac == 1) {
        qInfo() << "Usage: " << av[0] << "[APPIMAGE PATH]";
        return 0;
    }
    using AppImageUpdaterBridge::AppImageDeltaRevisioner;
    using AppImageUpdaterBridge::errorCodeToDescriptionString;
    QCoreApplication app(ac, av);
    QString AppImagePath(av[1]);

    /* set proxy settings */
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::Socks5Proxy);
    proxy.setHostName("127.0.0.1");
    proxy.setPort(9050);

    AppImageDeltaRevisioner Revisioner(AppImagePath);

    QObject::connect(&Revisioner, &AppImageDeltaRevisioner::error,
    [&](short code) {
        qCritical() << "Could not continue the update because , "
                    <<  errorCodeToDescriptionString(code);
        app.quit();
        return;
    });

    QObject::connect(&Revisioner, &AppImageDeltaRevisioner::finished,
    [&](QJsonObject newVersion, QString oldVersionPath) {
        (void)newVersion;
        (void)oldVersionPath;
        qInfo() << "Update Completed!";
        app.quit();
        return;
    });
    Revisioner.setShowLog(true);
    Revisioner.setProxy(proxy);
    Revisioner.start();
    return app.exec();
}

