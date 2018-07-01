#ifndef ZSYNC_ROLLING_CHECKSUM_HPP_INCLUDED
#define ZSYNC_ROLLING_CHECKSUM_HPP_INCLUDED
#include <QFuture>
#include <QtConcurrentMap>
#include <QtCore>
#include <ZsyncInternalStructures.hpp>

namespace AppImageUpdaterBridge
{
class ZsyncRollingChecksum : public QObject
{
    Q_OBJECT
public:
    explicit ZsyncRollingChecksum(QObject *parent = nullptr);
    void setConfiguration(quint64 nblocks,
                          quint64 blocksize,
                          unsigned int rsum_bytes,
                          unsigned int checksum_bytes,
                          unsigned int seq_matches,
                          const QString &filename);

    ~ZsyncRollingChecksum();

public Q_SLOTS:
    quint64 submitSourceFile(QFile *file);
    QVector<QPair<zs_blockid, zs_blockid>> neededBlockRanges(zs_blockid from, zs_blockid to);
    void addTargetBlock(zs_blockid b, rsum r, void *checksum);
private Q_SLOTS:
    rsum __attribute__((pure)) calculateRollingChecksum(const unsigned char *data, quint64 len);
    void calculateStrongChecksum(unsigned char *buffer, const unsigned char *data, quint64 len);
    void writeBlocks(const unsigned char *data,  zs_blockid bfrom, zs_blockid bto);
    qint64 readKnownData(unsigned char *buffer, off_t offset, quint64 len);
    void removeBlockFromHash(zs_blockid id);
    void addToRanges(zs_blockid id);
    zs_blockid nextKnownBlock(zs_blockid x);
    quint64 calcRHash(const hash_entry *const e);
    qint64 rangeBeforeBlock(zs_blockid x);
    quint64 checkChecksumOnHashChain(const hash_entry *e, const unsigned char *data, int onlyone);
    int buildHash(void);

    qint64 submitBlocks(const unsigned char *data, zs_blockid bfrom, zs_blockid bto);
    quint64 submitSourceData(unsigned char *data, quint64 len, off_t offset);

    zs_blockid getHashEntryBlockID(const hash_entry *e);
private:
    QMutex _pMutex;
    rsum _pCurrentSums[2] = { {0, 0}, {0, 0} };    /* current rsums */
    zs_blockid _nBlocks = 0; /* number of blocks in the target file */
    quint64 _nBlockSize = 0; /* bytes per block */
    int _nBlockShift = 0; /* log2(blocksize) */
    unsigned short _nRsumMaskA = 0;
    unsigned short _nRsumBits = 0;
    unsigned short _nRsumBytes = 0;
    unsigned _nHashFuncShift = 0; /* config for the hash function */
    unsigned int _nChecksumBytes = 0; /* length of the MD4 Checksum available */
    int _nSeqMatches = 0;
    quint64 _nContext = 0; /* precalculated blocksize * seq_matches */

    qint64 _nSkip;
    const hash_entry *_pRover = nullptr;

    const hash_entry *_pNextMatch = nullptr;
    zs_blockid _nNextKnown = 0;

    unsigned int _nHashMask;
    QSharedPointer<hash_entry> _pBlockHashes;
    hash_entry **_pRsumHash = nullptr;

    unsigned char *_cBitHash = nullptr;
    unsigned int _nBitHashMask = 0;

    qint64 _nNumRanges = 0;
    QSharedPointer<zs_blockid> _pRanges;
    quint64 _nGotBlocks = 0;

    QSharedPointer<QFile> _pTargetFile;
    /* Stats */
    quint64 _nHashHit = 0,
            _nWeakHit = 0,
            _nStrongHit = 0,
            _nCheckSummed = 0;
};
}
#endif // ZSYNC_ROLLING_CHECKSUM_HPP_INCLUDED
