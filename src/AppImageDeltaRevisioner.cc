/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018, Antony jr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @filename    : AppImageDeltaRevisioner.cc
 * @description : This where the delta revisioner is implemented.
 * Delta Revisioner is the public API to manage the entire revision
 * of a AppImage. From retrival of the embeded information from the
 * AppImage to the retrival of required remaining block ranges.
 * This is a controller to all the internal mechanisms , Handled neatly
 * via Qt's signals and slots.
 *
*/
#include <AppImageDeltaRevisioner_p.hpp>
#include <AppImageDeltaRevisioner.hpp>

using namespace AppImageUpdaterBridge;

#define CONSTRUCT(uniqueCode)  uniqueCode   \
                               connect(_pDeltaRevisioner , &AppImageDeltaRevisionerPrivate::started , \
                                        this , &AppImageDeltaRevisioner::started , Qt::DirectConnection); \
                               connect(_pDeltaRevisioner , &AppImageDeltaRevisionerPrivate::canceled , \
                                        this , &AppImageDeltaRevisioner::canceled , Qt::DirectConnection); \
                               connect(_pDeltaRevisioner , &AppImageDeltaRevisionerPrivate::finished , \
                                        this , &AppImageDeltaRevisioner::finished , Qt::DirectConnection); \
                               connect(_pDeltaRevisioner , &AppImageDeltaRevisionerPrivate::embededInformation , \
                                        this , &AppImageDeltaRevisioner::embededInformation , Qt::DirectConnection); \
                               connect(_pDeltaRevisioner , &AppImageDeltaRevisionerPrivate::updateAvailable , \
                                        this , &AppImageDeltaRevisioner::updateAvailable , Qt::DirectConnection); \
                               connect(_pDeltaRevisioner , &AppImageDeltaRevisionerPrivate::statusChanged , \
                                        this , &AppImageDeltaRevisioner::statusChanged , Qt::DirectConnection); \
                               connect(_pDeltaRevisioner , &AppImageDeltaRevisionerPrivate::error , \
                                        this , &AppImageDeltaRevisioner::error , Qt::DirectConnection); \
                               connect(_pDeltaRevisioner , &AppImageDeltaRevisionerPrivate::progress , \
                                        this , &AppImageDeltaRevisioner::progress , Qt::DirectConnection); \
                               connect(_pDeltaRevisioner , &AppImageDeltaRevisionerPrivate::logger , \
                                        this , &AppImageDeltaRevisioner::logger , Qt::DirectConnection);

#define TO_STRING(x) #x
                               
#define QUEUED_CALL_FUNC(o , f)  {  \
                                    auto metaObject = o->metaObject(); \
                                    metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(TO_STRING(f)))) \
                                                .invoke(o , Qt::QueuedConnection); \
                                 }

#define QUEUED_CALL_FUNC_QSTRING_ARG(o , f , s)   {  \
                                                    auto metaObject = o->metaObject(); \
                                                    metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(TO_STRING(f)))) \
                                                                .invoke(o , Qt::QueuedConnection , Q_ARG(QString , s)); \
                                                  }

#define QUEUED_CALL_FUNC_QFILE_ARG(o , f , qf)  {  \
                                                    auto metaObject = o->metaObject(); \
                                                    metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(TO_STRING(f)))) \
                                                                .invoke(o , Qt::QueuedConnection , Q_ARG(QFile* , qf)); \
                                                 }
                                                 
#define QUEUED_CALL_FUNC_BOOL_ARG(o , f , b)   {  \
                                                    auto metaObject = o->metaObject(); \
                                                    metaObject->method(metaObject->indexOfMethod(QMetaObject::normalizedSignature(TO_STRING(f)))) \
                                                                .invoke(o , Qt::QueuedConnection , Q_ARG(bool , b)); \
                                               }
                                                  
AppImageDeltaRevisioner::AppImageDeltaRevisioner(bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    CONSTRUCT(
        _pDeltaRevisioner = new AppImageDeltaRevisionerPrivate(singleThreaded , this);
    )
    return;
}

AppImageDeltaRevisioner::AppImageDeltaRevisioner(const QString &AppImagePath, bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    CONSTRUCT(
        _pDeltaRevisioner = new AppImageDeltaRevisionerPrivate(AppImagePath , singleThreaded , this);
    )
    return;
}

AppImageDeltaRevisioner::AppImageDeltaRevisioner(QFile *AppImage, bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    CONSTRUCT(
       _pDeltaRevisioner = new AppImageDeltaRevisionerPrivate(AppImage , singleThreaded , this);
    )
    return;
}

AppImageDeltaRevisioner::~AppImageDeltaRevisioner()
{
    _pDeltaRevisioner->deleteLater();
    return;
}

void AppImageDeltaRevisioner::start(void)
{
    QUEUED_CALL_FUNC(_pDeltaRevisioner , start(void));
    return;
}

void AppImageDeltaRevisioner::cancel(void)
{
    QUEUED_CALL_FUNC(_pDeltaRevisioner , cancel(void));
    return;
}

void AppImageDeltaRevisioner::setAppImage(const QString &AppImagePath)
{
    QUEUED_CALL_FUNC_QSTRING_ARG(_pDeltaRevisioner , setAppImage(const QString&) , AppImagePath);
    return;
}

void AppImageDeltaRevisioner::setAppImage(QFile *AppImage)
{
    QUEUED_CALL_FUNC_QFILE_ARG(_pDeltaRevisioner , setAppImage(QFile*) , AppImage);
    return;
}

void AppImageDeltaRevisioner::setShowLog(bool choice)
{
    QUEUED_CALL_FUNC_BOOL_ARG(_pDeltaRevisioner , setShowLog(bool) , choice);
    return;
}

void AppImageDeltaRevisioner::setOutputDirectory(const QString &dir)
{
    QUEUED_CALL_FUNC_QSTRING_ARG(_pDeltaRevisioner , setOutputDirectory(const QString&) , dir);
    return;
}

void AppImageDeltaRevisioner::getAppImageEmbededInformation(void)
{
    QUEUED_CALL_FUNC(_pDeltaRevisioner , getAppImageEmbededInformation(void));
    return;
}

void AppImageDeltaRevisioner::clear(void)
{
    QUEUED_CALL_FUNC(_pDeltaRevisioner , clear(void));
    return;
}

void AppImageDeltaRevisioner::checkForUpdate(void)
{
    QUEUED_CALL_FUNC(_pDeltaRevisioner , checkForUpdate(void));
    return;
}

QNetworkReply::NetworkError AppImageDeltaRevisioner::getNetworkError(void)
{
    return _pDeltaRevisioner->getNetworkError();
}

QString AppImageDeltaRevisioner::errorCodeToString(short code)
{
    return AppImageDeltaRevisionerPrivate::errorCodeToString(code);
}

QString AppImageDeltaRevisioner::statusCodeToString(short code)
{
    return AppImageDeltaRevisionerPrivate::statusCodeToString(code);
}
