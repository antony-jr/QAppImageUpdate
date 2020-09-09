#ifndef QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
#define QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
#include <QObject>
#include <QThread>
#include <QScopedPointer>
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QNetworkProxy>
#include <QJsonObject>

#include "qappimageupdatecodes.hpp"
#include "appimageupdateinformation_p.hpp"
#include "zsyncremotecontrolfileparser_p.hpp"
#include "zsyncwriter_p.hpp"

class QAppImageUpdatePrivate : public QObject {
	Q_OBJECT
public:
    QAppImageUpdatePrivate(bool singleThreaded = true, QObject *parent = nullptr);
    QAppImageUpdatePrivate(const QString &AppImagePath, bool singleThreaded = true, QObject *parent = nullptr);
    QAppImageUpdatePrivate(QFile *AppImage, bool singleThreaded = true, QObject *parent = nullptr);
    ~QAppImageUpdatePrivate();

    struct Action : public QAppImageUpdateCodes::Action { };
    struct GuiFlag : public QAppImageUpdateCodes::GuiFlag { };


public Q_SLOTS:
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

private Q_SLOTS:
    void handleGetEmbeddedInfoError(short); 
    void redirectEmbeddedInformation(QJsonObject);
    void handleCheckForUpdateError(short);
    void redirectUpdateCheck(QJsonObject);

Q_SIGNALS:
    void started(short);
    void canceled(short);
    void finished(QJsonObject info, short);
    void progress(int, qint64, qint64, int, QString, short);
    void logger(QString, QString); 
    void error(short, short);
private:
    int n_CurrentAction = Action::None;
    bool b_Started = false,
	 b_Finished = false,
	 b_Canceled = false,
	 b_Running = false,
	 b_CancelRequested = false;
    QScopedPointer<AppImageUpdateInformationPrivate> m_UpdateInformation;
    QScopedPointer<ZsyncRemoteControlFileParserPrivate> m_ControlFileParser;
    QScopedPointer<ZsyncWriterPrivate> m_DeltaWriter;
    QScopedPointer<QThread> m_SharedThread;
    QScopedPointer<QNetworkAccessManager> m_SharedNetworkAccessManager;
};

#endif // QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
