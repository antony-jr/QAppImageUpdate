/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2017, Antony jr
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
 * ------------------------------------------------------------------------------
 *  @filename           : AIUpdaterBridge.cpp
 *  @description        : This is the source file which fills in the code
 *  			  with reference to the prototype given in the 
 *  			  AIUpdaterBridge.hpp.
 *  			  A simple bridge to AppImage's Updating Mechanism 
 *  			  writen in C++ using Qt5. This small library helps 
 *  			  you to create awesome AutoUpdater in Qt5 for AppImages.
 * ------------------------------------------------------------------------------
*/
#include <AIUpdaterBridge.hpp>

AIUpdaterBridge::AIUpdaterBridge(const QString& appImage)
	: QObject(NULL)
{
	setAppImageUpdateInformation(appImage);
	return;
}

AIUpdaterBridge::AIUpdaterBridge(const QJsonObject& config)
	: QObject(NULL)
{
	setAppImageUpdateInformation(config);
	return;
}

void AIUpdaterBridge::doDebug(bool ch)
{
	debug = ch;
	return;
}

// Public Slots

void AIUpdaterBridge::setAppImageUpdateInformation(const QString& appImage)
{
	connect(&AppImageInformer , SIGNAL(updateInformation(const QString& , const QJsonObject&)),
                    this , SLOT(handleAppImageUpdateInformation(const QString& , const QJsonObject&)));
        connect(&AppImageInformer , SIGNAL(error(const QString& , short)),
                    this , SLOT(handleAppImageUpdateError(const QString& , short)));
	AppImageInformer.setAppImage(appImage);
	AppImageInformer.doDebug(debug);
	AppImageInformer.start();
	return;
}

void AIUpdaterBridge::setAppImageUpdateInformation(const QJsonObject& config)
{
	return;
}

// Private Slots

void AIUpdaterBridge::handleAppImageUpdateInformation(const QString& appImage , const QJsonObject& config)
{
	if(config["transport"].toString() == "zsync"){
		this->zsyncURL = config["url"].toUrl();
		if(debug){
			qDebug() << "AIUpdaterBridge:: zsyncURL ::" << zsyncURL;
		}
	}else if(config["transport"].toString() == "gh-releases-zsync"){
	// handle github releases zsync.
	QUrl releaseLink;
	releaseLink = QUrl("https://api.github.com/repos/" + config["username"].toString() + 
			   "/"  + config["repo"].toString() + "/releases/");
	if(config["tag"].toString().lower() == "latest"){
		releaseLink = QUrl(releaseLink.toString() + config["tag"].toString());
	}else{
		releaseLink = QUrl(releaseLink.toString() + "tags/" + config["tag"].toString());
	}
	if(debug){
		qDebug() << "AIUpdaterBridge:: github release link ::" << releaseLink;
	}

	}else{
	// if its not github releases zsync or generic zsync
	// then it must be bintray-zsync
	// Note: Since QAIUpdateInformation can handle errors
	// we don't really have to check for integrity now.
	
	// handle bintray zsync.
	}
	// There should be no errors at this stage.
	return;
}

void AIUpdaterBridge::handleAppImageUpdateInformationError(const QString& appImage , short errorCode)
{
	emit error(appImage , UNABLE_TO_GET_APPIMAGE_INFORMATION);
	return;
}


