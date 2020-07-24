#ifndef QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
#define QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QNetworkProxy>

#include "appimageupdaterbridge_enums.hpp"
#include "appimageupdateinformation_p.hpp"
#include "zsyncremotecontrolfileparser_p.hpp"
#include "zsyncwriter_p.hpp"
#include "zsyncblockrangedownloader_p.hpp"

class QAppImageUpdatePrivate : public QObject {
	Q_OBJECT
public:
    struct Action {
	enum : short {
	       GetEmbeddedInfo,
	       CheckForUpdate,
	       Update,
	       UpdateWithGUI       
    	};
    };

    struct GuiFlags { 
	enum {
        ShowProgressDialog = 0x1,
        ShowBeforeProgress = 0x2,
        ShowUpdateConfirmationDialog = 0x4,
        ShowFinishedDialog = 0x8,
        ShowErrorDialog = 0x10,
        AlertWhenAuthorizationIsRequired = 0x20,
        NotifyWhenNoUpdateIsAvailable = 0x40,
        NoRemindMeLaterButton = 0x80,
        NoSkipThisVersionButton = 0x100,
        Default = ShowBeforeProgress |
                  ShowProgressDialog |
                  ShowUpdateConfirmationDialog |
                  ShowFinishedDialog   |
                  ShowErrorDialog |
                  NotifyWhenNoUpdateIsAvailable |
                  NoRemindMeLaterButton |
                  NoSkipThisVersionButton
	};
    };

    QAppImageUpdatePrivate();
    ~QAppImageUpdatePrivate();

public Q_SLOTS:
    void setAppImage(const QString&);
    void setAppImage(QFile*);
    void setShowLog(bool);
    void setOutputDirectory(const QString&);
    void setProxy(const QNetworkProxy&);
    void start(short action = Action::Update,
	       int flags = GuiFlags::Default, 
	       QByteArray icon = QByteArray());
    void cancel();
    void clear();

Q_SIGNALS:
    void started(short);
    void canceled(short);

    /// Each action gives different information when finished
    void finished(QJsonObject info, short);
    
    void progress(int, qint64, qint64, double, QString, short);
    void logger(QString, QString); 
    void error(short, short);
private:
    short n_CurrentAction = Action::Update;
    bool b_Started = false,
	 b_Finished = false,
	 b_Canceled = false,
	 b_Running = false,
	 b_CancelRequested = false;
    QScopedPointer<AppImageUpdateInformationPrivate> m_UpdateInformation;
    QScopedPointer<ZsyncRemoteControlFileParserPrivate> m_ControlFileParser;
    QScopedPointer<ZsyncWriterPrivate> m_DeltaWriter;
    QScopedPointer<ZsyncBlockRangeDownloaderPrivate> m_BlockDownloader;
    QScopedPointer<QThread> m_SharedThread;
    QScopedPointer<QNetworkAccessManager> m_SharedNetworkAccessManager;
};

#endif // QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
