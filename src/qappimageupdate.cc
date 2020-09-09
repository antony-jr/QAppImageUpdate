#include "qappimageupdate.hpp"
#include "qappimageupdate_p.hpp"
#include "helpers_p.hpp"

QAppImageUpdate::QAppImageUpdate(bool singleThreaded, QObject *parent) {
	m_Private = QSharedPointer<QAppImageUpdatePrivate>(
			new QAppImageUpdatePrivate(singleThreaded = singleThreaded, parent=parent));
	auto s = m_Private.data();

	connect(s, &QAppImageUpdatePrivate::started,
		this, &QAppImageUpdate::started, Qt::DirectConnection);
	connect(s, &QAppImageUpdatePrivate::canceled,
		this, &QAppImageUpdate::canceled, Qt::DirectConnection);
	connect(s, &QAppImageUpdatePrivate::finished,
		this, &QAppImageUpdate::finished, Qt::DirectConnection);
	connect(s, &QAppImageUpdatePrivate::progress,
		this, &QAppImageUpdate::progress, Qt::DirectConnection);
	connect(s, &QAppImageUpdatePrivate::logger,
		this, &QAppImageUpdate::logger, Qt::DirectConnection);
	connect(s, &QAppImageUpdatePrivate::error,
		this, &QAppImageUpdate::error, Qt::DirectConnection);
}

QAppImageUpdate::QAppImageUpdate(const QString &AppImagePath, bool singleThreaded, QObject *parent)
	: QAppImageUpdate(singleThreaded, parent) {
	setAppImage(AppImagePath);
}

QAppImageUpdate::QAppImageUpdate(QFile *AppImage, bool singleThreaded, QObject *parent)
	: QAppImageUpdate(singleThreaded, parent) {
	setAppImage(AppImage);
}

QAppImageUpdate::~QAppImageUpdate() { }

void QAppImageUpdate::setAppImage(const QString &AppImagePath) {
    getMethod(m_Private.data(), "setAppImage(const QString&)")
	    .invoke(m_Private.data(), 
		    Qt::QueuedConnection,
                    Q_ARG(QString,AppImagePath)); 
}
void QAppImageUpdate::setAppImage(QFile *AppImage) {
    getMethod(m_Private.data(), "setAppImage(QFile*)")
	    .invoke(m_Private.data(), 
		    Qt::QueuedConnection,
                    Q_ARG(QFile,AppImage)); 
}
void QAppImageUpdate::setShowLog(bool boolean) {
    getMethod(m_Private.data(), "setShowLog(bool)")
	    .invoke(m_Private.data(), 
		    Qt::QueuedConnection,
                    Q_ARG(bool,boolean));
}

void QAppImageUpdate::setOutputDirectory(const QString &OutputDirectory) {
    getMethod(m_Private.data(), "setOutputDirectory(const QString&)")
	    .invoke(m_Private.data(), 
		    Qt::QueuedConnection,
                    Q_ARG(QString,OutputDirectory)); 
}

void QAppImageUpdate::setProxy(const QNetworkProxy &Proxy) {
    getMethod(m_Private.data(), "setProxy(const QNetworkProxy&)")
	    .invoke(m_Private.data(), 
		    Qt::QueuedConnection,
                    Q_ARG(QNetworkProxy, Proxy));
}

void QAppImageUpdate::start(short action = Action::Update, 
		            int flags = GuiFlag::Default,
		            QByteArray icon = QByteArray()) {
    getMethod(m_Private.data(), "start(short, int, QByteArray)")
	    .invoke(m_Private.data(), 
		    Qt::QueuedConnection,
                    Q_ARG(short, action),
		    Q_ARG(int, flags),
		    Q_ARG(QByteArray, icon));

}
 
void QAppImageUpdate::cancel() {
    getMethod(m_Private.data(), "cancel()")
	    .invoke(m_Private.data(), 
		    Qt::QueuedConnection);
}

