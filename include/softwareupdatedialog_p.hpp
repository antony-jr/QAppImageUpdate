#ifndef SOFTWARE_UPDATE_WIDGET_HPP_INCLUDED
#define SOFTWARE_UPDATE_WIDGET_HPP_INCLUDED
#include <QDialog>
#include <QPixmap>
#include <QSettings>
#include <QTimer>
#include <QString>
#include <QScopedPointer>


/* output from uic SoftwareUpdateDialog.ui ,
 * This copied because in some dev environments we can't 
 * include the ui header file, maybe possible but its
 * tedious and to solve that we simply copy that here. */
#ifndef UI_SOFTWAREUPDATEDIALOG_H
#define UI_SOFTWAREUPDATEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_SoftwareUpdateDialog
{
public:
    QGridLayout *gridLayout;
    QGridLayout *mainLayout;
    QTextEdit *releaseNotesTxtBox;
    QLabel *releaseNotesConst;
    QLabel *titleLbl;
    QLabel *softwareIcon;
    QLabel *descLbl;
    QPushButton *installUpdateBtn;
    QPushButton *remindMeLater;
    QPushButton *skipVersionBtn;
    QCheckBox *autoUpdateCheckBox;

    void setupUi(QDialog *SoftwareUpdateDialog)
    {
        if (SoftwareUpdateDialog->objectName().isEmpty())
            SoftwareUpdateDialog->setObjectName(QString::fromUtf8("SoftwareUpdateDialog"));
        SoftwareUpdateDialog->setWindowModality(Qt::ApplicationModal);
        SoftwareUpdateDialog->resize(725, 355);
        gridLayout = new QGridLayout(SoftwareUpdateDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        mainLayout = new QGridLayout();
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        releaseNotesTxtBox = new QTextEdit(SoftwareUpdateDialog);
        releaseNotesTxtBox->setObjectName(QString::fromUtf8("releaseNotesTxtBox"));
        releaseNotesTxtBox->setReadOnly(true);

        mainLayout->addWidget(releaseNotesTxtBox, 3, 1, 1, 4);

        releaseNotesConst = new QLabel(SoftwareUpdateDialog);
        releaseNotesConst->setObjectName(QString::fromUtf8("releaseNotesConst"));
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        font.setWeight(75);
        releaseNotesConst->setFont(font);

        mainLayout->addWidget(releaseNotesConst, 2, 1, 1, 4);

        titleLbl = new QLabel(SoftwareUpdateDialog);
        titleLbl->setObjectName(QString::fromUtf8("titleLbl"));
        QFont font1;
        font1.setPointSize(12);
        font1.setBold(true);
        font1.setWeight(75);
        titleLbl->setFont(font1);

        mainLayout->addWidget(titleLbl, 0, 1, 1, 4);

        softwareIcon = new QLabel(SoftwareUpdateDialog);
        softwareIcon->setObjectName(QString::fromUtf8("softwareIcon"));
        softwareIcon->setMinimumSize(QSize(125, 125));
        softwareIcon->setMaximumSize(QSize(125, 125));
        softwareIcon->setScaledContents(true);
        softwareIcon->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(softwareIcon, 0, 0, 3, 1);

        descLbl = new QLabel(SoftwareUpdateDialog);
        descLbl->setObjectName(QString::fromUtf8("descLbl"));
        QFont font2;
        font2.setPointSize(10);
        font2.setBold(false);
        font2.setWeight(50);
        descLbl->setFont(font2);
        descLbl->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        descLbl->setWordWrap(true);

        mainLayout->addWidget(descLbl, 1, 1, 1, 4);

        installUpdateBtn = new QPushButton(SoftwareUpdateDialog);
        installUpdateBtn->setObjectName(QString::fromUtf8("installUpdateBtn"));

        mainLayout->addWidget(installUpdateBtn, 5, 4, 1, 1);

        remindMeLater = new QPushButton(SoftwareUpdateDialog);
        remindMeLater->setObjectName(QString::fromUtf8("remindMeLater"));

        mainLayout->addWidget(remindMeLater, 5, 3, 1, 1);

        skipVersionBtn = new QPushButton(SoftwareUpdateDialog);
        skipVersionBtn->setObjectName(QString::fromUtf8("skipVersionBtn"));

        mainLayout->addWidget(skipVersionBtn, 5, 1, 1, 1);

        autoUpdateCheckBox = new QCheckBox(SoftwareUpdateDialog);
        autoUpdateCheckBox->setObjectName(QString::fromUtf8("autoUpdateCheckBox"));

        mainLayout->addWidget(autoUpdateCheckBox, 4, 1, 1, 4);


        gridLayout->addLayout(mainLayout, 0, 0, 1, 1);


        retranslateUi(SoftwareUpdateDialog);

        QMetaObject::connectSlotsByName(SoftwareUpdateDialog);
    } // setupUi

    void retranslateUi(QDialog *SoftwareUpdateDialog)
    {
        SoftwareUpdateDialog->setWindowTitle(QApplication::translate("SoftwareUpdateDialog", "Software Update", nullptr));
        releaseNotesConst->setText(QApplication::translate("SoftwareUpdateDialog", "Release Notes:", nullptr));
        titleLbl->setText(QString());
        softwareIcon->setText(QString());
        descLbl->setText(QString());
        installUpdateBtn->setText(QApplication::translate("SoftwareUpdateDialog", "Install Update", nullptr));
        remindMeLater->setText(QApplication::translate("SoftwareUpdateDialog", "Remind Me Later", nullptr));
        skipVersionBtn->setText(QApplication::translate("SoftwareUpdateDialog", "Skip This Version", nullptr));
        autoUpdateCheckBox->setText(QApplication::translate("SoftwareUpdateDialog", "Automatically download and install updates in the future", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SoftwareUpdateDialog: public Ui_SoftwareUpdateDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOFTWAREUPDATEDIALOG_H

class SoftwareUpdateDialog : public QDialog {
	Q_OBJECT
public:
	enum {
		NoRemindMeLaterButton = 0x80,
		NoSkipThisVersionButton = 0x100,
		Default = 0
	};
	SoftwareUpdateDialog(QWidget *parent = nullptr, QPixmap icon = QPixmap() , int flags = Default);	
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
