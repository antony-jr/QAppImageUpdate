#ifndef QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
#define QAPPIMAGE_UPDATE_PRIVATE_INCLUDED
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QNetworkProxy>

#include "qappimageupdatecodes_p.hpp"
#include "appimageupdateinformation_p.hpp"
#include "zsyncremotecontrolfileparser_p.hpp"
#include "zsyncwriter_p.hpp"
#include "zsyncblockrangedownloader_p.hpp"

class QAppImageUpdatePrivate : public QObject, public QAppImageUpdateCodesPrivate {
	Q_OBJECT
public:
    QAppImageUpdatePrivate();
    ~QAppImageUpdatePrivate();

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

Q_SIGNALS:
    void started(short);
    void canceled(short);

    /// Each action gives different information when finished
    void finished(QJsonObject info, short);
    
    void progress(int, qint64, qint64, double, QString, short);
    void logger(QString, QString); 
    void error(short, short);
private:
    Action n_CurrentAction = Action::None;
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
