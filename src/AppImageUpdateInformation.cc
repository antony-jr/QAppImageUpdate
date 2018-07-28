#include <AppImageUpdateInformation.hpp>

using namespace AppImageUpdaterBridge;

/*
 * Prints to the log.
 * LOGS,LOGE  -> Prints normal log messages.
 * INFO_START,INFO_END -> Prints info messages to log.
 * WARNING_START,WARNING_END -> Prints warning messages to log.
 * FATAL_START,FATAL_END -> Prints fatal messages to log.
 *
 * Example:
 * 	LOGS "This is a log message." LOGE
 *
 *
*/
#ifndef LOGGING_DISABLED
#define LOGS *(_pLogger.data()) <<
#define LOGR <<
#define LOGE ; \
	     emit(logger(_sLogBuffer , _sAppImagePath)); \
	     _sLogBuffer.clear();
#else
#define LOGS (void)
#define LOGR ;(void)
#define LOGE ;
#endif // LOGGING_DISABLED

#define INFO_START LOGS "   INFO: " LOGR
#define INFO_END LOGE

#define WARNING_START LOGS "WARNING: " LOGR
#define WARNING_END LOGE

#define FATAL_START LOGS "  FATAL: " LOGR
#define FATAL_END LOGE


/*
 * Macros used for readbility and to reduce repeated
 * code in the source.
 *
 * Warning:
 *     Hardcoded , Do not use this outside AppImageUpdateInformation
 *     class.
 *
 * Example:
 * 	MEMORY_ERROR()
*/
#define ERROR_STATE _bStarted = _bFinished = _bPaused \
		    = _bCanceled = _bResumeRequested = _bCancelRequested \
		    = _bPauseRequested = false;
#define MEMORY_ERROR() emit(error(NOT_ENOUGH_MEMORY)); ERROR_STATE
#define APPIMAGE_OPEN_ERROR() emit(error(CANNOT_OPEN_APPIMAGE)); ERROR_STATE
#define APPIMAGE_PERMISSION_ERROR() emit(error(NO_READ_PERMISSION)); ERROR_STATE
#define APPIMAGE_NOT_FOUND_ERROR() emit(error(APPIMAGE_NOT_FOUND)); ERROR_STATE
#define APPIMAGE_READ_ERROR() emit(error(APPIMAGE_NOT_READABLE)); ERROR_STATE
#define APPIMAGE_INVALID_UI_ERROR() emit(error(INVALID_UPDATE_INFORMATION)); ERROR_STATE
#define APPIMAGE_EMPTY_UI_ERROR() emit(error(EMPTY_UPDATE_INFORMATION)); ERROR_STATE
#define MAGIC_BYTES_ERROR() emit(error(INVALID_MAGIC_BYTES)); ERROR_STATE
#define ELF_FORMAT_ERROR() emit(error(UNSUPPORTED_ELF_FORMAT)); ERROR_STATE
#define SECTION_HEADER_NOT_FOUND_ERROR() emit(error(SECTION_HEADER_NOT_FOUND)); ERROR_STATE
#define APPIMAGE_TYPE_ERROR() emit(error(INVALID_APPIMAGE_TYPE)); ERROR_STATE
#define UNSUPPORTED_TRANSPORT_ERROR() emit(error(UNSUPPORTED_TRANSPORT)); ERROR_STATE


/*
 * Returns true if the magic byte is typeX AppImage.
 *
 * Warning:
 * 	This is hardcoded and thus the variable here corresponds
 * 	to the local variable declared in AppImageUpdateInformation::syncInformation
 *
*/
#define TYPE_1_APPIMAGE ((int)magicBytes[2] == 1)
#define TYPE_2_APPIMAGE ((int)magicBytes[2] == 2)


/*
 * Returns true if the given binary is a ElfXX executable.
 *
 * Warning:
 * 	This is hardcoded and thus the variable here corresponds
 * 	to the local variable declared in AppImageUpdateInformation::syncInformation
 *
*/
#define isElf32(data) (((Elf32_Ehdr*)data)->e_ident[EI_CLASS] == ELFCLASS32)
#define isElf64(data) (((Elf64_Ehdr*)data)->e_ident[EI_CLASS] == ELFCLASS64)

