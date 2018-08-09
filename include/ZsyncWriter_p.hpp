/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018, Antony jr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @filename    : ZsyncWriter_p.hpp
 * @description : This is where the core of the delta writer is described.
*/
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
	IDLE = 1,
	WRITTING_DOWNLOADED_BLOCK_RANGES = 100,
	EMITTING_REQUIRED_BLOCK_RANGES,
	SEARCHING_FOR_RANGE_CHECKSUMS,
	CHECKING_CHECKSUMS_FOR_DOWNLOADED_BLOCK_RANGES,
	WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE,
	ANALYZING_RESULTS_FOR_CONSTRUCTING_TARGET_FILE,
	CALCULATING_TARGET_FILE_SHA1_HASH,
	CONSTRUCTING_TARGET_FILE
	} status_code;

	explicit ZsyncWriterPrivate(void);
	~ZsyncWriterPrivate();

	static QString errorCodeToString(short);
	static QString statusCodeToString(short);
	static ZsyncCoreJobPrivate::Result startJob(const ZsyncCoreJobPrivate::Information&);
public Q_SLOTS:
        void clear(void);
#ifndef LOGGING_DISABLED
    	void setShowLog(bool);
    	void setLoggerName(const QString&);
#endif // LOGGING_DISABLED 
	void getBlockRanges(void);
	void writeBlockRanges(const QPair<qint32 , qint32>& , QByteArray*);
        void setConfiguration(size_t , size_t , qint32, qint32, qint32, qint32,
                          const QString& , const QString&, const QString& , const QVector<ZsyncCoreJobPrivate::Information>&);
	bool isPaused  (void) const;	
	bool isStarted (void) const;
	bool isRunning (void) const;
	bool isCanceled(void) const;
	bool isFinished(void) const;

	void start(void);
	void cancel(void);
	void pause(void);
	void resume(void);

private Q_SLOTS:
#ifndef LOGGING_DISABLED
    	void handleLogMessage(QString , QString);
#endif // LOGGING_DISABLED
	bool verifyAndConstructTargetFile(void);
	void handleFinished(void);

Q_SIGNALS:
        void finishedConfiguring();
	void blockRange(QPair<qint32 , qint32>);
	void endOfBlockRanges(void);
	void blockRangesWritten(QPair<qint32 , qint32> , bool);
	void started(void);
	void canceled(void);
	void finished(bool);
	void paused(void);
	void resumed(void);
	void progress(int);
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
		_sTargetFileSHA1,
		_sOutputDirectory;
	QScopedPointer<QVector<QPair<QPair<zs_blockid , zs_blockid> , QVector<QByteArray>>>> _pResults;
	QFutureWatcher<ZsyncCoreJobPrivate::Result> *_pWatcher = nullptr;
	QTemporaryFile *_pTargetFile = nullptr;
#ifndef LOGGING_DISABLED
	QString _sLogBuffer,
	    	_sLoggerName;
    	QScopedPointer<QDebug> _pLogger;
#endif // LOGGING_DISABLED 
};
}
#endif // ZSYNC_WRITER_PRIVATE_HPP_INCLUDED
