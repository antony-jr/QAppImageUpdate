#ifndef APPIMAGE_INSPECTOR_HPP_INCLUDED
#define APPIMAGE_INSPECTOR_HPP_INCLUDED
#include <QtCore>
#include <AppImageUpdateInformation.hpp>
#include <ZsyncInternalStructures_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>

namespace AppImageUpdaterBridge
{
class AppImageInspector : public QObject
{
    Q_OBJECT
public:
    enum : short {
        /* Low level error codes from ZsyncRemoteControlFileParser. */
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

    explicit AppImageInspector(QNetworkAccessManager *networkManager = nullptr);
    explicit AppImageInspector(AppImageUpdateInformation *, QNetworkAccessManager *networkManager = nullptr);
    AppImageInspector &setShowLog(bool);
    AppImageInspector &setUpdateInformation(AppImageUpdateInformation *);
    AppImageInspector &clear(void);
    ~AppImageInspector();
public Q_SLOTS:
    AppImageInspector &start(void);
    AppImageInspector &cancel(void);
    AppImageInspector &pause(void);
    AppImageInspector &resume(void);
    AppImageInspector &waitForFinished(void);

    bool isStarted() const;
    bool isRunning() const;
    bool isCanceled() const;
    bool isFinished() const;
    bool isUpdatesAvailable() const;

    void getTargetFileCheckSumBlocks(void);
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
    void handleLogMessage(QString, QString);

Q_SIGNALS:
    void targetFileCheckSumBlock(Private::zs_blockid, Private::rsum, void*);
    void endOfTargetFileCheckSumBlocks(void);
    void started(void);
    void canceled(void);
    void paused(void);
    void resumed(void);
    void finished(bool);
    void progress(int);
    void error(short);
    void logger(QString, QString);

private:
    bool updatesAvailable = false;
    QString _sAppImagePath,
            _sLogBuffer;
    QSharedPointer<AppImageUpdateInformation> _pUpdateInformation = nullptr;
    QSharedPointer<Private::ZsyncRemoteControlFileParserPrivate> _pControlFileParser = nullptr;
    QSharedPointer<QThread> _pControlFileParserThread = nullptr;
};
}

#endif // APPIMAGE_INPSECTOR_HPP_INCLUDED
