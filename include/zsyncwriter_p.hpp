/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018-2019, Antony jr
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
 * @filename    : zsyncwriter_p.hpp
 * @description : This is where the core of the delta writer is described.
*/
#ifndef ZSYNC_WRITER_PRIVATE_HPP_INCLUDED
#define ZSYNC_WRITER_PRIVATE_HPP_INCLUDED
#include <QBuffer>
#include <cmath>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QtEndian>
#include <QFileInfo>
#include <QtGlobal>
#include <QJsonObject>
#include <QObject>
#include <QUrl>
#include <QString>
#include <QScopedPointer>
#include <QTime>
#include <QTimer>
#include <QTemporaryFile>

#include "appimageupdaterbridge_enums.hpp"
#include "zsyncinternalstructures_p.hpp"

namespace AppImageUpdaterBridge
{
class ZsyncWriterPrivate : public QObject
{
    Q_OBJECT
public:
    explicit ZsyncWriterPrivate();
    ~ZsyncWriterPrivate();
public Q_SLOTS:
    void setShowLog(bool);
    void setLoggerName(const QString&);
    void setOutputDirectory(const QString&);
    void setConfiguration(qint32,qint32,qint32,
                          qint32,qint32,qint32,
                          const QString&,const QString&,const QString&,
                          QUrl, QBuffer*,bool);
    void start(void);
    void cancel(void);


    /* Used by the block range downloader. */
    void getBlockRanges();
    void writeSeqRaw(QByteArray*);
    void writeBlockRanges(qint32, qint32, QByteArray*);

private Q_SLOTS:
#ifndef LOGGING_DISABLED
    void handleLogMessage(QString, QString);
#endif // LOGGING_DISABLED
    bool verifyAndConstructTargetFile(void);
    void addToRanges(zs_blockid);
    qint32 alreadyGotBlock(zs_blockid);
    qint32 buildHash(void);
    qint32 checkCheckSumsOnHashChain(const hash_entry *, const unsigned char *, qint32 );
    quint32 calcRHash(const hash_entry *const);
    void calcMd4Checksum(unsigned char *, const unsigned char*,size_t);
    zs_blockid getHashEntryBlockId(const hash_entry *);
    short tryOpenSourceFile(const QString&, QFile**);
    short parseTargetFileCheckSumBlocks(void);
    void writeBlocks(const unsigned char *, zs_blockid, zs_blockid);
    void removeBlockFromHash(zs_blockid);
    qint32 submitSourceData(unsigned char*, size_t, off_t);
    qint32 submitSourceFile(QFile*);
    qint32 rangeBeforeBlock(zs_blockid);
    zs_blockid nextKnownBlock(zs_blockid);

Q_SIGNALS:
    void initStart();
    void initCancel();
    void finishedConfiguring();
    void blockRange(qint32, qint32);
    void endOfBlockRanges();
    void download(qint64, qint64, QUrl);
    void started();
    void canceled();
    void finished(QJsonObject, QString);
    void progress(int percentage, qint64 bytesReceived, qint64 bytesTotal, double speed, QString units);
    void statusChanged(short);
    void error(short);
    void logger(QString, QString);
private:
    bool b_Started = false,
         b_CancelRequested = false,
         b_AcceptRange = true;
    QUrl u_TargetFileUrl;
    QPair<rsum, rsum> p_CurrentWeakCheckSums = qMakePair(rsum({ 0, 0 }), rsum({ 0, 0 }));
    qint64 n_BytesWritten = 0;
    qint32 n_Blocks = 0,
           n_BlockSize = 0,
           n_BlockShift = 0, /* log2(blocksize). */
           n_Context = 0,    /* precalculated blocksize * seq_matches */
           n_WeakCheckSumBytes = 0,
           n_StrongCheckSumBytes = 0, /* no. of bytes available for the strong checksum. */
           n_SeqMatches = 0,
           n_Skip = 0,    /* skip forward on next submit_source_data. */
           n_TargetFileLength = 0;
    unsigned short p_WeakCheckSumMask = 0; /* This will be applied to the first 16 bits of the weak checksum. */

    const hash_entry *p_Rover = nullptr,
                      *p_NextMatch = nullptr;
    zs_blockid n_NextKnown = 0;

    /* Hash table for rsync algorithm */
    quint32 p_HashMask = 0;
    hash_entry *p_BlockHashes = nullptr;
    hash_entry **p_RsumHash = nullptr;

    /* And a 1-bit per rsum value table to allow fast negative lookups for hash
     * values that don't occur in the target file. */
    quint32 p_BitHashMask = 0;
    unsigned char *p_BitHash = nullptr;

    qint32 n_Ranges = 0;
    zs_blockid *p_Ranges = nullptr; /* Ranges needed to finish the under construction target file. */
    QVector<QPair<qint32, qint32>> p_RequiredRanges;
    QScopedPointer<QBuffer> p_TargetFileCheckSumBlocks; /* Checksum blocks that needs to be loaded into the memory.*/
    QScopedPointer<QCryptographicHash> p_Md4Ctx; /* Md4 Hasher context.*/
    QString s_SourceFilePath,
            s_TargetFileName,
            s_TargetFileSHA1,
            s_OutputDirectory;
    QScopedPointer<QTemporaryFile> p_TargetFile; /* under construction target file. */
    QScopedPointer<QTime> p_TransferSpeed;
#ifndef LOGGING_DISABLED
    QString s_LogBuffer,
            s_LoggerName;
    QScopedPointer<QDebug> p_Logger;
#endif // LOGGING_DISABLED 
};
}
#endif // ZSYNC_WRITER_PRIVATE_HPP_INCLUDED
