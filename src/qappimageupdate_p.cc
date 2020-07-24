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
*/

#include "../include/qappimageupdate_p.hpp"
#include "../include/helpers_p.hpp"


QAppImageUpdatePrivate::QAppImageUpdatePrivate(bool singleThreaded, QObject *parent)
    : QObject(parent) {
    setObjectName("QAppImageUpdatePrivate");
    if(!singleThreaded) {
        p_SharedThread.reset(new QThread);
        p_SharedThread->start();
    }


    m_SharedNetworkAccessManager.reset(new QNetworkAccessManager);
    m_UpdateInformation.reset(new AppImageUpdateInformationPrivate);
    m_DeltaWriter.reset(new ZsyncWriterPrivate);
    if(!singleThreaded) {
        m_SharedNetworkAccessManager->moveToThread(m_SharedThread.data());
        m_UpdateInformation->moveToThread(m_SharedThread.data());
        m_DeltaWriter->moveToThread(m_SharedThread.data());
    }
    m_ControlFileParser.reset(new ZsyncRemoteControlFileParserPrivate(m_SharedNetworkAccessManager.data()));
    m_BlockDownloader.reset(new ZsyncBlockRangeDownloaderPrivate(m_DeltaWriter.data(),
                            m_SharedNetworkAccessManager.data()));
    if(!singleThreaded) {
        m_ControlFileParser->moveToThread(m_SharedThread.data());
    }
    m_ControlFileParser->setObjectName("ZsyncRemoteControlFileParserPrivate");
    m_UpdateInformation->setObjectName("AppImageUpdateInformationPrivate");
    m_DeltaWriter->setObjectName("ZsyncWriterPrivate");
    
    // Set logger name
    m_UpdateInformation->setLoggerName("UpdateInformation");
    m_ControlFileParser->setLoggerName("ControlFileParser");
    m_DeltaWriter->setLoggerName("DeltaWriter");


    // Update information
    connect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::progress,
            this, &QAppImageUpdatePrivate::handleUpdateInformationProgress,
            Qt::UniqueConnection);
    connect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::error,
            this, &QAppImageUpdatePrivate::handleUpdateInformationError,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
    connect(p_UpdateInformation.data(), &AppImageUpdateInformationPrivate::logger,
            this, &QAppImageUpdatePrivate::logger,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));

    // Control file parsing
    connect(p_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::progress,
            this, &QAppImageUpdatePrivate::handleZsyncRemoteControlFileProgress,
            Qt::UniqueConnection);
    connect(p_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::error,
            this, &QAppImageUpdatePrivate::handleZsyncRemoteControlFileError,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
    connect(p_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::logger,
            this, &QAppImageUpdatePrivate::logger,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));

    // Delta Writer and Downloader
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::error,
            this, &QAppImageUpdatePrivate::handleZsyncWriterError,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::logger,
            this, &QAppImageUpdatePrivate::logger,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));

    // TODO: We need to figure out some way to avoid this
    /* Connect the recieveControlFile signal to ZsyncWriter */
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::progress,
            this, &QAppImageUpdatePrivate::progress,
            (Qt::ConnectionType)(Qt::DirectConnection | Qt::UniqueConnection));
    connect(p_ControlFileParser.data(), &ZsyncRemoteControlFileParserPrivate::zsyncInformation,
            p_DeltaWriter.data(), &ZsyncWriterPrivate::setConfiguration,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(p_DeltaWriter.data(), &ZsyncWriterPrivate::finishedConfiguring,
            p_DeltaWriter.data(), &ZsyncWriterPrivate::start,
            (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));

}

QAppImageUpdatePrivate::QAppImageUpdatePrivate(const QString &AppImagePath, bool singleThreaded, QObject *parent)
    : QAppImageUpdatePrivate(singleThreaded, parent) {
    setAppImage(AppImagePath);
    return;
}

QAppImageUpdatePrivate::QAppImageUpdatePrivate(QFile *AppImage, bool singleThreaded, QObject *parent)
    : QAppImageUpdatePrivate(singleThreaded, parent) {
    setAppImage(AppImage);
    return;
}

QAppImageUpdatePrivate::~QAppImageUpdatePrivate() {
    if(b_Started || b_Running){
	    cancel();
	    /*
	     * TODO: It might be a good idea to do this after running some tests.
	       QEventLoop loop;
	       connect(this, &QAppImageUpdatePrivate::canceled, &loop, &QEventLoop::quit); 
	       loop.exec();
	    */
    }

    if(!m_SharedThread.isNull()) {
        m_SharedThread->quit();
        m_SharedThread->wait();
    }
    return;
}

