#ifndef APPIMAGE_DELTA_WRITER_HPP_INCLUDED
#define APPIMAGE_DELTA_WRITER_HPP_INCLUDED
#include <QtCore>
#include <AppImageUpdateResource.hpp>

namespace AppImageUpdaterBridge {
	class ZsyncRemoteControlFileParserPrivate;
	
	class AppImageDeltaWriter : public QObject {
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
		APPIMAGE_UPDATE_RESOURCE_ERRORED	       
		} error_code;

		explicit AppImageDeltaWriter(AppImageUpdateResource*);
		~AppImageDeltaWriter();

		static QString errorCodeToString(short);
	public Q_SLOTS:
		AppImageDeltaWriter &checkForUpdate(void);
	private Q_SLOTS:
		void compareLocalAndRemoteAppImageSHA1Hash(QString);	
		void handleUpdateResourceError(short);
		void handleInfo(QJsonObject);
	Q_SIGNALS:
		void updateAvailable(bool , QString);
		void error(short);
		void progress(int);
		void logger(QString , QUrl);
	private:
		QString _sLocalAppImageSHA1Hash,
			_sLocalAppImagePath;
		AppImageUpdateResource *_pResource = nullptr;
		ZsyncRemoteControlFileParserPrivate *_pControlFileParser = nullptr;
	};
}

#endif // APPIMAGE_DELTA_WRITER_HPP_INCLUDED
