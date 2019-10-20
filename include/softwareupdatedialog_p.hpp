#ifndef SOFTWARE_UPDATE_WIDGET_HPP_INCLUDED
#define SOFTWARE_UPDATE_WIDGET_HPP_INCLUDED
#include <QDialog>
#include <QPixmap>
#include <QSettings>
#include <QTimer>
#include <QString>
#include <QScopedPointer>
#include <ui_SoftwareUpdateDialog.h>

class SoftwareUpdateDialog : public QDialog {
    Q_OBJECT
  public:
    enum {
        NoRemindMeLaterButton = 0x80,
        NoSkipThisVersionButton = 0x100,
        Default = 0
    };
    SoftwareUpdateDialog(QWidget *parent = nullptr, QPixmap icon = QPixmap(), int flags = Default);
    ~SoftwareUpdateDialog();
  public Q_SLOTS:
    void init(const QString&,const QString&,const QString&,const QString&);
  private Q_SLOTS:
    void pInit();
    void handleRemindMeLater();
    void handleSkipThisUpdate();
    void handleInstallUpdate();
  private:
    bool b_NoUseSettings = false;
    const QString m_TitleTemplate =
        QString::fromUtf8("A new version of %1 is available!");
    const QString m_VersionDescTemplate =
        QString::fromUtf8("%1 (%2) is now available--you have (%3). Would you like to download it ?");
    QPixmap m_Icon;
    QTimer m_Timer;
    QString m_Id;
    QScopedPointer<QSettings> m_Settings;
    Ui::SoftwareUpdateDialog m_Ui;
};

#endif // SOFTWARE_UPDATE_WIDGET_HPP_INCLUDED