/*
 * Pauses the current thread until _bResumeRequested is true.
 * Checks for the bool every 5 seconds.
 *
 * Warning:
 * 	This is hardcoded and This has to be used in
 * 	AppImageUpdateInformation::syncInformation private slot.
 *
 * Example:
 * 	pauseIfRequested();
 *	// This will only run after _bResumeRequested is set to true.
*/
#define pauseIfRequested() if(_bPauseRequested) { \
				{ \
					QMutexLocker locker(&_pMutex); \
					_bPaused = true; \
					_bStarted = false; \
					_bResumeRequested = false; \
					_bPauseRequested = false; \
				} \
				emit(paused()); \
				while(!_bResumeRequested) { \
					QThread::sleep(5); \
				} \
				{ \
					QMutexLocker locker(&_pMutex); \
					_bPaused = false; \
					_bStarted = true; \
				} \
				emit(resumed()); \
				}


/*
 * Sets the offset and length of the need section header
 * from a elf file.
 *
 * Warning:
 * 	Heavily hardcoded , Do not use this outside of AppImageUpdateInformation::syncInformation.
 *
 * Example:
 * 	long unsigned offset = 0 , length = 0;
 *      ElfXX_Ehdr *elfXX = (ElfXX_Ehdr *) data;
 *      ElfXX_Shdr *shdrXX = (ElfXX_Shdr *) (data + elfXX->e_shoff);
 *      strTab = (char *)(data + shdrXX[elfXX->e_shstrndx].sh_offset);
 *
 *      lookupSectionHeaders(strTab , shdr , elf , ".section_header_name");
 *
 *      // Now use offset and length (does nothing if the header is not found).
 */
#define lookupSectionHeaders(strTab , shdr , elf , section) for(int i = 0; i < elf->e_shnum; i++) { \
						  pauseIfRequested(); \
						  if(_bCancelRequested){ \
						      { \
						          QMutexLocker locker(&_pMutex); \
							  _bStarted = false; \
							  _bCanceled = true; \
							  _bCancelRequested = false; \
						      } \
						      _pAppImage->unmap(mapped); \
						      emit(canceled()); \
						      return; \
						  } \
						  emit(progress((int)((i * 100)/elf->e_shnum))); \
						  if(!strcmp(&strTab[shdr[i].sh_name] , section)){ \
							  offset = shdr[i].sh_offset; \
							  length = shdr[i].sh_size; \
							  emit(progress(80)); \
							  break; \
						  } \
						  }


/*
 * Since the constructor uses this code more often and
 * only the type of 'x' is unique we can use a simple
 * macro to reduce the number of lines in the source.
 *
 * Example:
 * 	CONSTRUCT(QFile *) or
 * 	CONSTRUCT(QString&) or
 * 	CONSTRUCT(nullptr)
*/
#define CONSTRUCT(x) try { \
 		     _pLogger = QSharedPointer<QDebug>(new QDebug(&_sLogBuffer)); \
		     } catch ( ... ) { \
 		     MEMORY_ERROR(); \
 		     throw; \
 		     } \
		     setAppImage(x);


/*
 * Returns a new QByteArray which contains the contents from
 * the given QIODevice from the given offset to the given
 * max count.
 * This function does not change the position of the QIODevice.
 *
 * Example:
 * 	QFile file("Some.AppImage")
 * 	file.open(QIODevice::ReadOnly);
 * 	QByteArray data = read((QIODevice*)&file , 512 , 1024);
*/
static QByteArray read(QIODevice *IO, qint64 offset, qint64 max)
{
    QByteArray ret;
    qint64 before = IO->pos();
    IO->seek(offset);
    ret = IO->read(max);
    IO->seek(before);
    return ret;
}

/*
 * A Dummy destructor for QFile* , used with smart
 * pointers where the QFile* is not intended to be
 * deleted when dereferenced. Useful when QSharedPointer
 * has to be used as a QWeakPointer which is required
 * in setAppImage(QFile*).
 *
 * Example:
 *      QFile *file = new QFile;
 * 	QSharedPointer<QFile> smartPointer = QSharedPointer<QFile>(file , doNotDelete);
 * 	smartPointer.clear();
 *
 * 	file->setFileName("File.txt"); // Can be reused by the user.
 * 	delete file; // But has to deallocated by the user.
*/
static void doNotDelete(QFile *file)
{
    (void)file;
    return;
}


