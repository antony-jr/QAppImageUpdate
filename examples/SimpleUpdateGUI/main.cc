#include <QApplication>
#include <AppImageUpdaterBridge>

using namespace AppImageUpdaterBridge;

int main(int ac, char **av)
{
    qInfo().noquote() << "SimpleUpdate , A Simple Updater using AppImageUpdaterBridge.";
    qInfo().noquote() << "Copyright (C) 2018 , Antony Jr.";

    if(ac == 1) {
        qInfo().noquote() << "\nUsage: " << av[0] << " [APPIMAGE PATH].";
        return -1;
    }
    int it = 1;
    QApplication app(ac, av);
    AppImageUpdaterDialog UWidget;
    
    QObject::connect(&UWidget, &AppImageUpdaterDialog::error , [&](QString eStr, short errorCode){
        qInfo() << "ERROR CODE:: " << errorCode;
        return;
    });

    QObject::connect(&UWidget , &AppImageUpdaterDialog::quit , &app , &QApplication::quit , Qt::QueuedConnection);
    QObject::connect(&UWidget , &AppImageUpdaterDialog::canceled , &app , &QApplication::quit , Qt::QueuedConnection);

    QObject::connect(&UWidget, &AppImageUpdaterDialog::finished, [&](QJsonObject newVersion) {
        (void)newVersion;
        ++it;
        if(it >= ac) {
            app.quit();
        } else {
            ++av;
            QString path(*av);
            UWidget.setAppImage(path);
            UWidget.init();
        }
        return;
    });

    ++av;
    QString path(*av);
    UWidget.setAppImage(path);
    UWidget.setShowErrorDialog(true);
    UWidget.setShowUpdateConfirmationDialog(true);
    UWidget.setShowFinishDialog(true);
    UWidget.init();
    return app.exec();
}
