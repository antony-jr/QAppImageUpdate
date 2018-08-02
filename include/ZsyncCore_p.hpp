#ifndef ZSYNC_CORE_WORKER_INCLUDED
#define ZSYNC_CORE_WORKER_INCLUDED
#include <QtCore>
#include <ZsyncInternalStructures_p.hpp>

namespace AppImageUpdaterBridge
{
class ZsyncCorePrivate
{
public:
    explicit ZsyncCorePrivate(size_t ,zs_blockid , size_t, qint32 ,qint32 ,qint32 ,QBuffer *, QFile *);
    QPair<qint32 , QVector<QPair<zs_blockid, zs_blockid>>*> *start(const QString&);
    ~ZsyncCorePrivate();

private:
/**/    void writeBlocks(const unsigned char *, zs_blockid, zs_blockid); 
/**/    void addToRanges(zs_blockid);
/**/    void removeBlockFromHash(zs_blockid);
/**/    qint32 submitSourceData(unsigned char*, size_t, off_t);
/**/    qint32 submitSourceFile(QFile*);
/**/    qint32 checkCheckSumsOnHashChain(const hash_entry *, const unsigned char *, qint32 );
/**/    qint32 rangeBeforeBlock(zs_blockid);
/**/    qint32 alreadyGotBlock(zs_blockid);
/**/    qint32 blocksToDo(void);
/**/    qint32 buildHash(void);
/**/    quint32 calcRHash(const hash_entry *const);
/**/    zs_blockid getHashEntryBlockId(const hash_entry *);
/**/    zs_blockid nextKnownBlock(zs_blockid);
    
    QPair<rsum , rsum> _pCurrentWeakCheckSums = qMakePair(rsum({ 0 , 0 }) , rsum({ 0 , 0 }));
    size_t _nBlocks = 0 , 
	   _nBlockSize = 0;
    qint32 _nBlockShift = 0, // log2(blocksize).
	   _nContext = 0,    // precalculated blocksize * seq_matches.
	   _nSkip = 0,       // skip forward on next submit_source_data.
	   _nGotBlocks = 0,  // # of blocks that we currently have in the under construction target file.
	   _nWeakCheckSumBytes = 0,
       	   _nStrongCheckSumBytes = 0, // # of bytes available for the strong checksum.
       	   _nSeqMatches = 0;
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
    zs_blockid _nBlockIdOffset = 0;
    zs_blockid *_pRanges = nullptr; // Ranges needed to finish the under construction target file.
    QFile *_pTargetFile = nullptr; // Under construction target file.
    QBuffer *_pTargetFileCheckSumBlocks = nullptr; // Checksum blocks that needs to be loaded into the memory.
};
}
#endif // ZSYNC_CORE_PRIVATE_INCLUDED
