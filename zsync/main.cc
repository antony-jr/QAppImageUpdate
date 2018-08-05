#include <ZsyncWriter_p.hpp>

using namespace AppImageUpdaterBridge;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ZsyncWriterPrivate Writer;
    return app.exec();
}
