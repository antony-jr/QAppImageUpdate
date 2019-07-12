#include <QApplication>
#include <QCryptographicHash>
#include <QScreen>

#include "../include/softwareupdatedialog_p.hpp"

static bool isSkipThisVersion(const QString &id, QScopedPointer<QSettings> &settings){
	if(settings.isNull()){
		return false;
	}
	QCryptographicHash hasher(QCryptographicHash::Md5);
	hasher.addData(id.toUtf8());
	QString hash(hasher.result().toHex());
	return settings->value(hash).toBool();
}

SoftwareUpdateDialog::SoftwareUpdateDialog(QWidget *parent , QPixmap icon ,int flags)
	: QDialog(parent , Qt::WindowStaysOnTopHint)
{
	m_Icon = icon;
	m_Ui.setupUi(this);

	if((flags & NoRemindMeLaterButton) && (flags & NoSkipThisVersionButton)){
		b_NoUseSettings = true;
		(m_Ui.autoUpdateCheckBox)->setVisible(false);
	}else{
	m_Settings.reset(new QSettings(QSettings::UserScope ,
			           QString::fromUtf8("updatedeployqt") , 
				   QApplication::applicationName()));
	}
	
	connect(&m_Timer , &QTimer::timeout , this , &SoftwareUpdateDialog::pInit);

	if(flags & NoRemindMeLaterButton){
		(m_Ui.remindMeLater)->setVisible(false);
	}else{
		connect(m_Ui.remindMeLater , &QPushButton::clicked , this , &SoftwareUpdateDialog::handleRemindMeLater);
	}

	if(flags & NoSkipThisVersionButton){
		(m_Ui.skipVersionBtn)->setVisible(false);
	}else{
		connect(m_Ui.skipVersionBtn , &QPushButton::clicked , this , &SoftwareUpdateDialog::handleSkipThisUpdate);
	}
	connect(m_Ui.installUpdateBtn , &QPushButton::clicked , this , &SoftwareUpdateDialog::handleInstallUpdate);	
	return;
}

SoftwareUpdateDialog::~SoftwareUpdateDialog()
{
}

void SoftwareUpdateDialog::init(const QString &AppName , const QString &OldVersion ,
		                const QString &NewVersion , const QString &ReleaseNotes){

	m_Id.clear();
	m_Id.append(AppName);
        m_Id.append(OldVersion);
	m_Id.append(NewVersion);
	m_Id.append(ReleaseNotes);

	if(!b_NoUseSettings){
	/* Check if the user choose to skip this version update. */
	if(isSkipThisVersion(m_Id , m_Settings)){
		setResult(QDialog::Rejected);
		emit rejected();
		return;
	}

	/* Check if we want to automatically download and install update. */
	if(m_Settings->value("autoUpdateOnFuture").toBool()){
		setResult(QDialog::Accepted);
		emit accepted();
		return;
	}
	
	
	/* Set if the users want to update automatically in the future. */
	m_Settings->setValue("autoUpdateOnFuture", 
			((m_Ui.autoUpdateCheckBox)->checkState() == Qt::Checked) ? true : false);
 
	}

	/* Set the title , description , and release notes. */
	(m_Ui.titleLbl)->setText(m_TitleTemplate.arg(AppName));
	(m_Ui.descLbl)->setText(m_VersionDescTemplate.arg(AppName , NewVersion , OldVersion));
	(m_Ui.releaseNotesTxtBox)->setHtml(ReleaseNotes);

	setWindowIcon(m_Icon);	
	(m_Ui.softwareIcon)->setPixmap(m_Icon);

	/* Center the dialog in the primary screen. */
	move(QGuiApplication::primaryScreen()->geometry().center() - this->rect().center());
	show();
	m_Timer.stop();
}

/* private handles */

void SoftwareUpdateDialog::pInit(){
	move(QGuiApplication::primaryScreen()->geometry().center() - this->rect().center());
	show();
	m_Timer.stop();
}

void SoftwareUpdateDialog::handleRemindMeLater(){
	hide();
	m_Timer.setInterval(3600 * 1000); /* 1 hour interval */
	m_Timer.setSingleShot(true);
	m_Timer.start();
}

void SoftwareUpdateDialog::handleInstallUpdate(){
	hide();
	m_Timer.stop();
	setResult(QDialog::Accepted);
	emit accepted();
}

void SoftwareUpdateDialog::handleSkipThisUpdate(){
	hide();
	m_Timer.stop();

	QCryptographicHash hasher(QCryptographicHash::Md5);
	hasher.addData(m_Id.toUtf8());
	QString hash(hasher.result().toHex());
	m_Settings->setValue(hash , true);

	setResult(QDialog::Rejected);
	emit rejected();
}
