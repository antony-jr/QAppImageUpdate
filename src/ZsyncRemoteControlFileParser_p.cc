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
 * @filename    : ZsyncRemoteControlFileParser_p.cc
 * @description : This is where the zsync control file parser in written.
 * The Zsync Control File Parser is responsible to parse the remote 
 * zsync control file and produce us with a more sensible data to work
 * with , the parser also creates job information for ZsyncCoreJobPrivate class.
*/
#include <ZsyncRemoteControlFileParser_p.hpp>

using namespace AppImageUpdaterBridge;

/*
 * Prints to the log.
 * LOGS,LOGE  -> Prints normal log messages.
 * INFO_START,INFO_END -> Prints info messages to log.
 * WARNING_START,WARNING_END -> Prints warning messages to log.
 * FATAL_START,FATAL_END -> Prints fatal messages to log.
 *
 * Example:
 *      LOGS "This is a log message." LOGE
 *
 *
*/
#ifndef LOGGING_DISABLED
#define LOGS *(_pLogger.data()) LOGR
#define LOGR <<
#define LOGE ; \
             emit(logger(_sLogBuffer , _sAppImagePath)); \
             _sLogBuffer.clear();
#else
#define LOGS (void)
#define LOGR ;(void)
#define LOGE ;
#endif // LOGGING_DISABLED
#define INFO_START LOGS "   INFO: "
#define INFO_END LOGE

#define WARNING_START LOGS "WARNING: "
#define WARNING_END LOGE

#define FATAL_START LOGS "  FATAL: "
#define FATAL_END LOGE


/*
 * ZsyncRemoteControlFileParserPrivate is the private class to handle all things
 * related to Zsync Control File. This class must be used privately.
 * This class caches the Zsync Control File in a QBuffer , So when its needed ,We
 * can quickly seek and start feeding in the checksum blocks.
 * This class automatically parses the zsync headers on the way to buffer the
 * control file.
 *
 * Example:
 *
 *      QNetworkAccessManager qnam;
 *      ZsyncRemoteControlFileParserPrivate rcfp(&qnam);
 *
 * Note:
 *    This class will not use the QNetworkAccessManager as the parent ,
 *    Therefore the developer as to deallocate if he allocates it.
 *    That is , Qt parent to child deallocation does not work in this class
 *    for a reason(i.e , This class is intended to run in a QThread.).
 *
*/
ZsyncRemoteControlFileParserPrivate::ZsyncRemoteControlFileParserPrivate(QNetworkAccessManager *networkManager)
    : QObject()
{
    emit statusChanged(INITIALIZING);
#ifndef LOGGING_DISABLED
    _pLogger = QSharedPointer<QDebug>(new QDebug(&_sLogBuffer));
#endif // LOGGING_DISABLED
    _pNManager = networkManager;
    connect(this, SIGNAL(error(short)), this, SLOT(handleErrorSignal(short)));
    emit statusChanged(IDLE);
    return;
}

ZsyncRemoteControlFileParserPrivate::~ZsyncRemoteControlFileParserPrivate()
{
    emit statusChanged(IDLE);
#ifndef LOGGING_DISABLED
    _pLogger.clear();
#endif // LOGGING_DISABLED
    _pControlFile.clear();
    return;
}

#ifndef LOGGING_DISABLED
/* Sets the name of the logger. */
void ZsyncRemoteControlFileParserPrivate::setLoggerName(const QString &name)
{
	_sLoggerName = QString(name);
	return;
}

/* This public method safely turns on and off the internal logger. */
void ZsyncRemoteControlFileParserPrivate::setShowLog(bool choose)
{
    if(choose) {
        disconnect(this, SIGNAL(logger(QString , QString)),
                   this, SLOT(handleLogMessage(QString , QString)));
        connect(this, SIGNAL(logger(QString , QString)),
                this, SLOT(handleLogMessage(QString , QString)));
	INFO_START LOGR " setShowLog : started logging." INFO_END;
    } else {
	INFO_START LOGR " setShowLog : stopping logging." INFO_END;
        disconnect(this, SIGNAL(logger(QString , QString)),
                   this, SLOT(handleLogMessage(QString , QString)));
    }
    return;
}
#endif // LOGGING_DISABLED

/* This public method safely sets the zsync control file url. */
void ZsyncRemoteControlFileParserPrivate::setControlFileUrl(const QUrl &controlFileUrl)
{
    INFO_START LOGR " setControlFileUrl : using " LOGR controlFileUrl LOGR " as zsync control file." INFO_END;
    _uControlFileUrl = controlFileUrl;
    return;
}

