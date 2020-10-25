#ifndef QAPPIMAGE_UPDATE_INTERFACE_PRIVATE_HPP_INCLUDED
#define QAPPIMAGE_UPDATE_INTERFACE_PRIVATE_HPP_INCLUDED
#ifdef BUILD_AS_PLUGIN
#include <QtPlugin>
#include <QString>
#include <QFile>
#include <QNetworkProxy>
#include <QJsonObject>

class QAppImageUpdateInterface {
  public Q_SLOTS:
    virtual void setGuiFlag(int) = 0;
    virtual void setIcon(QByteArray) = 0;
    virtual void setAppImagePath(const QString&) = 0;
    virtual void setAppImageFile(QFile*) = 0;
    virtual void setShowLog(bool) = 0;
    virtual void setOutputDirectory(const QString&) = 0;
    virtual void setProxy(const QNetworkProxy&) = 0;
    virtual void start(short) = 0;
    virtual void cancel() = 0;
    virtual void clear() = 0;

    virtual int getConstant(const QString&) = 0;
    virtual QObject *getObject() = 0;

    virtual QString errorCodeToString(short) = 0;
    virtual QString errorCodeToDescriptionString(short) = 0;
  Q_SIGNALS:
    virtual void torrentClientStarted() = 0;
    virtual void torrentStatus(int,int) = 0;
    virtual void started(short) = 0;
    virtual void canceled(short) = 0;
    virtual void finished(QJsonObject info, short) = 0;
    virtual void progress(int, qint64, qint64, double, QString, short) = 0;
    virtual void logger(QString, QString) = 0;
    virtual void error(short, short) = 0;
};

#ifndef QAppImageUpdateInterface_iid
#define QAppImageUpdateInterface_iid "com.antony-jr.QAppImageUpdate"
#endif

Q_DECLARE_INTERFACE(QAppImageUpdateInterface, QAppImageUpdateInterface_iid);
#endif // BUILD_AS_PLUGIN
#endif // QAPPIMAGEUPDATE_INTERFACE_PRIVATE_HPP_INCLUDED
