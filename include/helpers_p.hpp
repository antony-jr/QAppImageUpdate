#ifndef HELPERS_PRIVATE_HPP_INCLUDED
#define HELPERS_PRIVATE_HPP_INCLUDED
#include <QObject>
#include <QMetaMethod>
#include <QMetaObject>
#include <QNetworkReply>

QMetaMethod getMethod(QObject*,const char*);
short translateQNetworkReplyError(QNetworkReply::NetworkError);

#endif
