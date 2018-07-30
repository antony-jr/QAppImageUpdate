#ifndef APPIMAGE_INSPECTOR_HPP_INCLUDED
#define APPIMAGE_INSPECTOR_HPP_INCLUDED
#include <QtCore>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <AppImageUpdateInformation.hpp>

namespace AppImageUpdaterBridge {
	class AppImageInspector : public QObject {
		Q_OBJECT
	public:
	    enum : short {
            UNKNOWN_NETWORK_ERROR,
            IO_READ_ERROR,
            ERROR_RESPONSE_CODE,
            NO_MARKER_FOUND_IN_CONTROL_FILE,
            INVALID_ZSYNC_HEADERS_NUMBER,
            INVALID_ZSYNC_MAKE_VERSION,
            INVALID_ZSYNC_TARGET_FILENAME,
            INVALID_ZSYNC_MTIME,
            INVALID_ZSYNC_BLOCKSIZE,
            INVALID_TARGET_FILE_LENGTH,
            INVALID_HASH_LENGTH_LINE,
            INVALID_HASH_LENGTHS,
            INVALID_TARGET_FILE_URL,
            INVALID_TARGET_FILE_SHA1,
	    APPIMAGE_UPDATE_INFORMATION_ERRORED
    	    } error_code;

	    explicit AppImageInspector(AppImageUpdateInformation * ,
			    	       QNetworkAccessManager *networkManager = nullptr);
	    ~AppImageInspector();

	    static QString errorCodeToString(short);

	public Q_SLOTS:
	    bool isEmpty(void);
	    bool isUpdatesAvailable(void);
	    AppImageInspector &clear(void);
	    AppImageInspector &setShowLog(bool);
	    AppImageInspector &checkForUpdates(void);
	    AppImageInspector &getTargetFileCheckSumBlocks(void);
	    size_t getTargetFileBlocksCount(void);
    	    QUrl getControlFileUrl(void);
    	    QString getZsyncMakeVersion(void);
    	    QString getTargetFileName(void);
    	    QUrl getTargetFileUrl(void);
    	    QString getTargetFileSHA1(void);
    	    QDateTime getMTime(void);
    	    size_t getTargetFileBlockSize(void);
    	    size_t getTargetFileLength(void);
    	    qint32 getWeakCheckSumBytes(void);
    	    qint32 getStrongCheckSumBytes(void);
    	    qint32 getConsecutiveMatchNeeded(void);

	private Q_SLOTS:
	    void handleUpdateInformationError(short);
	    void handleControlFile(void);

        Q_SIGNALS:
	    void targetFileCheckSumBlock(Private::zs_blockid , Private::rsum , void*);
	    void endOfTargetFileCheckSumBlocks(void);
	    void updatesAvailable(bool);
	    void progress(int);
	    void error(short);
	    void logger(QString , QUrl); /* log msg , control file url. */
	
	private:
	    QSharedPointer<Private::ZsyncRemoteControlFileParserPrivate> _pControlFileParser = nullptr;
	    QSharedPointer<QThread> _pControlFileParserThread = nullptr;
	    AppImageUpdateInformation *_pUpdateInformation = nullptr;
	};
}

#endif // APPIMAGE_INSPECTOR_HPP_INCLUDED
