#ifdef BUILD_AS_PLUGIN
#include "qappimageupdateinterfaceimpl.hpp"

QAppImageUpdateInterfaceImpl::QAppImageUpdateInterfaceImpl(QObject *parent)
    : QObject(parent) {
    m_Private.reset(new QAppImageUpdate);

    auto s = m_Private.data();

    connect(s, &QAppImageUpdate::torrentClientStarted,
            this, &QAppImageUpdateInterfaceImpl::torrentClientStarted, 
	    Qt::DirectConnection);
    connect(s, &QAppImageUpdate::torrentStatus,
            this, &QAppImageUpdateInterfaceImpl::torrentStatus, 
	    Qt::DirectConnection);
    connect(s, &QAppImageUpdate::started,
            this, &QAppImageUpdateInterfaceImpl::started, Qt::DirectConnection);
    connect(s, &QAppImageUpdate::canceled,
            this, &QAppImageUpdateInterfaceImpl::canceled, Qt::DirectConnection);
    connect(s, &QAppImageUpdate::finished,
            this, &QAppImageUpdateInterfaceImpl::finished, Qt::DirectConnection);
    connect(s, &QAppImageUpdate::progress,
            this, &QAppImageUpdateInterfaceImpl::progress, Qt::DirectConnection);
    connect(s, &QAppImageUpdate::logger,
            this, &QAppImageUpdateInterfaceImpl::logger, Qt::DirectConnection);
    connect(s, &QAppImageUpdate::error,
            this, &QAppImageUpdateInterfaceImpl::error, Qt::DirectConnection);
}

QAppImageUpdateInterfaceImpl::~QAppImageUpdateInterfaceImpl() {
}

void QAppImageUpdateInterfaceImpl::setIcon(QByteArray icon) {
    m_Private->setIcon(icon);
}

void QAppImageUpdateInterfaceImpl::setGuiFlag(int flags) {
    m_Private->setGuiFlag(flags);
}

void QAppImageUpdateInterfaceImpl::setAppImagePath(const QString &a) {
    m_Private->setAppImage(a);
}

void QAppImageUpdateInterfaceImpl::setAppImageFile(QFile *a) {
    m_Private->setAppImage(a);
}

void QAppImageUpdateInterfaceImpl::setShowLog(bool a) {
    m_Private->setShowLog(a);
}

void QAppImageUpdateInterfaceImpl::setOutputDirectory(const QString &a) {
    m_Private->setOutputDirectory(a);
}

void QAppImageUpdateInterfaceImpl::setProxy(const QNetworkProxy &a) {
    m_Private->setProxy(a);
}

void QAppImageUpdateInterfaceImpl::start(short action) {
    m_Private->start(action);
}

void QAppImageUpdateInterfaceImpl::cancel() {
    m_Private->cancel();
}

void QAppImageUpdateInterfaceImpl::clear() {
    m_Private->clear();
}

int QAppImageUpdateInterfaceImpl::getConstant(const QString &constant) {
    int r = 0;
    auto split = constant.split("::");
    if(split.size() != 2) {
        return r;
    }

    auto type = split[0].toLower();
    auto constName = split[1].toLower();

    if(type == QString::fromUtf8("action")) {
        if(constName == QString::fromUtf8("getembeddedinfo")) {
            r = QAppImageUpdate::Action::GetEmbeddedInfo;
        } else if(constName == QString::fromUtf8("checkforupdate")) {
            r = QAppImageUpdate::Action::CheckForUpdate;
        } else if(constName == QString::fromUtf8("update")) {
            r = QAppImageUpdate::Action::Update;
        } else if(constName == QString::fromUtf8("updatewithtorrent")) {
            r = QAppImageUpdate::Action::UpdateWithTorrent;
        } else if(constName == QString::fromUtf8("updatewithgui")) {
            r = QAppImageUpdate::Action::UpdateWithGUI;
        } else if(constName == QString::fromUtf8("updatewithguiandtorrent")) {
            r = QAppImageUpdate::Action::UpdateWithGUIAndTorrent;
        } else {
            r = 0;
        }
    } else if(type == QString::fromUtf8("guiflag")) {

        if(constName == QString::fromUtf8("showprogressdialog")) {
            r = QAppImageUpdate::GuiFlag::ShowProgressDialog;
        } else if(constName == QString::fromUtf8("showbeforeprogress")) {
            r = QAppImageUpdate::GuiFlag::ShowBeforeProgress;
        } else if(constName == QString::fromUtf8("showupdateconfirmationdialog")) {
            r = QAppImageUpdate::GuiFlag::ShowUpdateConfirmationDialog;
        } else if(constName == QString::fromUtf8("showfinisheddialog")) {
            r = QAppImageUpdate::GuiFlag::ShowFinishedDialog;
        } else if(constName == QString::fromUtf8("showerrordialog")) {
            r = QAppImageUpdate::GuiFlag::ShowErrorDialog;
        } else if(constName == QString::fromUtf8("noshowerrordialogonpermissionerrors")) {
            r = QAppImageUpdate::GuiFlag::NoShowErrorDialogOnPermissionErrors;
        } else if(constName == QString::fromUtf8("noconfirmtorrentusage")) {
            r = QAppImageUpdate::GuiFlag::NoConfirmTorrentUsage;
        } else if(constName == QString::fromUtf8("notifywhennoupdateisavailable")) {
            r = QAppImageUpdate::GuiFlag::NotifyWhenNoUpdateIsAvailable;
        } else if(constName == QString::fromUtf8("noremindmelaterbutton")) {
            r = QAppImageUpdate::GuiFlag::NoRemindMeLaterButton;
        } else if(constName == QString::fromUtf8("noskipthisversionbutton")) {
            r = QAppImageUpdate::GuiFlag::NoSkipThisVersionButton;
        } else if(constName == QString::fromUtf8("default")) {
            r = QAppImageUpdate::GuiFlag::Default;
        } else  {
            r = 0;
        }
    }

    return r;
}

QObject *QAppImageUpdateInterfaceImpl::getObject() {
    return (QObject*)this;
}

QString QAppImageUpdateInterfaceImpl::errorCodeToString(short a) {
    return QAppImageUpdate::errorCodeToString(a);
}

QString QAppImageUpdateInterfaceImpl::errorCodeToDescriptionString(short a) {
    return QAppImageUpdate::errorCodeToDescriptionString(a);
}
#endif // BUILD_AS_PLUGIN