/*
 * This public overloaded method will use the embeded information in AppImages to get
 * and parse a remote zsync control file.
 * This also automatically calls getControlFile() slot.
*/
void ZsyncRemoteControlFileParserPrivate::setControlFileUrl(QJsonObject information)
{
     if(information["IsEmpty"].toBool()){
	     return;
     }
     emit statusChanged(PARSING_APPIMAGE_EMBEDED_UPDATE_INFORMATION);
     /*
      * Check if we are given the same information 
      * consecutively , If so then return what we know.
     */
     if(!_jUpdateInformation.isEmpty()){
	     if(_jUpdateInformation == information){
		     emit statusChanged(IDLE);
		     emit receiveControlFile();
		     return;
	     }
     }else{
	     {
	     _jUpdateInformation = information;
	     auto fileInfo = information["FileInformation"].toObject();
	     _sAppImagePath = fileInfo["AppImageFilePath"].toString();
	     }     
     }

     information = information["UpdateInformation"].toObject();
     if(information["transport"].toString() == "zsync" ) {
	INFO_START " setControlFileUrl : using direct zsync transport." INFO_END;
        setControlFileUrl(QUrl(information["zsyncUrl"].toString()));
    	getControlFile();
     } else if(information["transport"].toString() == "gh-releases-zsync") {
	INFO_START " setControlFileUrl : using github releases zsync transport." INFO_END;
        QUrl apiLink = QUrl("https://api.github.com/repos/" + information["username"].toString() +
                           "/"  + information["repo"].toString() + "/releases/");
        if(information["tag"].toString() == "latest") {
            apiLink = QUrl(apiLink.toString() + information["tag"].toString());
        } else {
            apiLink = QUrl(apiLink.toString() + "tags/" + information["tag"].toString());
        }

	_sZsyncFileName = information["filename"].toString();

	QNetworkRequest request;
	request.setUrl(apiLink);
    	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
   
        INFO_START " setControlFileUrl : github api request(" LOGR apiLink LOGR ")." INFO_END;	

	emit statusChanged(REQUESTING_GITHUB_API);
	auto reply = _pNManager->get(request);

    	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    	connect(reply, SIGNAL(finished(void)), this, SLOT(handleGithubAPIResponse(void))); 
     } else {
        /*
	 * if its not github releases zsync or generic zsync
	 * then it must be bintray-zsync.
	 * Note: Since AppImageUpdateInformation can handle errors , Thus
	 * we don't really have to check for integrity now.
	*/
	INFO_START " setControlFileUrl : using bintray zsync transport." INFO_END;
 	QUrl latestLink;
        latestLink = QUrl("https://bintray.com/" + information["username"].toString() +
                          "/" + information["repo"].toString() + "/" + information["packageName"].toString() + "/_latestVersion");
	
	_sZsyncFileName = information["filename"].toString();
    
	QNetworkRequest request;
	request.setUrl(latestLink);
    	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    
        emit statusChanged(REQUESTING_BINTRAY);	
	QNetworkReply *reply = _pNManager->head(request);

    	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    	connect(reply, &QNetworkReply::redirected , this, &ZsyncRemoteControlFileParserPrivate::handleBintrayRedirection);
     }
    return;
}

/* clears all internal cache in the class. */
void ZsyncRemoteControlFileParserPrivate::clear(void)
{
    INFO_START LOGR " clear : flushed everything." INFO_END;
    _sZsyncMakeVersion.clear();
    _sTargetFileName.clear();
    _sTargetFileSHA1.clear();
    _sZsyncFileName.clear();
#ifndef LOGGING_DISABLED
    _sLogBuffer.clear();
#endif // LOGGING_DISABLED
    _pMTime = QDateTime();
    _nTargetFileBlockSize = _nTargetFileLength = _nTargetFileBlocks = _nWeakCheckSumBytes = 0;
    _nStrongCheckSumBytes = _nConsecutiveMatchNeeded = _nCheckSumBlocksOffset = 0;
    _uTargetFileUrl.clear();
    _uControlFileUrl.clear();
    _pControlFile.clear();
    _pControlFile = nullptr;
    return;
}