/*
 * AppImageUpdateInformation is the main class that provides the
 * ability to easily get the update information from an AppImage.
 * This class can be constructed in three ways.
 * The default construct sets the QObject parent to be null and
 * creates an empty AppImageUpdateInformation Object.
 * The QString Overloaded Constructor , takes a QString as the path
 * to the AppImage and loads it into the memory using QFile , It also
 * opens the AppImage to start reading in the future.
 * The QFile pointer Overloaded Constructor takes a pointer to QFile
 * to use as the AppImage , Here the given QFile is not opened and
 * it has to opened before submiting to the constructor or else
 * it will result in an error , the user is responsible to close
 * the given QFile.
 *
 * Example:
 * 	QObject parent;
 * 	AppImageUpdateInformation AppImageInfo;
 *	// or
 *	AppImageUpdateInformation AppImageInfo(&parent);
 *
 *	AppImageUpdateInformation AppImageInfo("PathTo.AppImage");
 *	// or
 *	AppImageUpdateInformation AppImageInfo("PathTo.AppImage" , &parent);
 *
 *	QFile file("PathTo.AppImage");
 *	file.open(QIODevice::ReadOnly);
 *	AppImageUpdateInformation AppImageInfo(&file);
 *	// or
 *	AppImageUpdateInformation AppImageInfo(&file , &parent);
*/
AppImageUpdateInformation::AppImageUpdateInformation(QObject *parent)
    : QObject(parent)
{
    CONSTRUCT(nullptr);
    return;
}

AppImageUpdateInformation::AppImageUpdateInformation(const QString &AppImagePath, QObject *parent)
    : QObject(parent)
{
    CONSTRUCT(AppImagePath);
    return;
}

AppImageUpdateInformation::AppImageUpdateInformation(QFile *AppImage, QObject *parent)
    : QObject(parent)
{
    CONSTRUCT(AppImage);
    return;
}

/*
 * Destructs the AppImageUpdateInformation ,
 * When the user provides the AppImage as a QFile ,
 * QFile is not closed , the user is fully responsible
 * to deallocate or close the QFile.
*/
AppImageUpdateInformation::~AppImageUpdateInformation()
{
    if (!_pMutex.tryLock()){
	WARNING_START " ~ : Mutex is locked , unlocking and proceeding." WARNING_END;
        _pMutex.unlock();
    }else{
        _pMutex.unlock();	    
    }
    if((isRunning() || isPaused()) && !isFinished()) {
        WARNING_START  " ~ : Destructing without stopping other threads , may cause segfault." WARNING_END;
    }
    _pLogger.clear();
    _pAppImage.clear();
    _pFuture.clear();
    return;
}


/*
 * This method returns the caller object and sets the AppImage
 * referenced by the given QString , The QString is expected to be a
 * valid path either an absolute or a relative one.
 * if the path is empty then this exits doing nothing.
 * if the path specified does not exist then this will fire
 * the error signal.
 *
 * Example:
 * 	AppImageUpdateInformation AppImageInfo;
 * 	AppImageInfo.setAppImage("PathTo.AppImage");
 *
 */
