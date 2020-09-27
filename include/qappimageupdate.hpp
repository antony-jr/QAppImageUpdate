#ifndef QAPPIMAGE_UPDATE_HPP_INCLUDED
#define QAPPIMAGE_UPDATE_HPP_INCLUDED
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QFile>
#include <QNetworkProxy>
#include <QByteArray>
#include <QJsonObject>
#include <QtPlugin>

/// Enums and Codes
#include "qappimageupdateenums.hpp"
#include "qappimageupdatecodes.hpp"
/// ----

/// Forward declare private class.
class QAppImageUpdatePrivate;

class QAppImageUpdate : public QObject {
    Q_OBJECT
  public:
    QAppImageUpdate(bool singleThreaded = true, QObject *parent = nullptr);
    QAppImageUpdate(const QString &AppImagePath, bool singleThreaded = true, QObject *parent = nullptr);
    QAppImageUpdate(QFile *AppImage, bool singleThreaded = true, QObject *parent = nullptr);
    ~QAppImageUpdate();

    struct Action : public QAppImageUpdateCodes::Action { };
    struct GuiFlag : public QAppImageUpdateCodes::GuiFlag { };
    struct Error : public QAppImageUpdateEnums::Error { };

    static QString errorCodeToString(short);
    static QString errorCodeToDescriptionString(short);
  public Q_SLOTS:
    void setIcon(QByteArray);
    void setGuiFlag(int);
    void setAppImage(const QString&);
    void setAppImage(QFile*);
    void setShowLog(bool);
    void setOutputDirectory(const QString&);
    void setProxy(const QNetworkProxy&);
    void start(short action = Action::Update,
               int flags = GuiFlag::Default,
               QByteArray icon = QByteArray());
    void cancel();
    void clear();

  Q_SIGNALS:
    void started(short);
    void canceled(short);
    void finished(QJsonObject info, short);
    void progress(int, qint64, qint64, double, QString, short);
    void logger(QString, QString);
    void error(short, short);
    void quit();

  private:
    QSharedPointer<QAppImageUpdatePrivate> m_Private;
};

#endif // QAPPIMAGE_UPDATE_HPP_INCLUDED
