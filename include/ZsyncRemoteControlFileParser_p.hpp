#ifndef ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
#define ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
#include <QtCore>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <ZsyncInternalStructures_p.hpp>
#include <ZsyncCoreJob_p.hpp>

namespace AppImageUpdaterBridge
{
class ZsyncRemoteControlFileParserPrivate : public QObject
{
    Q_OBJECT
public:
    enum : short {
        UNKNOWN_NETWORK_ERROR = 50,
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

    explicit ZsyncRemoteControlFileParserPrivate(QNetworkAccessManager*);
    ~ZsyncRemoteControlFileParserPrivate();

    static QString errorCodeToString(short);
    static QString statusCodeToString(short);
public Q_SLOTS:
    bool isEmpty(void);
    void clear(void);
    void setControlFileUrl(const QUrl&);
    void setControlFileUrl(QJsonObject);
    void setLoggerName(const QString&);
    void setShowLog(bool);
    void getControlFile(void);
    void getTargetFileBlocks(void);
    void getUpdateCheckInformation(void);
    void getZsyncInformation(void);
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
    void handleBintrayRedirection(const QUrl&);
    void handleGithubAPIResponse(void);
    void handleDownloadProgress(qint64, qint64);
    void handleControlFile(void);
    void handleNetworkError(QNetworkReply::NetworkError);
    void handleErrorSignal(short);
    void handleLogMessage(QString , QUrl);
Q_SIGNALS:
    void zsyncInformation(size_t , size_t , qint32 , qint32 ,
                          qint32, qint32 ,QString,QString,QVector<ZsyncCoreJobPrivate::Information>);
    void updateCheckInformation(QJsonObject);
    void receiveTargetFileBlocks(zs_blockid, rsum, void*);
    void endOfTargetFileBlocks(void);
    void receiveControlFile(void);
    void progress(int);
    void error(short);
    void statusChanged(short);
#ifndef LOGGING_DISABLED
    void logger(QString , QUrl);
#endif // LOGGING_DISABLED
private:
    QJsonObject _jUpdateInformation;
    QString _sZsyncMakeVersion,
	    _sZsyncFileName, //Only used for github and bintray api responses.
            _sTargetFileName,
            _sTargetFileSHA1,
#ifndef LOGGING_DISABLED
            _sLoggerName,
            _sLogBuffer;
#endif // LOGGING_DISABLED
    QDateTime _pMTime;
    size_t _nTargetFileBlockSize = 0,
           _nTargetFileLength = 0,
           _nTargetFileBlocks = 0;
    qint32 _nWeakCheckSumBytes = 0,
           _nStrongCheckSumBytes = 0,
           _nConsecutiveMatchNeeded = 0;
    qint64 _nCheckSumBlocksOffset = 0;
    QUrl _uTargetFileUrl,
         _uControlFileUrl;

#ifndef LOGGING_DISABLED
    QSharedPointer<QDebug> _pLogger = nullptr;
#endif // LOGGING_DISABLED
    QSharedPointer<QBuffer> _pControlFile = nullptr;
    QNetworkAccessManager *_pNManager = nullptr;
};
}

#endif //ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
