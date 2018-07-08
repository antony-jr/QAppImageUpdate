#ifndef ZSYNC_CORE_WORKER_PRIVATE_INCLUDED
#define ZSYNC_CORE_WORKER_PRIVATE_INCLUDED
#include <QtCore>
#include <ZsyncInternalStructures_p.hpp>
#include <ZsyncRemoteControlFileParser_p.hpp>

namespace AppImageUpdaterBridgePrivate
{
class ZsyncCorePrivate : public QObject
{
    Q_OBJECT
public:
    explicit ZsyncCorePrivate(QNetworkAccessManager *NetworkManager = nullptr);
    explicit ZsyncCorePrivate(const QUrl& , QNetworkAccessManager *NetworkManager = nullptr);
    void setControlFileUrl(const QUrl&);
    void setShowLog(bool);
    void clear();
    ~ZsyncCorePrivate();

public Q_SLOTS:
    qint32 submitSourceFiles(QFile*);

private Q_SLOTS:
    void addTargetFileChecksumBlocks(zs_blockid, rsum, void*);
    qint32 getNeededBlocksCount(void);
    qint32 submitTargetFileBlocks(const char*, zs_blockid, zs_blockid );
    qint32 submitSourceData(char*, size_t, off_t);
    qint32 checkCheckSumsOnHashChain(const hash_entry *, const char *, qint32 );
    void writeBlocks(const char *, zs_blockid, zs_blockid);
    zs_blockid getHashEntryBlockId(const hash_entry *);
    void addToRanges(zs_blockid);
    qint32 rangeBeforeBlock(zs_blockid);
    qint32 alreadyGotBlock(zs_blockid);
    zs_blockid nextKnownBlock(zs_blockid);
    unsigned calcRHash(const hash_entry *const);
    qint32 buildHash(void);
    void removeBlockFromHash(zs_blockid);

Q_SIGNALS:
    void receiveNeededBlockRanges(const QVector<QPair<zs_blockid, zs_blockid>>&);

private:
    QMutex _pMutex;
    QPair<rsum> _pCurrentWeakCheckSums = qMakePair({ 0 , 0 } , { 0 , 0 });
    size_t _nBlocks = 0 , 
	   _nBlockSize = 0;
    qint32 _nBlockShift = 0, // log2(blocksize).
	   _nContext = 0,    // precalculated blocksize * seq_matches.
	   _nSkip = 0,       // skip forward on next submit_source_data.
	   _nGotBlocks = 0;  // # of blocks that we currently have in the under construction target file.
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

    QVector<QPair<zs_blockid , zs_blockid>> _pRanges; // Ranges needed to finish the under construction target file.
    QSharedPointer<QFile> _pTargetFile = nullptr; // Under construction target file.
    QSharedPointer<ZsyncRemoteControlFileParserPrivate> _pControlFileParser = nullptr; // Zsync control file.
};
}
#endif // ZSYNC_CORE_WORKER_PRIVATE_INCLUDED