AppImageUpdateInformation &AppImageUpdateInformation::setAppImage(const QString &AppImagePath)
{
    clear(); /* clear old data */
    if(!_pMutex.tryLock()){
	    WARNING_START " setAppImage : mutex is locked , ignoring operation." WARNING_END;
	    return *this;
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);

    if(isRunning() || isPaused() || isStarted()) {
        WARNING_START  " setAppImage : busy , operation ignored." WARNING_END;
        return *this;
    } else if(AppImagePath.isEmpty()) {
        return *this;
    }

    /*
    * Set the AppImage Name as a nickname for identification
    * on the logger.
    */
    _sAppImagePath = AppImagePath;
    _sAppImageName = QFileInfo(AppImagePath).fileName();

    try {
        _pAppImage = QSharedPointer<QFile>(new QFile);
    } catch ( ... ) {
        MEMORY_ERROR();
        throw;
    }


    INFO_START  " setAppImage : " LOGR AppImagePath LOGR "." INFO_END;

    _pAppImage->setFileName(AppImagePath);

    /* Check if the file actually exists. */
    if(!_pAppImage->exists()) {
        _pAppImage.clear(); /* delete everything to avoid futher errors. */
        FATAL_START  " setAppImage : cannot find the AppImage in the given path , file not found." FATAL_END;
        APPIMAGE_NOT_FOUND_ERROR();
        return *this;
    }

    /* Check if we have the permission to read it. */
    auto perm = _pAppImage->permissions();
    if(
        !(perm & QFileDevice::ReadUser) &&
        !(perm & QFileDevice::ReadGroup) &&
        !(perm & QFileDevice::ReadOther)
    ) {
        _pAppImage.clear();
        FATAL_START  " setAppImage : no permission("LOGR perm LOGR ") for reading the given AppImage." FATAL_END;
        APPIMAGE_PERMISSION_ERROR();
        return *this;
    }

    /*
     * Finally open the file.
    */
    if(!_pAppImage->open(QIODevice::ReadOnly)) {
        _pAppImage.clear();
        FATAL_START  " setAppImage : cannot open AppImage for reading." FATAL_END;
        APPIMAGE_OPEN_ERROR();
        return *this;
    }

    return *this;
}


/*
 * This is a overloaded method , Sets the AppImage with reference
 * to the given QFile pointer , The given QFile has to be opened and
 * must be readable.
 *
 * Example:
 * 	AppImageUpdateInformation AppImageInfo;
 * 	QFile file("PathTo.AppImage");
 * 	file.open(QIODevice::ReadOnly);
 * 	AppImageInfo.setAppImage(&file);
 * 	file.close();
*/
AppImageUpdateInformation &AppImageUpdateInformation::setAppImage(QFile *AppImage)
{
    clear(); /* clear old data. */
    if(!_pMutex.tryLock()){
	    WARNING_START " setAppImage : mutex is locked , ignoring operation." WARNING_END;
	    return *this;
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);

    if(isRunning() || isPaused() || isStarted()) {
        WARNING_START  " setAppImage : busy , operation ignored." WARNING_END;
        return *this;
    } else if(AppImage == nullptr) {
        return *this;
    }

    /* Set nickname and cache path. */
    _sAppImagePath = AppImage->fileName();
    _sAppImageName = QFileInfo(_sAppImagePath).fileName();

    _pAppImage = QSharedPointer<QFile>(AppImage, doNotDelete);

    INFO_START  " setAppImage : " LOGR _pAppImage->fileName() LOGR "." INFO_END;

    /* Check if exists */
    if(!_pAppImage->exists()) {
        _pAppImage.clear();
        FATAL_START  " setAppImage : cannot find the AppImage from given QFile , file does not exists." FATAL_END;
        APPIMAGE_NOT_FOUND_ERROR();
        return *this;
    }

    /* Check if readable. */
    if(!_pAppImage->isReadable()) {
        _pAppImage.clear(); /* delete everything */
        FATAL_START  " setAppImage : invalid QFile given, not readable." FATAL_END;
        APPIMAGE_READ_ERROR();
        return *this;
    }

    /* Check if opened. */
    if(!_pAppImage->isOpen()) {
        _pAppImage.clear(); /* delete everything */
        FATAL_START  " setAppImage : invalid QFile given, not opened." FATAL_END;
        APPIMAGE_OPEN_ERROR();
        return *this;
    }

    return *this;
}

/*
 * This returns the caller object , If the given bool
 * is true then it connects the logger signal to the
 * logPrinter slot to enable debugging messages.
 * On false this disconnects the logPrinter.
 *
 * Example:
 * 	AppImageUpdateInformation AppImageInfo("PathTo.AppImage");
 * 	AppImageInfo.setShowLog(true);
*/
AppImageUpdateInformation &AppImageUpdateInformation::setShowLog(bool logNeeded)
{
    if(!_pMutex.tryLock()){
	    WARNING_START " setShowLog : mutex is locked , ignoring operation." WARNING_END;
	    return *this;
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);
    if(logNeeded) {
        disconnect(this, &AppImageUpdateInformation::logger, this, &AppImageUpdateInformation::logPrinter);
        connect(this, &AppImageUpdateInformation::logger, this, &AppImageUpdateInformation::logPrinter);
        INFO_START  " setShowLog : true  , started logging." INFO_END;

    } else {
        INFO_START  " setShowLog : false , finishing logging." INFO_END;
        disconnect(this, &AppImageUpdateInformation::logger, this, &AppImageUpdateInformation::logPrinter);
    }
    return *this;
}