void QAppImageUpdatePrivate::setAppImage(const QString &AppImagePath) {
    if(b_Started || b_Running) {
        return;
    }

    clear();
    getMethod(m_UpdateInformation.data(), "setAppImage(const QString&)")
	    .invoke(m_UpdateInformation.data(), 
		    Qt::QueuedConnection,
                    Q_ARG(QString,AppImagePath));
    return;
}

void QAppImageUpdatePrivate::setAppImage(QFile *AppImage) {
    if(b_Started || b_Running) {
        return;
    }

    clear();
    getMethod(m_UpdateInformation.data(), "setAppImage(QFile *)")
	    .invoke(m_UpdateInformation.data(),
                    Qt::QueuedConnection,
                    Q_ARG(QFile*,AppImage));
}

void QAppImageUpdatePrivate::setShowLog(bool choice) {
    if(b_Started || b_Running) {
	    return;
    }

    getMethod(m_UpdateInformation.data(), "setShowLog(bool)")
	    .invoke(m_UpdateInformation.data(),
                    Qt::QueuedConnection, 
		    Q_ARG(bool, choice));

    getMethod(m_ControlFileParser.data(), "setShowLog(bool)")
	    .invoke(m_ControlFileParser.data(),
		    Qt::QueuedConnection, 
		    Q_ARG(bool, choice));

    getMethod(m_DeltaWriter.data(), "setShowLog(bool)")
	    .invoke(m_DeltaWriter.data(),
		    Qt::QueuedConnection, 
		    Q_ARG(bool, choice));
}

void QAppImageUpdatePrivate::setOutputDirectory(const QString &dir) {
    if(b_Started || b_Running) {
	    return;
    }

    getMethod(m_DeltaWriter.data(), "setOutputDirectory(const QString&)")
	    .invoke(m_DeltaWriter.data(),
                    Qt::QueuedConnection,
                    Q_ARG(QString, dir));
    return;
}

void QAppImageUpdatePrivate::setProxy(const QNetworkProxy &proxy) {
    if(b_Started || b_Running) {
	    return;
    }
    m_SharedNetworkAccessManager->setProxy(proxy);
    return;
}

void QAppImageUpdatePrivate::clear(void) {
    if(b_Started || b_Running) {
        return;
    }

    b_Started = b_Running = b_Finished = b_Canceled = b_CancelRequested = false;
    getMethod(m_UpdateInformation.data(), "clear(void)")
	    .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);
    getMethod(m_ControlFileParser.data(), "clear(void)")
	    .invoke(m_ControlFileParser.data(), Qt::QueuedConnection);
    return;
}



void QAppImageUpdatePrivate::start(short action) {
    if(b_Started || b_Running){
        return;
    }

    b_Started = b_Running = true;
    b_Canceled = false

    if(action = Action::GetEmbeddedInfo){
	    connect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::info,
		this, &QAppImageUpdatePrivate::redirectEmbeddedInformation,
		Qt::QueuedConnection);

	    connect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::error,
		this, &QAppImageUpdatePrivate::handleGetEmbeddedInfoError,
		Qt::QueuedConnection);

	    emit started(Action::GetEmbeddedInfo);
	    getMethod(m_UpdateInformation.data(), "getInfo(void)")
		    .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);
    
    }else if(action == Action::CheckForUpdate) {
	    connect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
		     m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)),
		     (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));
	    
	    connect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
		     m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)),
		     (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));
	    
	    connect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
		     this, SLOT(redirectUpdateCheck(QJsonObject)),
                     (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));
	    
	    connect(m_ControlFileParser.data(), SIGNAL(error(short)),
		     this, SLOT(handleCheckForUpdateError(short)),
                     (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));

	    emit started(Action::CheckForUpdate); 
    	    getMethod(m_UpdateInformation.data(), "getInfo(void)")
		    .invoke(m_UpdateInformation.data(), Qt::QueuedConnection);
    }else if(action == Action::Update) {
	    connect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
		     m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject)),
		     (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));
	    
	    connect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
		    m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void)),
		    (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));
    
	    connect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
		     this, SLOT(doStart(QJsonObject)),
                     (Qt::ConnectionType)(Qt::UniqueConnection | Qt::QueuedConnection));
	    
	    getMethod(p_UpdateInformation.data(), "getInfo(void)")
		    .invoke(p_UpdateInformation.data(), Qt::QueuedConnection);

    }else if(action == Action::UpdateWithGUI) {

    }else {
	    b_Started = b_Running = b_Canceled = false;
    }
    return;
}