/* Starts an async request to the given zsync control file. */
void ZsyncRemoteControlFileParserPrivate::getControlFile(void)
{
    if(_uControlFileUrl.isEmpty() || !_uControlFileUrl.isValid()) {
        WARNING_START LOGR " getControlFile : no zsync control file url(" LOGR _uControlFileUrl LOGR ") is given or valid." WARNING_END;
        return;
    }

    INFO_START LOGR " getControlFile : sending get request to " LOGR _uControlFileUrl LOGR "." INFO_END;

    QNetworkRequest request;
    request.setUrl(_uControlFileUrl);
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    emit statusChanged(REQUESTING_ZSYNC_CONTROL_FILE);	
    auto reply = _pNManager->get(request); 

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished(void)), this, SLOT(handleControlFile(void)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(handleDownloadProgress(qint64, qint64)));
    return;
}

/* Replies with a signal which has a QJsonObject which can be helpful
 * to check if the given AppImage is a different version from the 
 * remote version.
 * Useless if the parser is not working with AppImageUpdateInformationPrivate.
*/
void ZsyncRemoteControlFileParserPrivate::getUpdateCheckInformation(void)
{
	QJsonObject result {
		{ "EmbededUpdateInformation" , _jUpdateInformation},
		{ "RemoteTargetFileSHA1Hash" , _sTargetFileSHA1 }
	};

	emit updateCheckInformation(result);
	return;
}

/*
 * Creates ZsyncCoreJob Information , signals a list where the number
 * of items in the list is equal to the number of cores in the current computer
 * , The list consist of ZsyncCoreJob Information which can be used to start
 * the ZsyncCoreJob concurrently across all cores.
 * The jobs are configured so that the work load is balanced across all cores
 * approximately.
*/
void ZsyncRemoteControlFileParserPrivate::getZsyncInformation(void)
{
   QVector<ZsyncCoreJobPrivate::Information> result;
    if(!_pControlFile ||
       !_pControlFile->isOpen() ||
       _pControlFile->size() - _nCheckSumBlocksOffset < (_nWeakCheckSumBytes + _nStrongCheckSumBytes) ||
       !_nCheckSumBlocksOffset) {
	   emit error(IO_READ_ERROR);
	   return;
    }

    auto buffer = new QBuffer;
    QString SeedFilePath = (_jUpdateInformation["FileInformation"].toObject())["AppImageFilePath"].toString();
    buffer->open(QIODevice::ReadWrite);
    _pControlFile->seek(_nCheckSumBlocksOffset); /* Seek to the offset of the checksum block. */
    
    while(!_pControlFile->atEnd()){
	    buffer->write(_pControlFile->read(_nWeakCheckSumBytes + _nStrongCheckSumBytes));
    }

    /*
     * This is a very simple algorithm to partition the work load approximately
     * equal across all cores.
     *
     * Let 'nb' be the number of blocks in the target file.
     * Let 'nt' be the number of ideal threads in the current computer.
     *
     * Number of Blocks for First Thread = [nb - (nb mod nt)] / nt + (nb mod nt) ; nb mod nt > 0
     * 					 = nb / nt ; nb mod nt = 0
     *
     * Number of Blocks for Other Threads = Number Of Blocks for First Thread - (nb mod nt) ; nb mod nt > 0
     * 					  = nb / nt ; nb mod nt = 0
     *
     * If the ideal thread is one then 'Number of Blocks for Other Threads' will always return 0.
    */
    int firstThreadBlocksToDo = 0,
	otherThreadsBlocksToDo = 0;
    auto mod = (int)_nTargetFileBlocks % QThread::idealThreadCount();
    if(mod > 0){
	firstThreadBlocksToDo = (int)(((int)_nTargetFileBlocks - mod) / QThread::idealThreadCount()) + mod;
	otherThreadsBlocksToDo = firstThreadBlocksToDo - mod;
    }else{ // mod == 0
	firstThreadBlocksToDo = (int)((int)_nTargetFileBlocks / QThread::idealThreadCount());
	otherThreadsBlocksToDo = firstThreadBlocksToDo;
    }

    buffer->seek(0);

    QBuffer *partition = nullptr; 
    partition = new QBuffer;
    partition->open(QIODevice::WriteOnly);
    partition->write(buffer->read(firstThreadBlocksToDo * (_nWeakCheckSumBytes + _nStrongCheckSumBytes))); 
    partition->close();
    {
    ZsyncCoreJobPrivate::Information info(_nTargetFileBlockSize , 0 , firstThreadBlocksToDo ,
		    			  _nWeakCheckSumBytes , _nStrongCheckSumBytes , _nConsecutiveMatchNeeded , 
					  partition , nullptr , SeedFilePath);
    result.append(info);
    }
   

    if(otherThreadsBlocksToDo && QThread::idealThreadCount() > 1){
	int threadCount = 2;
	while(threadCount <= QThread::idealThreadCount()){
	auto fromId = firstThreadBlocksToDo * (threadCount - 1);
	partition = new QBuffer;
	partition->open(QIODevice::WriteOnly);
    	partition->write(buffer->read(otherThreadsBlocksToDo * (_nWeakCheckSumBytes + _nStrongCheckSumBytes)));
	partition->close();
	{
	ZsyncCoreJobPrivate::Information info(_nTargetFileBlockSize, fromId, otherThreadsBlocksToDo ,
					      _nWeakCheckSumBytes , _nStrongCheckSumBytes ,
					      _nConsecutiveMatchNeeded , partition , nullptr , SeedFilePath);
	result.append(info);
	}
	++threadCount;
	QCoreApplication::processEvents();
	}
   }

   buffer->close();
   delete buffer;

   emit zsyncInformation(_nTargetFileBlockSize, _nTargetFileBlocks, _nWeakCheckSumBytes , _nStrongCheckSumBytes ,
                          _nConsecutiveMatchNeeded , _nTargetFileLength , SeedFilePath , _sTargetFileName , 
			  _sTargetFileSHA1 , result);
   return;
}