/*
 * This returns the copy to the data held inside
 * a QJsonObject privately. If this is empty then
 * that means the user never started anything.
 *
 * Example:
 * 	QJsonObject info = AppImageInfo.getInfo();
*/
QJsonObject AppImageUpdateInformation::getInfo(void)
{
    if(!_pMutex.tryLock()){
	    WARNING_START " getInfo : mutex is locked , ignoring operation." WARNING_END;
	    return QJsonObject();
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);
    return _jInfo;
}

/*
 * This returns the current AppImage name which is
 * cached by setAppImage.
 *
 * Example:
 * 	QString AppImageName = AppImageInfo.getAppImageName();
*/
QString AppImageUpdateInformation::getAppImageName(void)
{
    if(!_pMutex.tryLock()){
	    WARNING_START " getAppImageName : mutex is locked , ignoring operation." WARNING_END;
	    return QString();
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);
    return _sAppImageName;
}

/*
 * This returns the current AppImage's full path which
 * is cached by setAppImage.
 *
 * Example:
 * 	QString AppImagePath = AppImageInfo.getAppImagePath();
*/
QString AppImageUpdateInformation::getAppImagePath(void)
{
    if(!_pMutex.tryLock()){
	    WARNING_START " getAppImagePath : mutex is locked , ignoring operation." WARNING_END;
	    return QString();
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);
    return _sAppImagePath;
}

/*
 * This returns the caller object , This clears all
 * the data held in the current object , making it
 * reusable.
 *
 * Example:
 * 	AppImageInfo.clear();
*/
AppImageUpdateInformation &AppImageUpdateInformation::clear(void)
{
    if(!_pMutex.tryLock()){
	    WARNING_START " clear : mutex is locked , ignoring operation." WARNING_END;
	    return *this;
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);
    
    if(isRunning() || isPaused()) {
        WARNING_START  " clear : busy , operation ignored." WARNING_END;
        return *this;
    }
    _bStarted = _bFinished = _bPaused = _bCanceled = _bResumeRequested = _bCancelRequested = _bPauseRequested = false;
    _jInfo = QJsonObject();
    _sLogBuffer.clear();
    _sAppImagePath.clear();
    _sAppImageName.clear();
    _pAppImage.clear();
    _pFuture.clear();

    INFO_START  " clear : flushed everything." INFO_END;
    return *this;
}

/*
 * Returns the caller object , Blocks the caller thread until
 * the process started by _pFuture is finished , (i.e) When
 * the finish signal is emitted in AppImageUpdateInformation::syncInformation.
*/
AppImageUpdateInformation &AppImageUpdateInformation::waitForFinished(void)
{
    if(!isRunning() || _pFuture.isNull() || isFinished()) {
        WARNING_START  " waitForFinished : no process is running." WARNING_END;
        return *this;
    }
    INFO_START  " waitForFinished : thread("
               LOGR QThread::currentThreadId() LOGR ") blocked until finish." INFO_END;
    _pFuture->waitForFinished();
    return *this;
}

/*
 * Returns the caller object , starts the information
 * syncing process in a new thread.
*/
AppImageUpdateInformation &AppImageUpdateInformation::start(void)
{
    if(!_pMutex.tryLock()){
	    WARNING_START " start : mutex is locked , ignoring operation." WARNING_END;
	    return *this;
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);
    if(
        isRunning() ||
        isStarted() ||
        isPaused()
    ) {
        WARNING_START  " start : already started." WARNING_END;
        return *this;
    } else if(_pAppImage.isNull()) {
        /*
         * Ignore this error since a error signal will be
         * fired from setAppImage method.
         */
        return *this;
    } else {
        INFO_START  " start : successfully started." INFO_END;

        _bFinished = false;
        _bStarted = true;

        _pFuture = QSharedPointer<QFuture<void>>(new QFuture<void>);
        *_pFuture = QtConcurrent::run(this, &AppImageUpdateInformation::syncInformation);
    }
    return *this;
}

