#ifndef QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
#define QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
#include <QObject>
#include <QThread>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QNetworkProxy>
#include <QJsonObject>

#include "qappimageupdatecodes.hpp"
#include "appimageupdateinformation_p.hpp"
#include "zsyncremotecontrolfileparser_p.hpp"
#include "zsyncwriter_p.hpp"

#ifndef NO_GUI
#include <QDialog>
// forward declare
class SoftwareUpdateDialog;
namespace Ui {
class AppImageUpdaterDialog;
}
#endif // NO_GUI

class QAppImageUpdatePrivate : public QObject {
    Q_OBJECT
  public:
    QAppImageUpdatePrivate(bool singleThreaded = true, QObject *parent = nullptr);
    QAppImageUpdatePrivate(const QString &AppImagePath, bool singleThreaded = true, QObject *parent = nullptr);
    QAppImageUpdatePrivate(QFile *AppImage, bool singleThreaded = true, QObject *parent = nullptr);
    ~QAppImageUpdatePrivate();

    struct Action : public QAppImageUpdateCodes::Action { };
    struct GuiFlag : public QAppImageUpdateCodes::GuiFlag { };

    static QString errorCodeToString(short);
    static QString errorCodeToDescriptionString(short);
  public Q_SLOTS:
    void setApplicationName(const QString&);
    void setIcon(QByteArray);
    void setGuiFlag(int);
    void setAppImage(const QString&);
    void setAppImage(QFile*);
    void setShowLog(bool);
    void setOutputDirectory(const QString&);
    void setProxy(const QNetworkProxy&);
    void start(short action = Action::Update,
               int flags = GuiFlag::None,
               QByteArray icon = QByteArray());
    void cancel();
    void clear();

  private Q_SLOTS:
    void setCurrentAppImagePath(QString);
    void handleGetEmbeddedInfoError(short);
    void redirectEmbeddedInformation(QJsonObject);
    void handleCheckForUpdateError(short);
    void redirectUpdateCheck(QJsonObject);
    void handleCheckForUpdateProgress(int);
    void handleGetEmbeddedInfoProgress(int);
    void handleUpdateProgress(int, qint64, qint64, double, QString);
    void handleUpdateStart();
    void handleUpdateCancel();
    void handleUpdateFinished(QJsonObject, QString);
    void handleUpdateError(short);
#ifndef NO_GUI
    void showWidget();
    void handleGUIConfirmationRejected();
    void handleGUIConfirmationAccepted();

    void doGUIUpdate();
    void handleGUIUpdateCheckProgress(int, qint64, qint64, double, QString);
    void handleGUIUpdateCheck(QJsonObject);
    void handleGUIUpdateCheckError(short);
    void handleGUIUpdateProgress(int, qint64, qint64, double, QString);
    void handleGUIUpdateStart();
    void handleGUIUpdateCancel();
    void handleGUIUpdateFinished(QJsonObject, QString);
    void handleGUIUpdateError(short);
#endif // NOT NO_GUI

  Q_SIGNALS:
    void started(short);
    void canceled(short);
    void finished(QJsonObject info, short);
    void progress(int, qint64, qint64, double, QString, short);
    void logger(QString, QString);
    void error(short, short);
    void quit();
  private:
    QByteArray m_Icon;
    int n_GuiFlag = GuiFlag::None;
    short n_CurrentAction = Action::None;
    bool b_Started = false,
         b_Finished = false,
         b_Canceled = false,
         b_Running = false,
         b_CancelRequested = false,
         b_GuiClassesConstructed = false;
    QString m_CurrentAppImagePath;
    QString m_ApplicationName;
    QScopedPointer<AppImageUpdateInformationPrivate> m_UpdateInformation;
    QScopedPointer<ZsyncRemoteControlFileParserPrivate> m_ControlFileParser;
    QScopedPointer<ZsyncWriterPrivate> m_DeltaWriter;
    QScopedPointer<QThread> m_SharedThread;
    QScopedPointer<QNetworkAccessManager> m_SharedNetworkAccessManager;
#ifndef NO_GUI
    QScopedPointer<QDialog> m_UpdaterDialog;
    QSharedPointer<SoftwareUpdateDialog> m_ConfirmationDialog;
    QSharedPointer<Ui::AppImageUpdaterDialog> m_Ui;
#endif // NOT NO_GUI
};

#endif // QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
