#include "../include/appimageupdaterbridge_enums.hpp"
#include "../include/helpers_p.hpp"

using namespace AppImageUpdaterBridge;

QMetaMethod getMethod(QObject *object, const char *function) {
    auto metaObject = object->metaObject();
    return metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(function)));
}

short translateQNetworkReplyError(QNetworkReply::NetworkError errorCode) {
    short e = 0;
    if(errorCode > 0 && errorCode < 101) {
        e = ConnectionRefusedError + ((short)errorCode - 1);
    } else if(errorCode == QNetworkReply::UnknownNetworkError) {
        e = UnknownNetworkError;
    } else if(errorCode == QNetworkReply::UnknownProxyError) {
        e = UnknownProxyError;
    } else if(errorCode >= 101 && errorCode < 201) {
        e = ProxyConnectionRefusedError + ((short)errorCode - 101);
    } else if(errorCode == QNetworkReply::ProtocolUnknownError) {
        e = ProtocolUnknownError;
    } else if(errorCode == QNetworkReply::ProtocolInvalidOperationError) {
        e = ProtocolInvalidOperationError;
    } else if(errorCode == QNetworkReply::UnknownContentError) {
        e = UnknownContentError;
    } else if(errorCode == QNetworkReply::ProtocolFailure) {
        e = ProtocolFailure;
    } else if(errorCode >= 201 && errorCode < 401) {
        e = ContentAccessDenied + ((short)errorCode - 201);
    } else if(errorCode >= 401 && errorCode <= 403) {
        e = InternalServerError + ((short)errorCode - 401);
    } else {
        e = UnknownServerError;
    }
    return e;
}
