#ifndef QAPPIMAGE_UPDATE_INTERFACE_IMPL_HPP_INCLUDED
#define QAPPIMAGE_UPDATE_INTERFACE_IMPL_HPP_INCLUDED
#ifdef BUILD_AS_PLUGIN
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QFile>
#include <QNetworkProxy>
#include <QByteArray>
#include <QJsonObject>
#include <QtPlugin>

#include "qappimageupdate.hpp"
#include "qappimageupdateinterface.hpp"

class QAppImageUpdateInterfaceImpl : public QObject, QAppImageUpdateInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QAppImageUpdateInterface_iid FILE "QAppImageUpdate.json")
    Q_INTERFACES(QAppImageUpdateInterface)
  public:
    QAppImageUpdateInterfaceImpl(QObject *parent = nullptr);
    ~QAppImageUpdateInterfaceImpl();
  public Q_SLOTS:
    void setIcon(QByteArray);
    void setGuiFlag(int);
    void setAppImagePath(const QString&);
    void setAppImageFile(QFile*);
    void setShowLog(bool);
    void setOutputDirectory(const QString&);
    void setProxy(const QNetworkProxy&);
    void start(short action);
    void cancel();
    void clear();

    int getConstant(const QString&);
    QObject *getObject();

    QString errorCodeToString(short);
    QString errorCodeToDescriptionString(short);
  Q_SIGNALS:
    void started(short);
    void canceled(short);
    void finished(QJsonObject info, short);
    void progress(int, qint64, qint64, double, QString, short);
    void logger(QString, QString);
    void error(short, short);
  private:
    QScopedPointer<QAppImageUpdate> m_Private;
};
#endif // BUILD_AS_PLUGIN
#endif // QAPPIMAGE_UPDATE_INTERFACE_IMPL_HPP_INCLUDED