void QAppImageUpdate::clear() {
    getMethod(m_Private.data(), "clear()")
	    .invoke(m_Private.data(), 
		    Qt::QueuedConnection);
}


//// Static Methods
QString QAppImageUpdate::errorCodeToString(short errorCode) {
    QString ret = "AppImageUpdaterBridge::errorCode(";
    switch(errorCode) {
    case QAppImageUpdateEnums::Error::NoAppimagePathGiven:
        ret += "NoAppImagePathGiven";
        break;
    case QAppImageUpdateEnums::Error::AppimageNotReadable:
        ret += "AppImageNotReadable";
        break;
    case QAppImageUpdateEnums::Error::NoReadPermission:
        ret += "NoReadPermission";
        break;
    case QAppImageUpdateEnums::Error::AppimageNotFound:
        ret += "AppimageNotFound";
        break;
    case QAppImageUpdateEnums::Error::CannotOpenAppimage:
        ret += "CannnotOpenAppimage";
        break;
    case QAppImageUpdateEnums::Error::EmptyUpdateInformation:
        ret += "EmptyUpdateInformation";
        break;
    case QAppImageUpdateEnums::Error::InvalidAppimageType:
        ret += "InvalidAppimageType";
        break;
    case QAppImageUpdateEnums::Error::InvalidMagicBytes:
        ret += "InvalidMagicBytes";
        break;
    case QAppImageUpdateEnums::Error::InvalidUpdateInformation:
        ret += "InvalidUpdateInformation";
        break;
    case QAppImageUpdateEnums::Error::NotEnoughMemory:
        ret += "NotEnoughMemory";
        break;
    case QAppImageUpdateEnums::Error::SectionHeaderNotFound:
        ret += "SectionHeaderNotFound";
        break;
    case QAppImageUpdateEnums::Error::UnsupportedElfFormat:
        ret += "UnsupportedElfFormat";
        break;
    case QAppImageUpdateEnums::Error::UnsupportedTransport:
        ret += "UnsupportedTransport";
        break;
    case QAppImageUpdateEnums::Error::UnknownNetworkError:
        ret += "UnknownNetworkError";
        break;
    case QAppImageUpdateEnums::Error::IoReadError:
        ret += "IoReadError";
        break;
    case QAppImageUpdateEnums::Error::ErrorResponseCode:
        ret += "ErrorResponseCode";
        break;
    case QAppImageUpdateEnums::Error::GithubApiRateLimitReached:
        ret += "GithubApiRateLimitReached";
        break;
    case QAppImageUpdateEnums::Error::NoMarkerFoundInControlFile:
        ret += "NoMarkerFoundInControlFile";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncHeadersNumber:
        ret += "InvalidZsyncHeadersNumber";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncMakeVersion:
        ret += "InvalidZsyncMakeVersion";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncTargetFilename:
        ret += "InvalidZsyncTargetFilename";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncMtime:
        ret += "InvalidZsyncMtime";
        break;
    case QAppImageUpdateEnums::Error::InvalidZsyncBlocksize:
        ret += "InvalidZsyncBlocksize";
        break;
    case QAppImageUpdateEnums::Error::InvalidTargetFileLength:
        ret += "InvalidTargetFileLength";
        break;
    case QAppImageUpdateEnums::Error::InvalidHashLengthLine:
        ret += "InvalidHashLengthLine";
        break;
    case QAppImageUpdateEnums::Error::InvalidHashLengths:
        ret += "InvalidHashLengths";
        break;
    case QAppImageUpdateEnums::Error::InvalidTargetFileUrl:
        ret += "InvalidTargetFileUrl";
        break;
    case QAppImageUpdateEnums::Error::InvalidTargetFileSha1:
        ret += "InvalidTargetFileSha1";
        break;
    case QAppImageUpdateEnums::Error::ConnectionRefusedError:
        ret += "ConnectionRefusedError";
        break;
    case QAppImageUpdateEnums::Error::RemoteHostClosedError:
        ret += "RemoteHostClosedError";
        break;
    case QAppImageUpdateEnums::Error::HostNotFoundError:
        ret += "HostNotFoundError";
        break;
    case QAppImageUpdateEnums::Error::TimeoutError:
        ret += "TimeoutError";
        break;
    case QAppImageUpdateEnums::Error::OperationCanceledError:
        ret += "OperationCanceledError";
        break;
    case QAppImageUpdateEnums::Error::SslHandshakeFailedError:
        ret += "SslHandshakeFailedError";
        break;
    case QAppImageUpdateEnums::Error::TemporaryNetworkFailureError:
        ret += "TemporaryNetworkFailureError";
        break;
    case QAppImageUpdateEnums::Error::NetworkSessionFailedError:
        ret += "NetworkSessionFailedError";
        break;
    case QAppImageUpdateEnums::Error::BackgroundRequestNotAllowedError:
        ret += "BackgroundRequestNotAllowedError";
        break;
    case QAppImageUpdateEnums::Error::TooManyRedirectsError:
        ret += "TooManyRedirectsError";
        break;
    case QAppImageUpdateEnums::Error::InsecureRedirectError:
        ret += "InsecureRedirectError";
        break;
    case QAppImageUpdateEnums::Error::ProxyConnectionRefusedError:
        ret += "ProxyConnectionRefusedError";
        break;
    case QAppImageUpdateEnums::Error::ProxyConnectionClosedError:
        ret += "ProxyConnectionClosedError";
        break;
    case QAppImageUpdateEnums::Error::ProxyNotFoundError:
        ret += "ProxyNotFoundError";
        break;
    case QAppImageUpdateEnums::Error::ProxyTimeoutError:
        ret += "ProxyTimeoutError";
        break;
    case QAppImageUpdateEnums::Error::ProxyAuthenticationRequiredError:
        ret += "ProxyAuthenticationRequiredError";
        break;
    case QAppImageUpdateEnums::Error::ContentAccessDenied:
        ret += "ContentAccessDenied";
        break;
    case QAppImageUpdateEnums::Error::ContentOperationNotPermittedError:
        ret += "ContentOperationNotPermittedError";
        break;
    case QAppImageUpdateEnums::Error::ContentNotFoundError:
        ret += "ContentNotFoundError";
        break;
    case QAppImageUpdateEnums::Error::AuthenticationRequiredError:
        ret += "AuthenticationRequiredError";
        break;
    case QAppImageUpdateEnums::Error::ContentReSendError:
        ret += "ContentReSendError";
        break;
    case QAppImageUpdateEnums::Error::ContentConflictError:
        ret += "ContentConflictError";
        break;
    case QAppImageUpdateEnums::Error::ContentGoneError:
        ret += "ContentGoneError";
        break;
    case QAppImageUpdateEnums::Error::InternalServerError:
        ret += "InternalServerError";
        break;
    case QAppImageUpdateEnums::Error::OperationNotImplementedError:
        ret += "OperationNotImplementedError";
        break;
    case QAppImageUpdateEnums::Error::ServiceUnavailableError:
        ret += "ServiceUnavailableError";
        break;
    case QAppImageUpdateEnums::Error::ProtocolUnknownError:
        ret += "ProtocolUnknownError";
        break;
    case QAppImageUpdateEnums::Error::ProtocolInvalidOperationError:
        ret += "ProtocolInvalidOperationError";
        break;
    case QAppImageUpdateEnums::Error::UnknownProxyError:
        ret += "UnknownProxyError";
        break;
    case QAppImageUpdateEnums::Error::UnknownContentError:
        ret += "UnknownContentError";
        break;
    case QAppImageUpdateEnums::Error::ProtocolFailure:
        ret += "ProtocolFailure";
        break;
    case QAppImageUpdateEnums::Error::UnknownServerError:
        ret += "UnknownServerError";
        break;
    case QAppImageUpdateEnums::Error::ZsyncControlFileNotFound:
        ret += "ZsyncControlFileNotFound";
        break;
    case QAppImageUpdateEnums::Error::HashTableNotAllocated:
        ret += "HashTableNotAllocated";
        break;
    case QAppImageUpdateEnums::Error::InvalidTargetFileChecksumBlocks:
        ret += "InvalidTargetFileChecksumBlocks";
        break;
    case QAppImageUpdateEnums::Error::CannotConstructHashTable:
        ret += "CannotConstructHashTable";
        break;
    case QAppImageUpdateEnums::Error::CannotOpenTargetFileChecksumBlocks:
        ret += "CannotOpenTargetFileChecksumBlocks";
        break;
    case QAppImageUpdateEnums::Error::QbufferIoReadError:
        ret += "QbufferIoReadError";
        break;
    case QAppImageUpdateEnums::Error::SourceFileNotFound:
        ret += "SourceFileNotFound";
        break;
    case QAppImageUpdateEnums::Error::NoPermissionToReadSourceFile:
        ret += "NoPermissionToReadSourceFile";
        break;
    case QAppImageUpdateEnums::Error::CannotOpenSourceFile:
        ret += "CannotOpenSourceFile";
        break;
    case QAppImageUpdateEnums::Error::NoPermissionToReadWriteTargetFile:
        ret += "NoPermissionToReadWriteTargetFile";
        break;
    case QAppImageUpdateEnums::Error::CannotOpenTargetFile:
        ret += "CannotOpenTargetFile";
        break;
    case QAppImageUpdateEnums::Error::TargetFileSha1HashMismatch:
        ret += "TargetFileSha1HashMismatch";
        break;

    default:
        ret += "Unknown";
        break;
    }
    ret += ")";
    return ret;
}

