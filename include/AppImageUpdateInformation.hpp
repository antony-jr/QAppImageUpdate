#ifndef APPIMAGE_UPDATE_INFORMATION_HPP_INCLUDED
#define APPIMAGE_UPDATE_INFORMATION_HPP_INCLUDED
#include <AppImageUpdateInformation_p.hpp>
#include <QNetworkAccessManager>

namespace AppImageUpdaterBridge {
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

    explicit AppImageUpdateInformation(QNetworkAccessManager *networkManager = nullptr);
    explicit AppImageUpdateInformation(const QString&, QNetworkAccessManager *networkManager = nullptr);
    explicit AppImageUpdateInformation(QFile *, QNetworkAccessManager *networkManager = nullptr);
    ~AppImageUpdateInformation();

    /* Public static methods. */
    static QString errorCodeToString(short);

public Q_SLOTS:
    void shareThreadWith(QObject*);
    QNetworkAccessManager *getSharedNetworkManager(void);
    bool isEmpty(void);
    AppImageUpdateInformation &setAppImage(const QString&);
    AppImageUpdateInformation &setAppImage(QFile *);
    AppImageUpdateInformation &setShowLog(bool);
    AppImageUpdateInformation &getInfo(void);
    QString getAppImageSHA1(void);
    QString getAppImageName(void);
    QString getAppImagePath(void);
    AppImageUpdateInformation &clear(void);

Q_SIGNALS:
    void info(QJsonObject);
    void progress(int);
    void error(short);
    void logger(QString , QString); /* log msg , appiamge path */

private:
    QSharedPointer<Private::AppImageUpdateInformationPrivate> _pUpdateInformationParser = nullptr;
    QSharedPointer<QThread> _pSharedThread = nullptr;
    QSharedPointer<QNetworkAccessManager> _pSharedNetworkManager = nullptr;
};
}
#endif // APPIMAGE_UPDATE_INFORMATION_HPP_INCLUDED
