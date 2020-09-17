#ifdef BUILD_AS_PLUGIN
#include "qappimageupdateinterfaceimpl.hpp"

QAppImageUpdateInterfaceImpl::QAppImageUpdateInterfaceImpl(QObject *parent)
	: QObject(parent)
{
	m_Private.reset(new QAppImageUpdate);

	auto s = m_Private.data();

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

QString QAppImageUpdateInterfaceImpl::errorCodeToString(short a) {
	return QAppImageUpdate::errorCodeToString(a);
}

QString QAppImageUpdateInterfaceImpl::errorCodeToDescriptionString(short a) {
	return QAppImageUpdate::errorCodeToDescriptionString(a);
}
#endif // BUILD_AS_PLUGIN