/*
 * Returns the number blocks in the target file.
*/
size_t ZsyncRemoteControlFileParserPrivate::getTargetFileBlocksCount(void)
{
    return _nTargetFileBlocks;
}

/*
 * Returns the url of the zsync control file.
*/
QUrl ZsyncRemoteControlFileParserPrivate::getControlFileUrl(void)
{
    return _uControlFileUrl;
}

/*
 * Returns the Zsync make version that is used to create the parsed
 * zsync control file.
 * This returns an empty string if a process is busy or the control
 * file was never parsed.
*/
QString ZsyncRemoteControlFileParserPrivate::getZsyncMakeVersion(void)
{
    return _sZsyncMakeVersion;
}

/*
 * Returns the target file's name.
 * This returns an emtpy string if a process is busy or the control
 * file was never parsed.
*/
QString ZsyncRemoteControlFileParserPrivate::getTargetFileName(void)
{
    return _sTargetFileName;
}

/*
 * Returns the target file's url.
 * Returns an empty url incase a process is busy or the control
 * file was never parsed.
*/
QUrl ZsyncRemoteControlFileParserPrivate::getTargetFileUrl(void)
{
    QUrl ret = (_uTargetFileUrl.isRelative()) ? 
	       QUrl(_uControlFileUrl.adjusted(QUrl::RemoveFilename).toString() + _sTargetFileName)
	       : _uTargetFileUrl;
    return ret;
}

/*
 * Returns the SHA1 Hash of the target file.
*/
QString ZsyncRemoteControlFileParserPrivate::getTargetFileSHA1(void)
{
    return _sTargetFileSHA1;
}

/*
 * Returns the Date and Time of Target file's MTime.
 * In case a process is busy or the control file was never parsed
 * then this would return an invalid datetime class.
*/
QDateTime ZsyncRemoteControlFileParserPrivate::getMTime(void)
{
    return _pMTime;
}

/*
 * Returns the BlockSize of the target file.
 * This would return 0 if any process is busy or the control file
 * was never parsed.
*/
size_t ZsyncRemoteControlFileParserPrivate::getTargetFileBlockSize(void)
{
    return _nTargetFileBlockSize;
}

/*
 * Returns the target file's length.
 * This would return 0 if any process is busy or the control file
 * was never parsed.
*/
size_t ZsyncRemoteControlFileParserPrivate::getTargetFileLength(void)
{
    return _nTargetFileLength;
}

/*
 * Returns the number of weak checksum bytes available in the
 * zsync control file.
*/
qint32 ZsyncRemoteControlFileParserPrivate::getWeakCheckSumBytes(void)
{
    return _nWeakCheckSumBytes;
}

/*
 * Returns the number of strong checksum bytes available in the
 * zsync control file.
*/
qint32 ZsyncRemoteControlFileParserPrivate::getStrongCheckSumBytes(void)
{
    return _nStrongCheckSumBytes;
}

/*
 * Returns the number of sequence matches needed for the zsync algorithm.
*/
qint32 ZsyncRemoteControlFileParserPrivate::getConsecutiveMatchNeeded(void)
{
    return _nConsecutiveMatchNeeded;
}

/*
 * Calculates the download progress of the control file in percentage
 * and emits it to the user.
*/
void ZsyncRemoteControlFileParserPrivate::handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    int nPercentage =
        static_cast<int>(
            (static_cast<float>
             (
                 bytesReceived
             ) * 100.0
            ) / static_cast<float>
            (
                bytesTotal
            )
        );
    emit progress(nPercentage);
    return;
}