/*
 * Returns the caller object , cancels any current opertions
*/
AppImageUpdateInformation &AppImageUpdateInformation::cancel(void)
{
    if(!_pMutex.tryLock()){
	    WARNING_START " cancel : mutex is locked , ignoring operation." WARNING_END;
	    return *this;
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);
    if(!isRunning() || !isStarted()) {
        WARNING_START  " cancel : no process is running." WARNING_END;
        return *this;
    }
    _bCancelRequested = true;
    INFO_START  " cancel : requesting cancel." INFO_END;
    return *this;
}

/*
 * Returns the caller object , pauses the new thread and waits
 * till resume slot is shot.
*/
AppImageUpdateInformation &AppImageUpdateInformation::pause(void)
{
    if(!_pMutex.tryLock()){
	    WARNING_START " pause : mutex is locked , ignoring operation." WARNING_END;
	    return *this;
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);
    if(!isRunning() || !isStarted() || isPaused()) {
        WARNING_START  " pause : already paused or no process is running." WARNING_END;
        return *this;
    }
    _bPauseRequested = true;
    INFO_START  " pause : requesting pause." INFO_END;
    return *this;
}

/*
 * Returns the caller object , resumes any paused process.
*/
AppImageUpdateInformation &AppImageUpdateInformation::resume(void)
{
    if(!_pMutex.tryLock()){
	    WARNING_START " resume : mutex is locked , ignoring operation." WARNING_END;
	    return *this;
    }
    _pMutex.unlock(); /* tryLock() locks the mutex , So we have to unlock before using it with QMutexLocker. */
    QMutexLocker locker(&_pMutex);
    if(!isPaused()) {
        WARNING_START  " resume : nothing is paused." WARNING_END;
        return *this;
    }
    _bResumeRequested = true;
    INFO_START  " resume : successfully resumed." INFO_END;
    return *this;
}

bool AppImageUpdateInformation::isRunning(void) const
{
    bool _bFutureRunning = false;
    if(!_pFuture.isNull()) {
        _bFutureRunning = _pFuture->isRunning();
    }
    return _bFutureRunning;
}

bool AppImageUpdateInformation::isCanceled(void) const
{
    return _bCanceled;
}

bool AppImageUpdateInformation::isPaused(void) const
{
    return _bPaused;
}

bool AppImageUpdateInformation::isStarted(void) const
{
    return _bStarted;
}

bool AppImageUpdateInformation::isFinished(void) const
{
    bool _bFutureFinished = false;
    if(!_pFuture.isNull()) {
        _bFutureFinished = _pFuture->isFinished();
    }
    return (_bFinished || _bFutureFinished);
}

