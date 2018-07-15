#include <ZsyncCore_p.hpp>

using namespace AppImageUpdaterBridgePrivate;

int main(int ac, char **av)
{
    if(ac < 2) {
        qInfo().noquote() << "Usage: " << av[0] << " [zsync meta file]";
        return 0;
    }
    QCoreApplication app(ac , av);
    QUrl controlFile(av[1]);
    QString seedFile(av[2]);
    ZsyncCorePrivate z;
#ifdef LOGGING_ENABLED
    z.setShowLog(true);
#endif
    z.setControlFileUrl(controlFile);
    
    z.addSourceFile(seedFile);
    return app.exec();
}