QString QAppImageUpdate::errorCodeToDescriptionString(short errorCode) {
    QString errorString;
    switch(errorCode) {
    case QAppImageUpdateEnums::Error::ConnectionRefusedError:
        errorString = QString::fromUtf8("The update server is not accepting requests.");
        break;
    case QAppImageUpdateEnums::Error::RemoteHostClosedError:
        errorString = QString::fromUtf8("The remote server closed the connection prematurely, ");
        errorString += QString::fromUtf8("before the entire reply was received and processed.");
        break;
    case QAppImageUpdateEnums::Error::HostNotFoundError:
        errorString = QString::fromUtf8("The remote host name was not found (invalid hostname).");
        break;
    case QAppImageUpdateEnums::Error::TimeoutError:
        errorString = QString::fromUtf8("The connection to the remote server timed out.");
        break;
    case QAppImageUpdateEnums::Error::SslHandshakeFailedError:
        errorString = QString::fromUtf8("The SSL/TLS handshake failed and the encrypted channel ");
        errorString += QString::fromUtf8("could not be established.");
        break;
    case QAppImageUpdateEnums::Error::TemporaryNetworkFailureError:
        errorString = QString::fromUtf8("The connection to the network was broken.");
        break;
    case QAppImageUpdateEnums::Error::NetworkSessionFailedError:
        errorString = QString::fromUtf8("The connection to the network was broken ");
        errorString += QString::fromUtf8("or could not be initiated.");
        break;
    case QAppImageUpdateEnums::Error::BackgroundRequestNotAllowedError:
        errorString = QString::fromUtf8("The background request is not currently allowed due to platform policy.");
        break;
    case QAppImageUpdateEnums::Error::TooManyRedirectsError:
        errorString = QString::fromUtf8("While following redirects, the maximum limit was reached.");
        break;
    case QAppImageUpdateEnums::Error::InsecureRedirectError:
        errorString = QString::fromUtf8("While following redirects, there was a redirect ");
        errorString += QString::fromUtf8("from a encrypted protocol (https) to an unencrypted one (http).");
        break;
    case QAppImageUpdateEnums::Error::ContentAccessDenied:
        errorString = QString::fromUtf8("The access to the remote content was denied (HTTP error 403).");
        break;
    case QAppImageUpdateEnums::Error::ContentOperationNotPermittedError:
        errorString = QString::fromUtf8("The operation requested on the remote content is not permitted.");
        break;
    case QAppImageUpdateEnums::Error::ContentNotFoundError:
        errorString = QString::fromUtf8("The remote content was not found at the server (HTTP error 404)");
        break;
    case QAppImageUpdateEnums::Error::AuthenticationRequiredError:
        errorString = QString::fromUtf8("The remote server requires authentication to serve the content, ");
        errorString += QString::fromUtf8("but the credentials provided were not accepted or given.");
        break;
    case QAppImageUpdateEnums::Error::ContentConflictError:
        errorString = QString::fromUtf8("The request could not be completed due to a conflict with the ");
        errorString += QString::fromUtf8("current state of the resource.");
        break;
    case QAppImageUpdateEnums::Error::ContentGoneError:
        errorString = QString::fromUtf8("The requested resource is no longer available at the server.");
        break;
    case QAppImageUpdateEnums::Error::InternalServerError:
        errorString = QString::fromUtf8("The server encountered an unexpected condition which prevented ");
        errorString += QString::fromUtf8("it from fulfilling the request.");
        break;
    case QAppImageUpdateEnums::Error::OperationNotImplementedError:
        errorString = QString::fromUtf8("The server does not support the functionality required to fulfill the request.");
        break;
    case QAppImageUpdateEnums::Error::ServiceUnavailableError:
        errorString = QString::fromUtf8("The server is unable to handle the request at this time.");
        break;
    case QAppImageUpdateEnums::Error::ProtocolUnknownError:
        errorString = QString::fromUtf8("The Network Access API cannot honor the request because the protocol");
        errorString += QString::fromUtf8(" is not known.");
        break;
    case QAppImageUpdateEnums::Error::ProtocolInvalidOperationError:
        errorString = QString::fromUtf8("The requested operation is invalid for this protocol.");
        break;
    case QAppImageUpdateEnums::Error::UnknownNetworkError:
        errorString = QString::fromUtf8("An unknown network-related error was detected.");
        break;
    case QAppImageUpdateEnums::Error::UnknownContentError:
        errorString = QString::fromUtf8("An unknown error related to the remote content was detected.");
        break;
    case QAppImageUpdateEnums::Error::ProtocolFailure:
        errorString = QString::fromUtf8("A breakdown in protocol was detected ");
        errorString += QString::fromUtf8("(parsing error, invalid or unexpected responses, etc.)");
        break;
    case QAppImageUpdateEnums::Error::UnknownServerError:
        errorString = QString::fromUtf8("An unknown error related to the server response was detected.");
        break;
    case QAppImageUpdateEnums::Error::NoAppimagePathGiven:
        errorString = QString::fromUtf8("No AppImage given.");
        break;
    case QAppImageUpdateEnums::Error::AppimageNotReadable:
        errorString = QString::fromUtf8("The AppImage is not readable.");
        break;
    case QAppImageUpdateEnums::Error::NoReadPermission:
        errorString = QString::fromUtf8("You don't have the permission to read the AppImage.");
        break;
    case QAppImageUpdateEnums::Error::AppimageNotFound:
        errorString = QString::fromUtf8("The AppImage does not exist.");
        break;
    case QAppImageUpdateEnums::Error::CannotOpenAppimage:
        errorString = QString::fromUtf8("The AppImage cannot be opened.");
        break;
    case QAppImageUpdateEnums::Error::EmptyUpdateInformation:
        errorString = QString::fromUtf8("The AppImage does not include any update information.");
        break;
    case QAppImageUpdateEnums::Error::InvalidAppimageType:
        errorString = QString::fromUtf8("The AppImage has an unknown type.");
        break;
    case QAppImageUpdateEnums::Error::InvalidMagicBytes:
        errorString = QString::fromUtf8("The AppImage is not valid.");
        break;
    case QAppImageUpdateEnums::Error::InvalidUpdateInformation:
        errorString = QString::fromUtf8("The AppImage has invalid update information.");
        break;
    case QAppImageUpdateEnums::Error::NotEnoughMemory:
        errorString = QString::fromUtf8("Not enough memory.");
        break;
    case QAppImageUpdateEnums::Error::SectionHeaderNotFound:
        errorString = QString::fromUtf8("The AppImage does not contain update information ");
        errorString += QString::fromUtf8("at a valid section header.");
        break;
    case QAppImageUpdateEnums::Error::UnsupportedElfFormat:
        errorString = QString::fromUtf8("The AppImage is not in supported ELF format.");
        break;
    case QAppImageUpdateEnums::Error::UnsupportedTransport:
        errorString = QString::fromUtf8("The AppImage specifies an unsupported update transport.");
        break;
    case QAppImageUpdateEnums::Error::IoReadError:
        errorString = QString::fromUtf8("Unknown IO read error.");
        break;
    case QAppImageUpdateEnums::Error::GithubApiRateLimitReached:
        errorString = QString::fromUtf8("GitHub API rate limit reached, please try again later.");
        break;
    case QAppImageUpdateEnums::Error::ErrorResponseCode:
        errorString = QString::fromUtf8("Bad response from the server, please try again later.");
        break;
    case QAppImageUpdateEnums::Error::NoMarkerFoundInControlFile:
    case QAppImageUpdateEnums::Error::InvalidZsyncHeadersNumber:
    case QAppImageUpdateEnums::Error::InvalidZsyncMakeVersion:
    case QAppImageUpdateEnums::Error::InvalidZsyncTargetFilename:
    case QAppImageUpdateEnums::Error::InvalidZsyncMtime:
    case QAppImageUpdateEnums::Error::InvalidZsyncBlocksize:
    case QAppImageUpdateEnums::Error::InvalidTargetFileLength:
    case QAppImageUpdateEnums::Error::InvalidHashLengthLine:
    case QAppImageUpdateEnums::Error::InvalidHashLengths:
    case QAppImageUpdateEnums::Error::InvalidTargetFileUrl:
    case QAppImageUpdateEnums::Error::InvalidTargetFileSha1:
    case QAppImageUpdateEnums::Error::HashTableNotAllocated:
    case QAppImageUpdateEnums::Error::InvalidTargetFileChecksumBlocks:
    case QAppImageUpdateEnums::Error::CannotOpenTargetFileChecksumBlocks:
    case QAppImageUpdateEnums::Error::CannotConstructHashTable:
    case QAppImageUpdateEnums::Error::QbufferIoReadError:
        errorString = QString::fromUtf8("Invalid zsync meta file.");
        break;
    case QAppImageUpdateEnums::Error::ZsyncControlFileNotFound:
        errorString = QString::fromUtf8("The zsync control file was not found in the specified location.");
        break;
    case QAppImageUpdateEnums::Error::SourceFileNotFound:
        errorString = QString::fromUtf8("The current AppImage could not be found, maybe it was deleted while updating?");
        break;
    case QAppImageUpdateEnums::Error::NoPermissionToReadSourceFile:
        errorString = QString::fromUtf8("You don't have the permission to read the current AppImage.");
        break;
    case QAppImageUpdateEnums::Error::CannotOpenSourceFile:
        errorString = QString::fromUtf8("The current AppImage cannot be opened.");
        break;
    case QAppImageUpdateEnums::Error::NoPermissionToReadWriteTargetFile:
        errorString = QString::fromUtf8("You have no write or read permissions for the new version.");
        break;
    case QAppImageUpdateEnums::Error::CannotOpenTargetFile:
        errorString = QString::fromUtf8("The new version cannot be opened to write or read.");
        break;
    case QAppImageUpdateEnums::Error::TargetFileSha1HashMismatch:
        errorString = QString::fromUtf8("The newly constructed AppImage failed the integrity check, please try again.");
        break;
    default:
        errorString = QString::fromUtf8("Unknown error.");
        break;
    }
    return errorString;
}
