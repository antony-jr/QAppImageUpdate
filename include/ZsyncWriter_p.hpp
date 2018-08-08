#ifndef ZSYNC_WRITER_PRIVATE_HPP_INCLUDED
#define ZSYNC_WRITER_PRIVATE_HPP_INCLUDED
#include <QtCore>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <ZsyncCoreJob_p.hpp>

namespace AppImageUpdaterBridge {
class ZsyncWriterPrivate : public QObject {
	Q_OBJECT
public:
	enum : short {
	HASH_TABLE_NOT_ALLOCATED = 100,
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
	IDLE = 2,
	ERRORED = 3
	} status_code;

	explicit ZsyncWriterPrivate(void);
	~ZsyncWriterPrivate();

	static ZsyncCoreJobPrivate::Result startJob(const ZsyncCoreJobPrivate::Information&);
public Q_SLOTS:
        void clear(void);
	void getBlockRanges(void);
	void writeBlockRanges(const QPair<qint32 , qint32>& , QByteArray*);
        void setConfiguration(size_t , size_t , qint32, qint32, qint32, qint32,
                          const QString& , const QString&,const QVector<ZsyncCoreJobPrivate::Information>&);
	bool isPaused  (void) const;	
	bool isStarted (void) const;
	bool isRunning (void) const;
	bool isCanceled(void) const;
	bool isFinished(void) const;

	void start(void);

private Q_SLOTS:
	void handleFinished(void);

Q_SIGNALS:
        void finishedConfiguring();
	void blockRange(QSharedPointer<QPair<qint32 , qint32>>);
	void endOfBlockRanges(void);
	void blockRangesWritten(QPair<qint32 , qint32> , bool);
	void started(void);
	void finished(bool);
	void paused(void);
	void resumed(void);
	void statusChanged(short);
	void error(short);
	void logger(QString , QString);
private:
	bool isEmpty = true;
    size_t _nBlockSize = 0,
	       _nBlocks = 0;
	qint32 _nBlockShift = 0;
	qint32 _nWeakCheckSumBytes = 0,
	       _nStrongCheckSumBytes = 0,
	       _nSeqMatches = 0,
	       _nTargetFileLength = 0;
	QVector<ZsyncCoreJobPrivate::Information> _pZsyncCoreJobInfos;
	QString _sSourceFilePath,
		_sTargetFileName,
		_sOutputDirectory;
	QScopedPointer<QVector<QPair<QPair<zs_blockid , zs_blockid> , QVector<QByteArray>>>> _pResults;
	QFutureWatcher<ZsyncCoreJobPrivate::Result> *_pWatcher = nullptr;
	QTemporaryFile *_pTargetFile = nullptr;
};
}
#endif // ZSYNC_WRITER_PRIVATE_HPP_INCLUDED
