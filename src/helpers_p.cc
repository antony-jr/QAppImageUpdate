#include "qappimageupdateenums.hpp"
#include "helpers_p.hpp"

QMetaMethod getMethod(QObject *object, const char *function) {
    auto metaObject = object->metaObject();
    return metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(function)));
}

short translateQNetworkReplyError(QNetworkReply::NetworkError errorCode) {
    short e = 0;
    if(errorCode > 0 && errorCode < 101) {
        e = QAppImageUpdateEnums::Error::ConnectionRefusedError + ((short)errorCode - 1);
    } else if(errorCode == QNetworkReply::UnknownNetworkError) {
        e = QAppImageUpdateEnums::Error::UnknownNetworkError;
    } else if(errorCode == QNetworkReply::UnknownProxyError) {
        e = QAppImageUpdateEnums::Error::UnknownProxyError;
    } else if(errorCode >= 101 && errorCode < 201) {
        e = QAppImageUpdateEnums::Error::ProxyConnectionRefusedError + ((short)errorCode - 101);
    } else if(errorCode == QNetworkReply::ProtocolUnknownError) {
        e = QAppImageUpdateEnums::Error::ProtocolUnknownError;
    } else if(errorCode == QNetworkReply::ProtocolInvalidOperationError) {
        e = QAppImageUpdateEnums::Error::ProtocolInvalidOperationError;
    } else if(errorCode == QNetworkReply::UnknownContentError) {
        e = QAppImageUpdateEnums::Error::UnknownContentError;
    } else if(errorCode == QNetworkReply::ProtocolFailure) {
        e = QAppImageUpdateEnums::Error::ProtocolFailure;
    } else if(errorCode >= 201 && errorCode < 401) {
        e = QAppImageUpdateEnums::Error::ContentAccessDenied + ((short)errorCode - 201);
    } else if(errorCode >= 401 && errorCode <= 403) {
        e = QAppImageUpdateEnums::Error::InternalServerError + ((short)errorCode - 401);
    } else {
        e = QAppImageUpdateEnums::Error::UnknownServerError;
    }
    return e;
}