void QAppImageUpdatePrivate::cancel(void) {
    if(!b_Started || !b_Running) {
	    return;
    }

    b_CancelRequested = true;

    if(n_CurrentAction == Action::Update || n_CurrentAction == Action::UpdateWithGUI) {
    	getMethod(m_DeltaWriter.data(),"cancel(void)")
		.invoke(m_DeltaWriter.data(), Qt::QueuedConnection);
    	getMethod(m_BlockDownloader.data(), "cancel(void)")
		.invoke(m_BlockDownloader.data(), Qt::QueuedConnection);
    }
    return;
}


/// * * *
/// Private Slots

void QAppImageUpdatePrivate::handleGetEmbeddedInfoError(short code) {
	b_Canceled = b_Started = b_Running = false;	
	b_Finished = false;
	b_CancelRequested = false;
	disconnect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::error,
		   this, &QAppImageUpdatePrivate::handleGetEmbeddedInfoError);
	disconnect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::info,
		   this, &QAppImageUpdatePrivate::redirectEmbeddedInformation);   
	emit error(code, QAppImageUpdatePrivate::Action::GetEmbeddedInfo); 
}

void QAppImageUpdatePrivate::redirectEmbeddedInformation(QJsonObject info) {
	b_Canceled = b_Started = b_Running = false;	
	b_Finished = true;
	disconnect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::error,
		   this, &QAppImageUpdatePrivate::handleGetEmbeddedInfoError);
	disconnect(m_UpdateInformation.data(),  &AppImageUpdateInformationPrivate::info,
		   this, &QAppImageUpdatePrivate::redirectEmbeddedInformation);   
	

	if(b_CancelRequested) {
		b_CancelRequested = false;
		b_Canceled = true;
		emit canceled(Action::GetEmbeddedInfo);
		return;
	}
	emit finished(info, Action::GetEmbeddedInfo); 
}


void QAppImageUpdatePrivate::handleCheckForUpdateError(short code) {
	b_Canceled = b_Started = b_Running = false;	
	b_Finished = false;
	b_CancelRequested = false;

	disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
	       m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject))); 
	disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
	       m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void))); 
	disconnect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
	       this, SLOT(redirectUpdateCheck(QJsonObject)));
	disconnect(m_ControlFileParser.data(), SIGNAL(error(short)),
	       this, SLOT(handleCheckForUpdateError(short)))
  
	emit error(code, QAppImageUpdatePrivate::Action::CheckForUpdate); 
}

void QAppImageUpdatePrivate::redirectUpdateCheck(QJsonObject info) {
    disconnect(m_UpdateInformation.data(), SIGNAL(info(QJsonObject)),
	       m_ControlFileParser.data(), SLOT(setControlFileUrl(QJsonObject))); 
    disconnect(m_ControlFileParser.data(), SIGNAL(receiveControlFile(void)),
	       m_ControlFileParser.data(), SLOT(getUpdateCheckInformation(void))); 
    disconnect(m_ControlFileParser.data(), SIGNAL(updateCheckInformation(QJsonObject)),
	       this, SLOT(redirectUpdateCheck(QJsonObject)));
    disconnect(m_ControlFileParser.data(), SIGNAL(error(short)),
	       this, SLOT(handleCheckForUpdateError(short)))
  
    // Can this happen without an error? 
    if(info.isEmpty()) {
        return;
    }

    auto embeddedUpdateInformation = info["EmbededUpdateInformation"].toObject();
    auto oldVersionInformation = embeddedUpdateInformation["FileInformation"].toObject();

    QString remoteTargetFileSHA1Hash = info["RemoteTargetFileSHA1Hash"].toString(), 
            localAppImageSHA1Hash = oldVersionInformation["AppImageSHA1Hash"].toString(),
            localAppImagePath = oldVersionInformation["AppImageFilePath"].toString();

    QJsonObject updateinfo {
	{ "UpdateAvailable", localAppImageSHA1Hash != remoteTargetFileSHA1Hash},
	{ "AbsolutePath", localAppImagePath},
	{ "Sha1Hash",  localAppImageSHA1Hash },
	{ "RemoteSha1Hash", remoteTargetFileSHA1Hash},
	{ "ReleaseNotes", }
    };
    
    b_Started = b_Running = false;
    b_Finished = true;
    b_Canceled = false;
    

    if(b_CancelRequested) {
		b_CancelRequested = false;
		b_Canceled = true;
		emit canceled(Action::CheckForUpdate);
		return;
    }
    emit finished(updateinfo, QAppImageUpdatePrivate::CheckForUpdate);
}