/*
 * This private slot parses the latest bintray package url.
*/
void ZsyncRemoteControlFileParserPrivate::handleBintrayRedirection(const QUrl &url)
{
    INFO_START LOGR " handleBintrayRedirection : start to parse latest package url." INFO_END;
    emit statusChanged(PARSING_BINTRAY_REDIRECTED_URL_FOR_LATEST_PACKAGE_URL);
    QNetworkReply *senderReply = (QNetworkReply*)QObject::sender();
    int responseCode = senderReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    INFO_START LOGR " handleBintrayRedirection : http response code(" LOGR responseCode LOGR ")." INFO_END;
    if(responseCode > 400) { // Check if we have a bad response code.
        senderReply->abort();
	senderReply->deleteLater();
	emit error(ERROR_RESPONSE_CODE);
        return;
    }
    /* cut all ties. */
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, &QNetworkReply::redirected , this, &ZsyncRemoteControlFileParserPrivate::handleBintrayRedirection);

    senderReply->abort();
    senderReply->deleteLater();

    INFO_START LOGR " handleBintrayRedirection : redirected url(" LOGR url LOGR ")." INFO_END;

    QStringList information = url.toString().split("/");
    _sZsyncFileName.replace("_latestVersion", url.fileName());
    setControlFileUrl(QUrl("https://dl.bintray.com/" + information[3] + "/" + information[4] + "/" + _sZsyncFileName));
    getControlFile();
    return;
} 

/*
 * This private slot parses the github api response.
*/
void ZsyncRemoteControlFileParserPrivate::handleGithubAPIResponse(void)
{
    INFO_START LOGR " handleGithubAPIResponse : starting to parse github api response." INFO_END;
    emit statusChanged(PARSING_GITHUB_API_RESPONSE);
    QNetworkReply *senderReply = (QNetworkReply*)QObject::sender();
    int responseCode = senderReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    INFO_START LOGR " handleGithubAPIResponse : http response code(" LOGR responseCode LOGR ")." INFO_END;
    if(responseCode > 400) { // Check if we have a bad response code.
        senderReply->deleteLater();
	if(responseCode == 403){ // Check if we hit rate limit.
		emit error(GITHUB_API_RATE_LIMIT_REACHED);
	}else { 
		emit error(ERROR_RESPONSE_CODE);
	}
	return;
    }

    /* Cut all ties. */
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, SIGNAL(finished(void)), this, SLOT(handleGithubAPIResponse(void)));
   
    QJsonDocument jsonResponse = QJsonDocument::fromJson(senderReply->readAll());
    senderReply->deleteLater();
    
    QJsonObject jsonObject = jsonResponse.object();
    QJsonArray assetsArray = jsonObject["assets"].toArray();
    QString version = jsonObject["tag_name"].toString();
    QVector<QJsonObject> assets;
    QRegExp rx(_sZsyncFileName); // Patern Matching with wildcards!
    rx.setPatternSyntax(QRegExp::Wildcard);

    INFO_START " handleGithubAPIResponse : latest version is " LOGR version INFO_END;
    INFO_START " handleGithubAPIResponse : asset required is " LOGR _sZsyncFileName INFO_END;

    foreach (const QJsonValue &value, assetsArray){
	auto asset = value.toObject();
	INFO_START " handleGithubAPIResponse : inspecting asset(" LOGR asset["name"].toString() INFO_END;
        if(rx.exactMatch(asset["name"].toString())) {
            setControlFileUrl(QUrl(asset["browser_download_url"].toString()));
            getControlFile();
	    break;
        }
	QCoreApplication::processEvents();
    }
    return;
}

