/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018-2019, Antony jr
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
 * @filename    : appimagedeltarevisioner.cc
 * @description : This where the delta revisioner is implemented.
 * Delta Revisioner is the public API to manage the entire revision
 * of a AppImage. From retrival of the embeded information from the
 * AppImage to the retrival of required remaining block ranges.
 * This is a controller to all the internal mechanisms , Handled neatly
 * via Qt's signals and slots.
 *
*/
#include "../include/appimagedeltarevisioner_p.hpp"
#include "../include/appimagedeltarevisioner.hpp"
#include "../include/helpers_p.hpp"

using namespace AppImageUpdaterBridge;

AppImageDeltaRevisioner::AppImageDeltaRevisioner(bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    p_DeltaRevisioner = new AppImageDeltaRevisionerPrivate(singleThreaded, this);
    connectSignals();
    return;
}

AppImageDeltaRevisioner::AppImageDeltaRevisioner(const QString &AppImagePath, bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    p_DeltaRevisioner = new AppImageDeltaRevisionerPrivate(AppImagePath, singleThreaded, this);
    connectSignals();
    return;
}

AppImageDeltaRevisioner::AppImageDeltaRevisioner(QFile *AppImage, bool singleThreaded, QObject *parent)
    : QObject(parent)
{
    p_DeltaRevisioner = new AppImageDeltaRevisionerPrivate(AppImage, singleThreaded, this);
    connectSignals();
    return;
}

AppImageDeltaRevisioner::~AppImageDeltaRevisioner()
{
    p_DeltaRevisioner->deleteLater();
    return;
}

void AppImageDeltaRevisioner::start(void)
{
    getMethod(p_DeltaRevisioner, "start(void)").invoke(p_DeltaRevisioner, Qt::QueuedConnection);
    return;
}

void AppImageDeltaRevisioner::cancel(void)
{
    getMethod(p_DeltaRevisioner, "cancel(void)").invoke(p_DeltaRevisioner, Qt::QueuedConnection);
    return;
}

void AppImageDeltaRevisioner::setAppImage(const QString &AppImagePath)
{
    getMethod(p_DeltaRevisioner, "setAppImage(const QString&)")
    .invoke(p_DeltaRevisioner, Qt::QueuedConnection, Q_ARG(QString, AppImagePath));
    return;
}

void AppImageDeltaRevisioner::setAppImage(QFile *AppImage)
{
    getMethod(p_DeltaRevisioner, "setAppImage(QFile*)")
    .invoke(p_DeltaRevisioner, Qt::QueuedConnection, Q_ARG(QFile*, AppImage));
    return;
}

void AppImageDeltaRevisioner::setShowLog(bool choice)
{
    getMethod(p_DeltaRevisioner, "setShowLog(bool)")
    .invoke(p_DeltaRevisioner, Qt::QueuedConnection, Q_ARG(bool, choice));
    return;
}

void AppImageDeltaRevisioner::setOutputDirectory(const QString &dir)
{
    getMethod(p_DeltaRevisioner, "setOutputDirectory(const QString&)")
    .invoke(p_DeltaRevisioner, Qt::QueuedConnection, Q_ARG(QString, dir));
    return;
}

void AppImageDeltaRevisioner::getAppImageEmbededInformation(void)
{
    getMethod(p_DeltaRevisioner, "getAppImageEmbededInformation(void)").invoke(p_DeltaRevisioner, Qt::QueuedConnection);
    return;
}

void AppImageDeltaRevisioner::clear(void)
{
    getMethod(p_DeltaRevisioner, "clear(void)").invoke(p_DeltaRevisioner, Qt::QueuedConnection);
    return;
}

void AppImageDeltaRevisioner::checkForUpdate(void)
{
    getMethod(p_DeltaRevisioner, "checkForUpdate(void)").invoke(p_DeltaRevisioner, Qt::QueuedConnection);
    return;
}

void AppImageDeltaRevisioner::connectSignals()
{
    connect(p_DeltaRevisioner, &AppImageDeltaRevisionerPrivate::started,
            this, &AppImageDeltaRevisioner::started, Qt::DirectConnection);
    connect(p_DeltaRevisioner, &AppImageDeltaRevisionerPrivate::canceled,
            this, &AppImageDeltaRevisioner::canceled, Qt::DirectConnection);
    connect(p_DeltaRevisioner, &AppImageDeltaRevisionerPrivate::finished,
            this, &AppImageDeltaRevisioner::finished, Qt::DirectConnection);
    connect(p_DeltaRevisioner, &AppImageDeltaRevisionerPrivate::embededInformation,
            this, &AppImageDeltaRevisioner::embededInformation, Qt::DirectConnection);
    connect(p_DeltaRevisioner, &AppImageDeltaRevisionerPrivate::updateAvailable,
            this, &AppImageDeltaRevisioner::updateAvailable, Qt::DirectConnection);
    connect(p_DeltaRevisioner, &AppImageDeltaRevisionerPrivate::statusChanged,
            this, &AppImageDeltaRevisioner::statusChanged, Qt::DirectConnection);
    connect(p_DeltaRevisioner, &AppImageDeltaRevisionerPrivate::error,
            this, &AppImageDeltaRevisioner::error, Qt::DirectConnection);
    connect(p_DeltaRevisioner, &AppImageDeltaRevisionerPrivate::progress,
            this, &AppImageDeltaRevisioner::progress, Qt::DirectConnection);
    connect(p_DeltaRevisioner, &AppImageDeltaRevisionerPrivate::logger,
            this, &AppImageDeltaRevisioner::logger, Qt::DirectConnection);
}
