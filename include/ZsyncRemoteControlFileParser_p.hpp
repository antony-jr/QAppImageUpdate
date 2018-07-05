#ifndef ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
#define ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
#include <QtCore>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <ZsyncCoreWorker_p.hpp>

namespace AppImageUpdaterBridgePrivate {
class ZsyncRemoteControlFileParserPrivate : public QObject {
	Q_OBJECT
public:
	enum : short {
	    CANNOT_PROCESS_TARGET_BLOCKS,
	    NETWORK_TIMEOUT,
	    NO_INTERNET_CONNECTION,
        ERROR_RESPONSE_CODE,
        CONTROL_FILE_NOT_FOUND
	} error_code;

	explicit ZsyncRemoteControlFileParserPrivate(QNetworkAccessManager *NetworkManager = nullptr);
	explicit ZsyncRemoteControlFileParserPrivate(const QUrl& , QNetworkAccessManager *NetworkManager = nullptr);
	void setControlFileUrl(const QUrl&);
	void setShowLog(bool);
	void setTimeoutTime(qint64);
	~ZsyncRemoteControlFileParserPrivate();

public Q_SLOTS:
	void getTargetFileBlocks(void);
	const size_t &getTargetFileBlocksCount(void);
	const QUrl &getControlFileUrl(void);
	const QString &getZsyncMakeVersion(void);
	const QString &getTargetFileName(void);
	const QUrl &getTargetFileUrl(void);
	const QString &getTargetFileSHA1(void);
	const QDateTime &getMTime(void);
	const size_t &getTargetFileBlockSize(void);
	const size_t &getTargetFileLength(void);
	const qint32 &getWeakCheckSumBytes(void);
	const qint32 &getStrongCheckSumBytes(void);
	const qint32 &getConsecutiveMatchNeeded(void);
private Q_SLOTS:
	void sendTargetFileBlocks(qint64 , qint64);
    void sentAllTargetFileBlocks(void);
	void checkNetworkConnection(QNetworkAccessManager::NetworkAccessibility);
	void handleControlFileHeader(qint64 , qint64);
	void handleTimeout(void);
    void handleError(short);
	void handleNetworkError(QNetworkReply::NetworkError);
    void logPrinter(QString);
Q_SIGNALS:
	void handleTargetFileBlocks(zs_blockid , rsum , void*);
	void gotAllTargetFileBlocks(void);
	void progress(int);
	void error(short);
	void logger(QString);
private:
	QMutex _pMutex;
	bool _bSupportForRangeRequests = false,
         _bSafeToProceed = false;
    QByteArray _pZsyncHeader,
               _pBlockSumBuffer;
    zs_blockid _nCurrentBlockId = 0;
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
	qint64 _nTargetFileBlocksRangeStart = 0,
	       _nTargetFileBlocksRangeEnd = 0,
	       _nTimeoutTime = 10;
	QUrl _uTargetFileUrl,
	     _uControlFileUrl;

	QTimer _pTimeoutTimer;
	QNetworkRequest _pCurrentRequest;
	QNetworkReply *_pCurrentReply = nullptr;
	QSharedPointer<QNetworkAccessManager> _pNManager = nullptr;
	QSharedPointer<QDebug> _pLogger = nullptr;
};
}

#endif //ZSYNC_CONTROL_FILE_PARSER_PRIVATE_HPP_INCLUDED
