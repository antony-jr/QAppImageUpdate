#ifndef APPIMAGE_UPDATER_WIDGET_HPP_INCLUDED
#define APPIMAGE_UPDATER_WIDGET_HPP_INCLUDED
#include <QPixmap>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QMutex>
#include <QMetaObject>
#include <QMetaMethod>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

/* AppImage Updater Bridge libraries. */
#include <AppImageDeltaRevisioner.hpp>
#include <AppImageUpdaterBridgeErrorCodes.hpp>
#include <AppImageUpdaterBridgeStatusCodes.hpp>

namespace AppImageUpdaterBridge
{
class AppImageUpdaterWidget : public QWidget
{
    Q_OBJECT
public:
    AppImageUpdaterWidget(int idleSeconds = 0, QWidget *parent = nullptr);
    ~AppImageUpdaterWidget();

    virtual bool continueWithUpdate(QJsonObject info)
    {
        (void)info;
        return true;
    }

    virtual bool openNewVersion(QJsonObject info)
    {
        (void)info;
        return false;
    }
public Q_SLOTS:
    void init(void);
    
    void setAppImage(const QString&);
    void setAppImage(QFile *);
    void setShowBeforeStarted(bool);
    void setShowLog(bool);
    void setIconPixmap(const QPixmap&);
    void resetIdleTimer(void);

private Q_SLOTS:
    void showWidget(void);
    void handleIdleTimerTimeout(void);
    void handleUpdateAvailable(bool, QJsonObject);
    void handleError(short);
    void handleFinished(QJsonObject, QString);
    void handleProgress(int, qint64, qint64, double, QString);

Q_SIGNALS:
    void quit(void);
    void started(void);
    void canceled(void);
    void error(QString, short);
    void finished(QJsonObject);

private:
    QMutex _pMutex;
    QTimer _pIdleTimer;
    bool _bShowBeforeStarted = false;
    AppImageDeltaRevisioner *_pDRevisioner = nullptr;
    QGridLayout *_pGridLayout = nullptr;
    QLabel *_pStatusLbl = nullptr;
    QLabel *_pIconLbl = nullptr;
    QProgressBar *_pProgressBar = nullptr;
    QPushButton *_pCancelBtn = nullptr;
};
}

#endif // APPIMAGE_UPDATER_WIDGET_HPP_INCLUDED