/*
 * This is a private slot which is sync but this will
 * be runned in a sperate thread. This is the main
 * slot which reads the information from the provided
 * AppImage.
*/
void AppImageUpdateInformation::syncInformation(void)
{
    /*
     * Check for interruption requests.
    */
    pauseIfRequested();/*Blocks this thread until resume if requested.*/
    if(_bCancelRequested) {
        {
            QMutexLocker locker(&_pMutex);
            _bStarted = false;
            _bCanceled = true;
            _bCancelRequested = false;
        }
        emit(canceled());/*Signal canceled.*/
        return;/*terminates the current thread.*/
    }


    QString updateString;
    QStringList info;
    QIODevice *AppImage = (QIODevice*)_pAppImage.data();

    emit(started());/*Signal started.*/
    INFO_START  " syncInformation : new thread(" LOGR QThread::currentThreadId() LOGR ") started." INFO_END;

    /*
     * Read the magic byte , i.e the AI stamp
     * on the given binary. The characters 'AI'
     * are hardcoded at the offset 8 with a
     * maximum of 3 characters.
     * The 3rd character decides the type of the
     * AppImage.
    */
    auto magicBytes = read(AppImage, /*offset=*/8,/*maxchars=*/ 3);
    if (magicBytes[0] != 'A' && magicBytes[1] != 'I') {
        FATAL_START  " syncInformation : invalid magic bytes("
                    LOGR (unsigned)magicBytes[0] LOGR ","
                    LOGR (unsigned)magicBytes[1] LOGR ")." FATAL_END;
        MAGIC_BYTES_ERROR();
        return;
    }

    /*
     * 0x1H -> Type 1 AppImage.
     * 0x2H -> Type 2 AppImage. (Latest Version)
    */
    if(TYPE_1_APPIMAGE) {

        INFO_START  " syncInformation : AppImage is confirmed to be type 1." INFO_END;

        progress(/*percentage=*/80); /*Signal progress.*/
        updateString = QString::fromUtf8(read(AppImage, APPIMAGE_TYPE1_UPDATE_INFO_POS, APPIMAGE_TYPE1_UPDATE_INFO_LEN));

    } else if(TYPE_2_APPIMAGE) {

        INFO_START  " syncInformation : AppImage is confirmed to be type 2." INFO_END;
        INFO_START  " syncInformation : mapping AppImage to memory." INFO_END;

        {
            uint8_t *data = NULL;
            char *strTab = NULL;
            uchar *mapped = NULL;
            unsigned long offset = 0, length = 0;

            {
                QMutexLocker locker(&_pMutex);
                mapped = _pAppImage->map(/*offset=*/0, /*max=*/_pAppImage->size()); // mmap in Qt.
            }

            if(mapped == NULL) {
                FATAL_START  " syncInformation : not enough memory to map AppImage to memory." FATAL_END;
                MEMORY_ERROR();
                return;
            }

            data = (uint8_t*) mapped;
            if(isElf32(data)) {
                INFO_START  " syncInformation : AppImage architecture is x86 (32 bits)." INFO_END;

                Elf32_Ehdr *elf32 = (Elf32_Ehdr *) data;
                Elf32_Shdr *shdr32 = (Elf32_Shdr *) (data + elf32->e_shoff);

                strTab = (char *)(data + shdr32[elf32->e_shstrndx].sh_offset);
                lookupSectionHeaders(strTab, shdr32, elf32, APPIMAGE_TYPE2_UPDATE_INFO_SHDR);
            } else if(isElf64(data)) {
                INFO_START  " syncInformation : AppImage architecture is x86_64 (64 bits)." INFO_END;

                Elf64_Ehdr *elf64 = (Elf64_Ehdr *) data;
                Elf64_Shdr *shdr64 = (Elf64_Shdr *) (data + elf64->e_shoff);

                strTab = (char *)(data + shdr64[elf64->e_shstrndx].sh_offset);
                lookupSectionHeaders(strTab, shdr64, elf64, APPIMAGE_TYPE2_UPDATE_INFO_SHDR);
            } else {
                _pAppImage->unmap(mapped);
                FATAL_START  " syncInformation : Unsupported elf format." FATAL_END;
                ELF_FORMAT_ERROR();
                return;
            }

            _pAppImage->unmap(mapped); // equivalent to unmap.

            if(offset == 0 || length == 0) {
                FATAL_START  " syncInformation : cannot find '"
                            LOGR APPIMAGE_TYPE2_UPDATE_INFO_SHDR LOGR"' section header." FATAL_END;
                SECTION_HEADER_NOT_FOUND_ERROR();
            } else {
                updateString = QString::fromUtf8(read(AppImage, offset, length));
            }
        }
    } else {
        WARNING_START  " syncInformation : unable to confirm AppImage type." WARNING_END;
        if(
            (read(AppImage, ELF_MAGIC_POS, ELF_MAGIC_VALUE_SIZE) == ELF_MAGIC_VALUE) &&
            (read(AppImage, ISO_MAGIC_POS, ISO_MAGIC_VALUE_SIZE) == ISO_MAGIC_VALUE)
        ) {
            WARNING_START  " syncInformation : guessing AppImage type to be 1." WARNING_END;
            emit(progress(80));
            updateString = QString::fromUtf8(read(AppImage, APPIMAGE_TYPE1_UPDATE_INFO_POS, APPIMAGE_TYPE1_UPDATE_INFO_LEN));
        } else {
            FATAL_START  " syncInformation : invalid AppImage type(" LOGR (unsigned)magicBytes[2] LOGR ")." FATAL_END;
            APPIMAGE_TYPE_ERROR();
            return;
        }
    }

    /*
     * Safely close the AppImage since we no longer need
     * access to the AppImage.
     */
    _pAppImage->close();
    _pAppImage.clear(); /* delete QFile */

    INFO_START  " syncInformation : closed AppImage safely." INFO_END;

    if(updateString.isEmpty()) {
        FATAL_START  " syncInformation : update information is empty." FATAL_END;
        APPIMAGE_EMPTY_UI_ERROR();
        return;
    }

    INFO_START  " syncInformation : updateString("LOGR updateString LOGR ")." INFO_END;

    /*
     * Split the raw update information with the specified
     * delimiter.
    */
    info = updateString.split(APPIMAGE_UPDATE_INFO_DELIMITER);


    if(info.size() < 2) {
        FATAL_START  " syncInformation : update information has invalid delimiters." FATAL_END;
        APPIMAGE_INVALID_UI_ERROR();
        return;
    } else if(info.size() == 2) {
        QJsonObject buffer {
            { "transport", info.at(0) },
            { "zsyncUrl", info.at(1) }
        };
        _jInfo = buffer;
    } else if(info.size() == 5) {
        if(info.at(0) == "gh-releases-zsync") {
            QJsonObject buffer {
                {"transport", info.at(0) },
                {"username", info.at(1) },
                {"repo", info.at(2) },
                {"tag", info.at(3) },
                {"filename", info.at(4) }
            };
            _jInfo = buffer;
        } else if(info.at(0) == "bintray-zsync") {
            QJsonObject buffer {
                {"transport", info.at(0) },
                {"username", info.at(1) },
                {"repo", info.at(2) },
                {"packageName", info.at(3) },
                {"filename", info.at(4) }
            };
            _jInfo = buffer;
        } else {
            FATAL_START  " syncInformation : unsupported transport mechanism given." FATAL_END;
            UNSUPPORTED_TRANSPORT_ERROR();
            return;
        }

    } else {
        FATAL_START  " syncInformation : update information has invalid number of entries("LOGR info.size() LOGR")." FATAL_END;
        APPIMAGE_INVALID_UI_ERROR();
        return;
    }

    emit(progress(100)); /*Signal progress.*/
    /*reset.*/
    {
        QMutexLocker locker(&_pMutex);
        _bStarted = _bPaused = _bCanceled = false;
        _bFinished = true;
    }

    INFO_START  " syncInformation : finished." INFO_END;
    emit(finished());/*Signal finished.*/
    return;
}

