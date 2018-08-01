#ifndef APPIMAGE_DELTA_WRITER_HPP_INCLUDED
#define APPIMAGE_DELTA_WRITER_HPP_INCLUDED
#include <QtCore>
#include <QNetworkAccessManager>

namespace AppImageUpdaterBridge {
	class AppImageUpdateInformationPrivate;
	class ZsyncRemoteControlFileParserPrivate;

	class AppImageDeltaWriter : public QObject {
		Q_OBJECT
	public:
		enum : short {
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
        	INVALID_TARGET_FILE_SHA1
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

		explicit AppImageDeltaWriter(QObject *parent = nullptr);
		explicit AppImageDeltaWriter(const QString& , QObject *parent = nullptr);
		explicit AppImageDeltaWriter(QFile * , QObject *parent = nullptr);
		~AppImageDeltaWriter();

		static QString errorCodeToString(short);
		static QString statusCodeToString(short);
	
	public Q_SLOTS:
		AppImageDeltaWriter &setAppImage(const QString&);
		AppImageDeltaWriter &setAppImage(QFile*);
		AppImageDeltaWriter &setShowLog(bool);
		AppImageDeltaWriter &getAppImageEmbededInformation(void);
		AppImageDeltaWriter &clear(void);
		AppImageDeltaWriter &checkForUpdate(void);
		QThread *sharedQThread(void) const;
		QNetworkAccessManager *sharedNetworkAccessManager(void) const;

	private Q_SLOTS:
		void compareLocalAndRemoteAppImageSHA1Hash(QString);	
		void handleInfo(QJsonObject);
	Q_SIGNALS:
		void embededInformation(QJsonObject);
		void updateAvailable(bool , QString);
		void statusChanged(short);
		void error(short);
		void progress(int);
		void logger(QString , QUrl);
	private:
		QString _sLocalAppImageSHA1Hash,
			_sLocalAppImagePath;
		AppImageUpdateInformationPrivate *_pUpdateInformation = nullptr;
		ZsyncRemoteControlFileParserPrivate *_pControlFileParser = nullptr;
		QThread *_pSharedThread = nullptr;
		QNetworkAccessManager *_pSharedNetworkAccessManager = nullptr;
	};
}

#endif // APPIMAGE_DELTA_WRITER_HPP_INCLUDED
