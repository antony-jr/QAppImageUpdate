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
	struct ZsyncCoreJobInfo {
		size_t blockSize = 0;
		zs_blockid blockIdOffset = 0;
	       	size_t nblocks = 0; 
		qint32 weakCheckSumBytes = 0;
	       	qint32 strongCheckSumBytes = 0;
	       	qint32 seqMatches = 0;
		QBuffer *checkSumBlocks = nullptr;
	       	QFile *targetFile = nullptr;
		QString seedFilePath;

		ZsyncCoreJobInfo(size_t bs , zs_blockid bio , size_t nb , qint32 wcksum , qint32 scksum , 
				qint32 sm , QBuffer *p , QFile *tf, const QString &sfp)
		{
			blockSize = bs;
			blockIdOffset = bio;
			nblocks = nb;
			weakCheckSumBytes = wcksum;
			strongCheckSumBytes = scksum;
			seqMatches = sm;
			checkSumBlocks = p;
			targetFile = tf;
			seedFilePath = sfp;
			return;
		}

	};


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

	} status_code;

	explicit ZsyncWriterPrivate(void);
	~ZsyncWriterPrivate();

	static ZsyncCoreJobPrivate::Result startJob(const ZsyncCoreJobInfo&);
public Q_SLOTS:
        void clear(void);
	void getBlockRanges(void);
	void writeBlockRanges(const QPair<qint32 , qint32>& , QByteArray*);
        void setConfiguration(size_t , size_t , qint32 , 
			      qint32 , qint32 , qint32 , qint32 , 
			      QBuffer* , const QString& , const QString& ,
			      const QString &targetFileOutputDirectory = QString());
	bool isPaused  (void) const;	
	bool isStarted (void) const;
	bool isRunning (void) const;
	bool isCanceled(void) const;
	bool isFinished(void) const;

	void start(void);

private Q_SLOTS:
	void handleFinished(void);

Q_SIGNALS:
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
	QScopedPointer<QList<ZsyncCoreJobInfo>> _pZsyncCoreJobInfos;
	QScopedPointer<QVector<QPair<QPair<zs_blockid , zs_blockid> , QVector<QByteArray>>>> _pResults;
	QFutureWatcher<ZsyncCoreJobPrivate::Result> *_pWatcher = nullptr;
	QTemporaryFile *_pTargetFile = nullptr;
	QString _sSourceFilePath,
		_sTargetFileName,
		_sOutputDirectory;
};
}
#endif // ZSYNC_WRITER_PRIVATE_HPP_INCLUDED
