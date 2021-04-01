#ifdef BUILD_AS_PLUGIN
#include "qappimageupdateinterface.hpp"
#endif
#include "qappimageupdate.hpp"
#include "qappimageupdate_p.hpp"
#include "helpers_p.hpp"

QAppImageUpdate::QAppImageUpdate(bool singleThreaded, QObject *parent)
    : QObject(parent) {
    m_Private = QSharedPointer<QAppImageUpdatePrivate>(
                    new QAppImageUpdatePrivate(singleThreaded = singleThreaded));
    auto s = m_Private.data();

    connect(s, &QAppImageUpdatePrivate::torrentClientStarted,
	    this, &QAppImageUpdate::torrentClientStarted, Qt::DirectConnection);
    connect(s, &QAppImageUpdatePrivate::torrentStatus,
	    this, &QAppImageUpdate::torrentStatus, Qt::DirectConnection);
    connect(s, &QAppImageUpdatePrivate::started,
            this, &QAppImageUpdate::started, Qt::DirectConnection);
    connect(s, &QAppImageUpdatePrivate::canceled,
            this, &QAppImageUpdate::canceled, Qt::DirectConnection);
    connect(s, &QAppImageUpdatePrivate::finished,
            this, &QAppImageUpdate::finished, Qt::DirectConnection);
    connect(s, &QAppImageUpdatePrivate::progress,
            this, &QAppImageUpdate::progress, Qt::DirectConnection);
    connect(s, &QAppImageUpdatePrivate::logger,
            this, &QAppImageUpdate::logger, Qt::DirectConnection);
    connect(s, &QAppImageUpdatePrivate::error,
            this, &QAppImageUpdate::error, Qt::DirectConnection);
    connect(s, &QAppImageUpdatePrivate::quit,
            this, &QAppImageUpdate::quit, Qt::DirectConnection);
}

QAppImageUpdate::QAppImageUpdate(const QString &AppImagePath, bool singleThreaded, QObject *parent)
    : QAppImageUpdate(singleThreaded, parent) {
    setAppImage(AppImagePath);
}

QAppImageUpdate::QAppImageUpdate(QFile *AppImage, bool singleThreaded, QObject *parent)
    : QAppImageUpdate(singleThreaded, parent) {
    setAppImage(AppImage);
}

QAppImageUpdate::~QAppImageUpdate() { }

void QAppImageUpdate::setApplicationName(const QString &applicationName) {
    getMethod(m_Private.data(), "setApplicationName(const QString&)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(QString, applicationName));
}

void QAppImageUpdate::setIcon(QByteArray icon) {
    getMethod(m_Private.data(), "setIcon(QByteArray)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(QByteArray,icon));
}


void QAppImageUpdate::setGuiFlag(int flags) {
    getMethod(m_Private.data(), "setGuiFlag(int)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(int,flags));
}


void QAppImageUpdate::setAppImage(const QString &AppImagePath) {
    getMethod(m_Private.data(), "setAppImage(const QString&)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(QString,AppImagePath));
}
void QAppImageUpdate::setAppImage(QFile *AppImage) {
    getMethod(m_Private.data(), "setAppImage(QFile*)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(QFile*,AppImage));
}
void QAppImageUpdate::setShowLog(bool boolean) {
    getMethod(m_Private.data(), "setShowLog(bool)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(bool,boolean));
}

void QAppImageUpdate::setOutputDirectory(const QString &OutputDirectory) {
    getMethod(m_Private.data(), "setOutputDirectory(const QString&)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(QString,OutputDirectory));
}

void QAppImageUpdate::setProxy(const QNetworkProxy &Proxy) {
    getMethod(m_Private.data(), "setProxy(const QNetworkProxy&)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(QNetworkProxy, Proxy));
}

void QAppImageUpdate::start(short action, int flags, QByteArray icon) {
    getMethod(m_Private.data(), "start(short, int, QByteArray)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(short, action),
            Q_ARG(int, flags),
            Q_ARG(QByteArray, icon));

}

void QAppImageUpdate::cancel() {
    getMethod(m_Private.data(), "cancel()")
    .invoke(m_Private.data(),
            Qt::QueuedConnection);
}

void QAppImageUpdate::clear() {
    getMethod(m_Private.data(), "clear()")
    .invoke(m_Private.data(),
            Qt::QueuedConnection);
}


//// Static Methods
QString QAppImageUpdate::errorCodeToString(short errorCode) {
    return QAppImageUpdatePrivate::errorCodeToString(errorCode);
}

QString QAppImageUpdate::errorCodeToDescriptionString(short errorCode) {
    return QAppImageUpdatePrivate::errorCodeToDescriptionString(errorCode);
}

QString QAppImageUpdate::versionString() {
    return QAppImageUpdatePrivate::versionString();
}
