#ifndef APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
#define APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
#include <QPixmap>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QMutex>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include <QMessageBox>

#include "appimageupdaterbridge_enums.hpp"
#include "appimagedeltarevisioner.hpp"

namespace AppImageUpdaterBridge
{
class AppImageUpdaterDialog : public QDialog
{
    Q_OBJECT
public:
    enum {
	    ShowProgressDialog = 0x1,
	    ShowBeforeProgress = 0x2,
	    ShowUpdateConfirmationDialog = 0x4,
	    ShowFinishedDialog = 0x8,
	    ShowErrorDialog = 0x10,
	    AlertWhenAuthorizationIsRequired = 0x20,
	    NotifyWhenNoUpdateIsAvailable = 0x40,
	    Default = ShowProgressDialog |
		      ShowUpdateConfirmationDialog |
	              ShowFinishedDialog   |
		      ShowErrorDialog |
		      NotifyWhenNoUpdateIsAvailable	
    };

    AppImageUpdaterDialog(QPixmap img = QPixmap(), 
		 	  QWidget *parent = nullptr , int flags = Default);
    AppImageUpdaterDialog(const QString&,
		          QPixmap img = QPixmap(), 
			  QWidget *parent = nullptr , int flags = Default);
    AppImageUpdaterDialog(QFile*,
		          QPixmap img = QPixmap(), 
			  QWidget *parent = nullptr , int flags = Default);
    ~AppImageUpdaterDialog();

public Q_SLOTS:
    void init();

private Q_SLOTS:
    void showWidget(void);
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
    void requiresAuthorization(QString , short , QString);

private:
    int p_Flags = 0;
    QPixmap p_AppImageIcon;
    QString s_CurrentAppImagePath; /* Used only for error dialog box. */
    QGridLayout *p_GridLayout = nullptr;
    QLabel *p_StatusLbl = nullptr;
    QLabel *p_IconLbl = nullptr;
    QProgressBar *p_ProgressBar = nullptr;
    QPushButton *p_CancelBtn = nullptr;
 
    AppImageDeltaRevisioner *p_DRevisioner = nullptr;
    double n_MegaBytesTotal = 0;
    const QString progressTemplate = QString::fromUtf8("Updating %1 MiB of %2 MiB at %3 %4...");
};
}

#endif // APPIMAGE_UPDATER_DIALOG_HPP_INCLUDED