/*
 * This private slot parses the control file.
*/
void ZsyncRemoteControlFileParserPrivate::handleControlFile(void)
{
    INFO_START LOGR " handleControlFile : starting to parse zsync control file." INFO_END;
    emit statusChanged(PARSING_ZSYNC_CONTROL_FILE);	
    QNetworkReply *senderReply = (QNetworkReply*)QObject::sender();
    int responseCode = senderReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    INFO_START LOGR " handleControlFile : http response code(" LOGR responseCode LOGR ")." INFO_END;
    if(responseCode > 400) {
	senderReply->deleteLater();
        emit error(ERROR_RESPONSE_CODE);
        return;
    }

    /*
     * Disconnect all ties before casting it as QIODevice to read.
    */
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this, SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, SIGNAL(finished(void)), this, SLOT(handleControlFile(void)));
    disconnect(senderReply, SIGNAL(downloadProgress(qint64, qint64)),
               this, SLOT(handleDownloadProgress(qint64, qint64)));

    _pControlFile = QSharedPointer<QBuffer>(new QBuffer);
    _pControlFile->open(QIODevice::ReadWrite);

    /*
     * Since QNetworkReply is sequential QIODevice , We cannot seek but we
     * need to seek on command for future operation so we copy everything
     * to our newly allocated QIODevice and Also find the offset on the way.
     * Since this is sequential , We cannot use *->pos() to get the current position
     * of the file pointer , Therefore we will use a temporary variable.
     */
    {
        INFO_START LOGR " handleControlFile : searching for checksum blocks offset in the zsync control file." INFO_END;
        qint64 pos = 0;
	char previousMark = 0;
        int bufferSize = 1; // 1 Byte for now.
        emit statusChanged(SEARCHING_TARGET_FILE_CHECKSUM_BLOCK_OFFSET_IN_ZSYNC_CONTROL_FILE);
	while(!senderReply->atEnd()) {
            /*
             * Read one byte at a time , Since the marker for the
             * offset of the checksum blocks is \n\n.
             *
             * Therefore,
             *
             * ZsyncHeaders = (0 , offset - 2)
             * Checksums = (offset , EOF)
            */
            QByteArray data = senderReply->read(bufferSize);
	    pos += bufferSize;
	    if(bufferSize < 1024 && previousMark == 10 && data.at(0) == 10) {
                /*
                 * Set the offset and increase the buffer size
                 * to finish the rest of the copy faster.
                */
                INFO_START LOGR " handleControlFile : found checksum blocks offset(" LOGR pos LOGR ") in zsync control file." INFO_END;
                _nCheckSumBlocksOffset = pos;
                bufferSize = 1024; /* Use standard size of 1024 bytes. */
            }else{
		previousMark = data.at(0);
	    }
            _pControlFile->write(data);
        }
    }
    senderReply->deleteLater();

    _pControlFile->seek(0); /* seek to the top again. */
    if(!_nCheckSumBlocksOffset) {
        /* error , we don't know the marker and therefore it must be an invalid control file.*/
        emit error(NO_MARKER_FOUND_IN_CONTROL_FILE );
        return;
    }

    emit statusChanged(STORING_ZSYNC_CONTROL_FILE_DATA_TO_MEMORY);

    QString ZsyncHeader(_pControlFile->read(_nCheckSumBlocksOffset - 2)); /* avoid reading the marker. */
    QStringList ZsyncHeaderList = QString(ZsyncHeader).split("\n");
    if(ZsyncHeaderList.size() < 8) {
        emit error(INVALID_ZSYNC_HEADERS_NUMBER);
        return;
    }

    _sZsyncMakeVersion = ZsyncHeaderList.at(0).split("zsync: ")[1];
    if(_sZsyncMakeVersion == ZsyncHeaderList.at(0)) {
        emit error(INVALID_ZSYNC_MAKE_VERSION);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync make version confirmed to be " LOGR _sZsyncMakeVersion LOGR "." INFO_END;

    _sTargetFileName = ZsyncHeaderList.at(1).split("Filename: ")[1];
    if(_sTargetFileName == ZsyncHeaderList.at(1)) {
        emit error(INVALID_ZSYNC_TARGET_FILENAME);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file name confirmed to be " LOGR _sTargetFileName LOGR "." INFO_END;

    _pMTime = QDateTime::fromString(ZsyncHeaderList.at(2).split("MTime: ")[1], "ddd, dd MMM yyyy HH:mm:ss +zzz0");
    if(!_pMTime.isValid()) {
        emit error(INVALID_ZSYNC_MTIME);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file MTime confirmed to be " LOGR _pMTime LOGR "." INFO_END;

    _nTargetFileBlockSize = (size_t)ZsyncHeaderList.at(3).split("Blocksize: ")[1].toInt();
    if(_nTargetFileBlockSize < 1024) {
        emit error(INVALID_ZSYNC_BLOCKSIZE);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file blocksize confirmed to be " LOGR _nTargetFileBlockSize LOGR " bytes." INFO_END;

    _nTargetFileLength =  (size_t)ZsyncHeaderList.at(4).split("Length: ")[1].toInt();
    if(_nTargetFileLength == 0) {
        emit error(INVALID_TARGET_FILE_LENGTH);
        return;
    }
    INFO_START LOGR " handleControlFile : zysnc target file length confirmed to be " LOGR _nTargetFileLength LOGR " bytes." INFO_END;

    QString HashLength = ZsyncHeaderList.at(5).split("Hash-Lengths: ")[1];
    QStringList HashLengths = HashLength.split(',');
    if(HashLengths.size() != 3) {
        emit error(INVALID_HASH_LENGTH_LINE);
        return;
    }

    _nConsecutiveMatchNeeded = HashLengths.at(0).toInt();
    _nWeakCheckSumBytes = HashLengths.at(1).toInt();
    _nStrongCheckSumBytes = HashLengths.at(2).toInt();
    if(_nWeakCheckSumBytes < 1 || _nWeakCheckSumBytes > 4
       || _nStrongCheckSumBytes < 3 || _nStrongCheckSumBytes > 16
       || _nConsecutiveMatchNeeded > 2 || _nConsecutiveMatchNeeded < 1) {
        emit error(INVALID_HASH_LENGTHS);
        return;
    }

    INFO_START LOGR " handleControlFile : " LOGR _nWeakCheckSumBytes LOGR " bytes of weak checksum is available." INFO_END;
    INFO_START LOGR " handleControlFile : " LOGR _nStrongCheckSumBytes LOGR " bytes of strong checksum is available." INFO_END;
    INFO_START LOGR " handleControlFile : " LOGR _nConsecutiveMatchNeeded LOGR " consecutive matches is needed." INFO_END;

    _uTargetFileUrl = QUrl(ZsyncHeaderList.at(6).split("URL: ")[1]);
    if(!_uTargetFileUrl.isValid()) {
        emit error(INVALID_TARGET_FILE_URL);
        return;
    }
    INFO_START LOGR " handleControlFile : zsync target file url is confirmed to be " LOGR _uTargetFileUrl LOGR "." INFO_END;

    _sTargetFileSHA1 = ZsyncHeaderList.at(7).split("SHA-1: ")[1];
    if(_sTargetFileSHA1 == ZsyncHeaderList.at(7)) {
        emit error(INVALID_TARGET_FILE_SHA1);
        return;
    }
    _sTargetFileSHA1 = _sTargetFileSHA1.toUpper();
    INFO_START LOGR " handleControlFile : zsync target file sha1 hash is confirmed to be " LOGR _sTargetFileSHA1 LOGR "." INFO_END;

    _nTargetFileBlocks = (_nTargetFileLength + _nTargetFileBlockSize - 1) / _nTargetFileBlockSize;
    INFO_START LOGR " handleControlFile : zsync target file has " LOGR _nTargetFileBlocks LOGR " number of blocks." INFO_END;

    emit statusChanged(FINALIZING_PARSING_ZSYNC_CONTROL_FILE);
    emit receiveControlFile();
    emit statusChanged(IDLE);
    return;
}

void ZsyncRemoteControlFileParserPrivate::handleNetworkError(QNetworkReply::NetworkError errorCode)
{
    if(errorCode == QNetworkReply::OperationCanceledError) {
        return;
    }
    QNetworkReply *senderReply = (QNetworkReply*)QObject::sender();
    disconnect(senderReply, SIGNAL(error(QNetworkReply::NetworkError)),
               this,SLOT(handleNetworkError(QNetworkReply::NetworkError)));
    disconnect(senderReply, SIGNAL(finished(void)), this, SLOT(handleControlFile(void)));
    disconnect(senderReply, SIGNAL(downloadProgress(qint64, qint64)),
               this, SLOT(handleDownloadProgress(qint64, qint64)));

    FATAL_START LOGR " handleNetworkError : " LOGR errorCode LOGR "." FATAL_END;

    senderReply->deleteLater();
    emit error(UNKNOWN_NETWORK_ERROR);
    return;
}

/*
 * This slot will be called anytime error signal is emitted.
*/
void ZsyncRemoteControlFileParserPrivate::handleErrorSignal(short errorCode)
{
    emit statusChanged(IDLE);
    FATAL_START LOGR " error : " LOGR errorCodeToString(errorCode) LOGR " occured.";
    clear(); // clear all data to prevent later corrupted data collisions.
    return;
}

#ifndef LOGGING_DISABLED
void ZsyncRemoteControlFileParserPrivate::handleLogMessage(QString message , QString AppImageName)
{
    qInfo().noquote() << "["
	     << QDateTime::currentDateTime().toString(Qt::ISODate)
	     << "] "
	     << _sLoggerName
	     << "("
	     << AppImageName
	     << "):: "
	     << message;
    return;
}
#endif // LOGGING_DISABLED

QString ZsyncRemoteControlFileParserPrivate::errorCodeToString(short errorCode)
{
    QString errorCodeString = "AppImageDeltaWriter::errorCode(";
    switch(errorCode) {
    case UNKNOWN_NETWORK_ERROR:
        errorCodeString.append("UNKNOWN_NETWORK_ERROR");
        break;
    case IO_READ_ERROR:
        errorCodeString.append("IO_READ_ERROR");
        break;
    case ERROR_RESPONSE_CODE:
        errorCodeString.append("ERROR_RESPONSE_CODE");
        break;
    case NO_MARKER_FOUND_IN_CONTROL_FILE:
        errorCodeString.append("NO_MARKER_FOUND_IN_CONTROL_FILE");
        break;
    case INVALID_ZSYNC_HEADERS_NUMBER:
        errorCodeString.append("INVALID_ZSYNC_HEADERS_NUMBER");
        break;
    case INVALID_ZSYNC_MAKE_VERSION:
        errorCodeString.append("INVALID_ZSYNC_MAKE_VERSION");
        break;
    case INVALID_ZSYNC_TARGET_FILENAME:
        errorCodeString.append("INVALID_ZSYNC_TARGET_FILENAME");
        break;
    case INVALID_ZSYNC_MTIME:
        errorCodeString.append("INVALID_ZSYNC_MTIME");
        break;
    case INVALID_ZSYNC_BLOCKSIZE:
        errorCodeString.append("INVALID_ZSYNC_BLOCKSIZE");
        break;
    case INVALID_TARGET_FILE_LENGTH:
        errorCodeString.append("INVALID_TARGET_FILE_LENGTH");
        break;
    case INVALID_HASH_LENGTH_LINE:
        errorCodeString.append("INVALID_HASH_LENGTH_LINE");
        break;
    case INVALID_HASH_LENGTHS:
        errorCodeString.append("INVALID_HASH_LENGTHS");
        break;
    case INVALID_TARGET_FILE_URL:
        errorCodeString.append("INVALID_TARGET_FILE_URL");
        break;
    case INVALID_TARGET_FILE_SHA1:
        errorCodeString.append("INVALID_TARGET_FILE_SHA1");
        break;
    default:
        errorCodeString.append("UNKNOWN_ERROR_CODE");
        break;
    }

    errorCodeString.append(")");
    return errorCodeString;
}

QString ZsyncRemoteControlFileParserPrivate::statusCodeToString(short code)
{
	QString statusCodeString = "AppImageDeltaWriter::statusCode(";
	switch(code){
		case INITIALIZING:
			statusCodeString.append("INITIALIZING");
			break;
		case IDLE:
			statusCodeString.append("IDLE");
			break;
		case PARSING_APPIMAGE_EMBEDED_UPDATE_INFORMATION:
			statusCodeString.append("PARSING_APPIMAGE_EMBEDED_UPDATE_INFORMATION");
			break;
		case REQUESTING_GITHUB_API:
			statusCodeString.append("REQUESTING_GITHUB_API");
			break;
		case PARSING_GITHUB_API_RESPONSE:
			statusCodeString.append("PARSING_GITHUB_API_RESPONSE");
			break;
		case REQUESTING_ZSYNC_CONTROL_FILE:
			statusCodeString.append("REQUESTING_ZSYNC_CONTROL_FILE");
			break;
		case REQUESTING_BINTRAY:
			statusCodeString.append("REQUESTING_BINTRAY");
			break;
		case PARSING_BINTRAY_REDIRECTED_URL_FOR_LATEST_PACKAGE_URL:
			statusCodeString.append("PARSING_BINTRAY_REDIRECTED_URL_FOR_LATEST_PACKAGE_URL");
			break;
		case PARSING_ZSYNC_CONTROL_FILE:
			statusCodeString.append("PARSING_ZSYNC_CONTROL_FILE");
			break;
		case SEARCHING_TARGET_FILE_CHECKSUM_BLOCK_OFFSET_IN_ZSYNC_CONTROL_FILE:
			statusCodeString.append("SEARCHING_TARGET_FILE_CHECKSUM_BLOCK_OFFSET_IN_ZSYNC_CONTROL_FILE");
			break;
		case STORING_ZSYNC_CONTROL_FILE_DATA_TO_MEMORY:
			statusCodeString.append("STORING_ZSYNC_CONTROL_FILE_DATA_TO_MEMORY");
			break;
		case FINALIZING_PARSING_ZSYNC_CONTROL_FILE:
			statusCodeString.append("FINALIZING_PARSING_ZSYNC_CONTROL_FILE");
			break;
		default:
			statusCodeString.append("Unknown");
			break;
	}
	statusCodeString.append(")");
	return statusCodeString;
}
