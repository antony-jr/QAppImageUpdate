#ifndef APPIMAGE_UPDATE_RESOURCE_HPP_INCLUDED
#define APPIMAGE_UPDATE_RESOURCE_HPP_INCLUDED
#include <QtCore>
#include <QNetworkAccessManager>

namespace AppImageUpdaterBridge {

class AppImageUpdateResourcePrivate;

class AppImageUpdateResource : public QObject
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

    explicit AppImageUpdateResource(QObject *parent = nullptr);
    explicit AppImageUpdateResource(const QString& , QObject *parent = nullptr);
    explicit AppImageUpdateResource(QFile * , QObject *parent = nullptr);
    ~AppImageUpdateResource();

    /* Public static methods. */
    static QString errorCodeToString(short);

public Q_SLOTS:
    AppImageUpdateResource &setAppImage(const QString&);
    AppImageUpdateResource &setAppImage(QFile *);
    AppImageUpdateResource &setShowLog(bool);
    AppImageUpdateResource &getInfo(void);
    AppImageUpdateResource &clear(void);
    QThread *sharedQThread(void) const;
    QNetworkAccessManager *sharedQNetworkAccessManager(void) const;

Q_SIGNALS:
    void info(QJsonObject);
    void progress(int);
    void error(short);
    void logger(QString , QString); /* log msg , appimage path */

private:
    AppImageUpdateResourcePrivate *_pUpdateInformation = nullptr;
    QThread *_pSharedThread = nullptr;
    QNetworkAccessManager *_pSharedNetworkAccessManager = nullptr;
};
}
#endif // APPIMAGE_UPDATE_RESOURCE_HPP_INCLUDED
