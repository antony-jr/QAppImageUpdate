#ifndef ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
#define ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
#include <QtCore>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <ZsyncInternalStructures_p.hpp>

namespace AppImageUpdaterBridgePrivate
{
class ZsyncRemoteControlFileParserPrivate : public QObject
{
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
        INVALID_TARGET_FILE_SHA1
    } error_code;

    explicit ZsyncRemoteControlFileParserPrivate(QNetworkAccessManager *NetworkManager = nullptr);
    explicit ZsyncRemoteControlFileParserPrivate(const QUrl&, QNetworkAccessManager *NetworkManager = nullptr);
    void setControlFileUrl(const QUrl&);
    void setShowLog(bool);
    void clear(void);
    ~ZsyncRemoteControlFileParserPrivate();

    /* static function to return a QString for corresponding error code.*/
    static QString errorCodeToString(short);
public Q_SLOTS:
    void getControlFile(void);
    void getTargetFileBlocks(void);
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
    void handleDownloadProgress(qint64, qint64);
    void handleControlFile(void);
    void handleNetworkError(QNetworkReply::NetworkError);
    void handleErrorSignal(short);
    void handleLogMessage(QString);
Q_SIGNALS:
    void receiveTargetFileBlocks(zs_blockid, rsum, void*);
    void endOfTargetFileBlocks(void);
    void receiveControlFile(void);
    void progress(int);
    void error(short);
    void logger(QString);
private:
    QMutex _pMutex;
    QString _sZsyncMakeVersion,
            _sTargetFileName,
            _sTargetFileSHA1,
            _sControlFileName,
            _sLogBuffer;
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

    QSharedPointer<QNetworkAccessManager> _pNManager = nullptr;
    QSharedPointer<QDebug> _pLogger = nullptr;
    QSharedPointer<QBuffer> _pControlFile = nullptr;
};
}

#endif //ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