/* This private slot proxies the log messages from
 * the logger signal to qDebug().
*/
void AppImageUpdateInformation::logPrinter(QString msg , QString path)
{
    (void)path;
    qDebug().noquote() << "["
                       <<  QDateTime::currentDateTime().toString(Qt::ISODate)
                       << "] AppImageUpdateInformation("
                       << _sAppImageName << ")::" << msg;
    return;
}

/*
 * This static method returns a QString which corresponds the
 * AppImageUpdateInformation::error_code , Useful when logging and debuging.
 *
 * Example:
 * 	qDebug()
 * 	<< AppImageUpdateInformation::errorCodeToString(AppImageUpdateInformation::APPIMAGE_NOT_FOUND);
*/
QString AppImageUpdateInformation::errorCodeToString(short errorCode)
{
    QString ret = "AppImageUpdateInformation::errorCode(";
    switch(errorCode) {
    case 0:
        ret += "APPIMAGE_NOT_READABLE)";
        break;
    case 1:
        ret += "NO_READ_PERMISSION)";
        break;
    case 2:
        ret += "APPIMAGE_NOT_FOUND)";
        break;
    case 3:
        ret += "CANNOT_OPEN_APPIMAGE)";
        break;
    case 4:
        ret += "EMPTY_UPDATE_INFORMATION)";
        break;
    case 5:
        ret += "INVALID_APPIMAGE_TYPE)";
        break;
    case 6:
        ret += "INVALID_MAGIC_BYTES)";
        break;
    case 7:
        ret += "INVALID_UPDATE_INFORMATION)";
        break;
    case 8:
        ret += "NOT_ENOUGH_MEMORY)";
        break;
    case 9:
        ret += "SECTION_HEADER_NOT_FOUND)";
        break;
    case 10:
        ret += "UNSUPPORTED_ELF_FORMAT)";
        break;
    case 11:
        ret += "UNSUPPORTED_TRANSPORT)";
        break;
    default:
        ret += "Unknown)";
        break;
    }
    return ret;
}
