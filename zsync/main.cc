#include <AppImageDeltaWriter.hpp>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if(argc == 1){
	    return -2;
    }
    QString AppImagePath(argv[1]);
    AppImageDeltaWriter writer(AppImagePath);
   
    QObject::connect(&writer , &AppImageDeltaWriter::blockDownloaderInformation ,
    [&](QHash<qint32 , QByteArray> *hashTable , QVector<QPair<qint32 , qint32>> *ranges , QTemporaryFile *f)
    {
    f->setAutoRemove(false);
    qDebug() << "Required Ranges:: " << *ranges;
    app.quit();
    return;
    });
    writer.setShowLog(true).start();
    return app.exec();
}
