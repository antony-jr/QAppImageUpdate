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
 * @filename    : ZsyncCoreJob_p.hpp
 * @description : This is where the core of the Zsync Algorithm is described.
*/
#ifndef ZSYNC_CORE_JOB_PRIVATE_HPP_INCLUDED
#define ZSYNC_CORE_JOB_PRIVATE_HPP_INCLUDED
#include <QtCore>
#include <ZsyncInternalStructures_p.hpp>

namespace AppImageUpdaterBridge
{
class ZsyncCoreJobPrivate
{
public:

    enum : short {
        NO_ERROR = 0,
        HASH_TABLE_NOT_ALLOCATED = 100,
        INVALID_TARGET_FILE_CHECKSUM_BLOCKS,
        CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS,
        QBUFFER_IO_READ_ERROR,
        SOURCE_FILE_NOT_FOUND,
        NO_PERMISSION_TO_READ_SOURCE_FILE,
        CANNOT_OPEN_SOURCE_FILE
    } error_code;

    struct Information {
        size_t blockSize = 0, blocks = 0;
        zs_blockid blockIdOffset = 0;
        qint32 weakCheckSumBytes = 0,
               strongCheckSumBytes = 0,
               seqMatches = 0;
        QBuffer *checkSumBlocks = nullptr;
        QFile *targetFile = nullptr;
        QString seedFilePath;

        Information(void)
        {
            return;
        }

        Information(size_t bs, zs_blockid offset, size_t nb,
                    qint32 weakCksumb, qint32 strongCksumb, qint32 seq,
                    QBuffer *checksums, QFile *tfile, const QString &sfilePath )
        {
            blockSize = bs;
            blockIdOffset = offset;
            blocks = nb;
            weakCheckSumBytes = weakCksumb;
            strongCheckSumBytes = strongCksumb;
            seqMatches = seq;
            checkSumBlocks = checksums;
            targetFile = tfile;
            seedFilePath = sfilePath;
            return;
        }

    };

    struct Result {
        short errorCode = 0;
        qint32 gotBlocks = 0;
        QVector<QPair<QPair<zs_blockid, zs_blockid>, QVector<QByteArray>>> *requiredRanges = nullptr;
    };

    explicit ZsyncCoreJobPrivate(const Information&);
    ~ZsyncCoreJobPrivate();

    Result operator () (void);
private:
    void addToRanges(zs_blockid);
    qint32 alreadyGotBlock(zs_blockid);
    qint32 buildHash(void);
    qint32 checkCheckSumsOnHashChain(const hash_entry *, const unsigned char *, qint32 );
    quint32 calcRHash(const hash_entry *const);
    void calcMd4Checksum(unsigned char *, const unsigned char*,size_t);
    QVector<QPair<QPair<zs_blockid, zs_blockid>, QVector<QByteArray>>> *getRequiredRanges(void);
    zs_blockid getHashEntryBlockId(const hash_entry *);
    short tryOpenSeedFile(QFile**);
    short parseTargetFileCheckSumBlocks(void);
    void writeBlocks(const unsigned char *, zs_blockid, zs_blockid);
    void removeBlockFromHash(zs_blockid);
    qint32 submitSourceData(unsigned char*, size_t, off_t);
    qint32 submitSourceFile(QFile*);
    qint32 rangeBeforeBlock(zs_blockid);
    zs_blockid nextKnownBlock(zs_blockid);

    QPair<rsum, rsum> _pCurrentWeakCheckSums = qMakePair(rsum({ 0, 0 }), rsum({ 0, 0 }));
    size_t _nBlocks = 0, _nBlockSize = 0;
    qint32 _nBlockShift = 0, // log2(blocksize).
           _nContext = 0,    // precalculated blocksize * seq_matches.
           _nWeakCheckSumBytes = 0,
           _nStrongCheckSumBytes = 0, // # of bytes available for the strong checksum.
           _nSeqMatches = 0,
           _nSkip = 0,     // skip forward on next submit_source_data.
           _nBlockIdOffset = 0;
    unsigned short _pWeakCheckSumMask = 0; // This will be applied to the first 16 bits of the weak checksum.

    const hash_entry *_pRover = nullptr,
                      *_pNextMatch = nullptr;
    zs_blockid _nNextKnown = 0;

    /* Hash table for rsync algorithm */
    quint32 _pHashMask = 0;
    hash_entry *_pBlockHashes = nullptr;
    hash_entry **_pRsumHash = nullptr;

    /* And a 1-bit per rsum value table to allow fast negative lookups for hash
     * values that don't occur in the target file. */
    quint32 _pBitHashMask = 0;
    unsigned char *_pBitHash = nullptr;

    qint32 _nRanges = 0;
    zs_blockid *_pRanges = nullptr; // Ranges needed to finish the under construction target file.
    QBuffer *_pTargetFileCheckSumBlocks = nullptr; // Checksum blocks that needs to be loaded into the memory.
    QFile *_pTargetFile = nullptr; // Under construction target file.
    QCryptographicHash *_pMd4Ctx = nullptr; // Md4 Hasher context.
    QString _sSeedFilePath;
};
}
#endif // ZSYNC_CORE_JOB_PRIVATE_HPP_INCLUDED