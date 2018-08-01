#ifndef APPIMAGE_UPDATE_INFORMATION_HPP_INCLUDED
#define APPIMAGE_UPDATE_INFORMATION_HPP_INCLUDED
#include <QtCore>

namespace AppImageUpdaterBridge {

class AppImageUpdateInformationPrivate;

class AppImageUpdateInformation : public QObject
{
    Q_OBJECT
public:
    enum : short {
        APPIMAGE_NOT_READABLE,
        NO_READ_PERMISSION,
        APPIMAGE_NOT_FOUND,
        CANNOT_OPEN_APPIMAGE,
        EMPTY_UPDATE_INFORMATION,
        INVALID_APPIMAGE_TYPE,
        INVALID_MAGIC_BYTES,
        INVALID_UPDATE_INFORMATION,
        NOT_ENOUGH_MEMORY,
        SECTION_HEADER_NOT_FOUND,
        UNSUPPORTED_ELF_FORMAT,
        UNSUPPORTED_TRANSPORT
    } error_code;

    explicit AppImageUpdateInformation(QObject *parent = nullptr);
    explicit AppImageUpdateInformation(const QString&, QObject *parent = nullptr);
    explicit AppImageUpdateInformation(QFile *, QObject *parent = nullptr);
    ~AppImageUpdateInformation();

    /* Public static methods. */
    static QString errorCodeToString(short);

public Q_SLOTS:
    AppImageUpdateInformation &setAppImage(const QString&);
    AppImageUpdateInformation &setAppImage(QFile *);
    AppImageUpdateInformation &setShowLog(bool);
    AppImageUpdateInformation &getInfo(void);
    AppImageUpdateInformation &clear(void);

Q_SIGNALS:
    void info(QJsonObject);
    void progress(int);
    void error(short);
    void logger(QString , QString); /* log msg , appimage path */

private:
    AppImageUpdateInformationPrivate *_pUpdateInformation = nullptr;
};
}
#endif // APPIMAGE_UPDATE_INFORMATION_HPP_INCLUDED
