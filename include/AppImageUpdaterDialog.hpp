#ifndef APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
#define APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
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
#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include <QMessageBox>

/* AppImage Updater Bridge libraries. */
#include <AppImageDeltaRevisioner.hpp>
#include <AppImageUpdaterBridgeErrorCodes.hpp>
#include <AppImageUpdaterBridgeStatusCodes.hpp>

namespace AppImageUpdaterBridge
{
class AppImageUpdaterDialog : public QDialog
{
    Q_OBJECT
public:
    AppImageUpdaterDialog(int idleSeconds = 0, QWidget *parent = nullptr);
    ~AppImageUpdaterDialog();
public Q_SLOTS:
    void init(void);

    void setAppImage(const QString&);
    void setAppImage(QFile *);
    void setMovePoint(const QPoint&);
    void setShowProgressDialog(bool);
    void setShowUpdateConfirmationDialog(bool);
    void setShowNoUpdateDialog(bool);
    void setShowFinishDialog(bool);
    void setShowErrorDialog(bool);
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
    QPoint _pMovePoint;
    QString _sCurrentAppImagePath; /* Used only for error dialog box. */
    QPixmap _pAppImageIcon; /* in 100x100 pixels. */
    double _nMegaBytesTotal = 0;
    const QString progressTemplate = QString::fromUtf8("Updating %1 MiB of %2 MiB at %3 %4...");
    bool _bShowProgressDialog = true ,
	 _bShowBeforeStarted = false,
         _bShowUpdateConfirmationDialog = false,
         _bShowFinishDialog = false,
         _bShowErrorDialog = false,
         _bShowNoUpdateDialog = false;
    AppImageDeltaRevisioner *_pDRevisioner = nullptr;
    QGridLayout *_pGridLayout = nullptr;
    QLabel *_pStatusLbl = nullptr;
    QLabel *_pIconLbl = nullptr;
    QProgressBar *_pProgressBar = nullptr;
    QPushButton *_pCancelBtn = nullptr;
};
}

#endif // APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
