#ifndef APPIMAGE_DELTA_WRITER_HPP_INCLUDED
#define APPIMAGE_DELTA_WRITER_HPP_INCLUDED
#include <QtCore>
#include <QtConcurrentMap>
#include <QFuture>
#include <QFutureWatcher>
#include <QNetworkAccessManager>
#include <ZsyncCoreJob_p.hpp>

namespace AppImageUpdaterBridge {
	class AppImageUpdateInformationPrivate;
	class ZsyncRemoteControlFileParserPrivate;
	
	class AppImageDeltaWriter : public QObject {
		Q_OBJECT
	public:
		enum : short {
		NO_ERROR = -1,
		APPIMAGE_NOT_READABLE = 0, //  50< and >=0 , AppImage Update Information Error.
        	NO_READ_PERMISSION,
        	APPIMAGE_NOT_FOUND,
        	CANNOT_OPEN_APPIMAGE,
        	EMPTY_UPDATE_INFORMATION,
        	INVALID_APPIMAGE_TYPE,
        	INVALID_MAGIC_BYTES,
        	INVALID_UPDATE_INFORMATION,
        	NOT_ENOUGH_MEMORY,
        	SECTION_HEADER_NOT_FOUND,
        	UNSUPPORTED_ELF_FORMAT,
        	UNSUPPORTED_TRANSPORT,
		UNKNOWN_NETWORK_ERROR = 50, // >= 50 , Zsync Control File Parser Error.
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
        	HASH_TABLE_NOT_ALLOCATED = 100, // >= 100 , Zsync Core Job error.
        	INVALID_TARGET_FILE_CHECKSUM_BLOCKS,
        	CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS,
        	QBUFFER_IO_READ_ERROR,
        	SOURCE_FILE_NOT_FOUND,
        	NO_PERMISSION_TO_READ_SOURCE_FILE,
        	CANNOT_OPEN_SOURCE_FILE,
		NO_PERMISSION_TO_READ_WRITE_TARGET_FILE,
		CANNOT_OPEN_TARGET_FILE,
		TARGET_FILE_SHA1_HASH_MISMATCH
		} error_code;

		enum : short {
		INITIALIZING = 0,
		IDLE = 1,
		OPENING_APPIMAGE,
		CALCULATING_APPIMAGE_SHA1_HASH,
		READING_APPIMAGE_MAGIC_BYTES,
		FINDING_APPIMAGE_ARCHITECTURE,
		MAPPING_APPIMAGE_TO_MEMORY,
		SEARCHING_FOR_UPDATE_INFORMATION_SECTION_HEADER,
		UNMAPPING_APPIMAGE_FROM_MEMORY,
		FINALIZING_APPIMAGE_EMBEDED_UPDATE_INFORMATION,
		PARSING_APPIMAGE_EMBEDED_UPDATE_INFORMATION = 50,
		REQUESTING_GITHUB_API,
		PARSING_GITHUB_API_RESPONSE,
		REQUESTING_ZSYNC_CONTROL_FILE,
		REQUESTING_BINTRAY,
		PARSING_BINTRAY_REDIRECTED_URL_FOR_LATEST_PACKAGE_URL,
		PARSING_ZSYNC_CONTROL_FILE,
		SEARCHING_TARGET_FILE_CHECKSUM_BLOCK_OFFSET_IN_ZSYNC_CONTROL_FILE,
		STORING_ZSYNC_CONTROL_FILE_DATA_TO_MEMORY,
		FINALIZING_PARSING_ZSYNC_CONTROL_FILE,
		EMITTING_TARGET_FILE_CHECKSUM_BLOCKS,
		FINALIZING_TRANSMISSION_OF_TARGET_FILE_CHECKSUM_BLOCKS
		} status_code;

		explicit AppImageDeltaWriter(bool singleThreaded = false , QObject *parent = nullptr);
		explicit AppImageDeltaWriter(const QString& , bool singleThreaded = false , QObject *parent = nullptr);
		explicit AppImageDeltaWriter(QFile * , bool singleThreaded = false , QObject *parent = nullptr);
		~AppImageDeltaWriter();

		static QString errorCodeToString(short);
		static QString statusCodeToString(short);
	
	public Q_SLOTS:
		AppImageDeltaWriter &start(void);
		AppImageDeltaWriter &cancel(void);
		AppImageDeltaWriter &pause(void);
		AppImageDeltaWriter &resume(void);
		AppImageDeltaWriter &waitForFinished(void);
		
		bool isCanceled(void) const;
		bool isFinished(void) const;
		bool isPaused(void) const;
		bool isRunning(void) const;
		bool isStarted(void) const;

		AppImageDeltaWriter &setAppImage(const QString&);
		AppImageDeltaWriter &setAppImage(QFile*);
		AppImageDeltaWriter &setShowLog(bool);
		AppImageDeltaWriter &getAppImageEmbededInformation(void);
		AppImageDeltaWriter &checkForUpdate(void);
		AppImageDeltaWriter &verifyAndConstructTargetFile(void);
		AppImageDeltaWriter &clear(void);
		QThread *sharedQThread(void) const;
		QNetworkAccessManager *sharedNetworkAccessManager(void) const;

	private Q_SLOTS:
		static ZsyncCoreJobPrivate::JobResult startJob(const ZsyncCoreJobPrivate::JobInformation&);
		void handleFinished(void);
		void handleUpdateAvailable(bool , QString);
		void handleUpdateCheckInformation(QJsonObject);
	Q_SIGNALS:
		void started(void);
		void canceled(void);
		void paused(void);
		void resumed(void);
		void finished(void);

		void verifiedAndConstructedTargetFile(void);
		void blockDownloaderInformation(QHash<qint32 , QByteArray>*,
						QVector<QPair<qint32 , qint32>>*,
						QTemporaryFile *);
		void embededInformation(QJsonObject);
		void updateAvailable(bool , QString);
		void statusChanged(short);
		void error(short);
		void progress(int);
		void logger(QString , QUrl);
	private:
		QList<ZsyncCoreJobPrivate::JobInformation> jobs;
		QScopedPointer<QHash<qint32 , QByteArray>> _pBlockHashes;
		QScopedPointer<QVector<QPair<qint32 , qint32>>> _pRanges;
		QScopedPointer<QTemporaryFile> _pTargetFile;
		QScopedPointer<AppImageUpdateInformationPrivate> _pUpdateInformation;
		QScopedPointer<ZsyncRemoteControlFileParserPrivate> _pControlFileParser;
		QScopedPointer<QThread> _pSharedThread;
		QScopedPointer<QNetworkAccessManager> _pSharedNetworkAccessManager;
		QScopedPointer<QFuture<ZsyncCoreJobPrivate::JobResult>> _pFuture;
		QScopedPointer<QFutureWatcher<ZsyncCoreJobPrivate::JobResult>> _pWatcher;
	};
}

#endif // APPIMAGE_DELTA_WRITER_HPP_INCLUDED
