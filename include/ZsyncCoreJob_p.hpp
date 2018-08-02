#ifndef ZSYNC_CORE_JOB_PRIVATE_HPP_INCLUDED
#define ZSYNC_CORE_JOB_PRIVATE_HPP_INCLUDED
#include <QtCore>
#include <ZsyncInternalStructures_p.hpp>

namespace AppImageUpdaterBridge
{
class ZsyncCoreJobPrivate
{
public:
    struct JobInformation {
	 bool isEmpty = true;
	 size_t blockSize , blocks;
	 zs_blockid blockIdOffset;
	 qint32 weakCheckSumBytes,
		strongCheckSumBytes,
		seqMatches;
	 QBuffer *checkSumBlocks;
	 QFile *targetFile;
     QFile *seedFile;

	 JobInformation(size_t bs,
			zs_blockid bio,
			size_t nb,
			qint32 wcksumn,
			qint32 scksumn,
			qint32 sm,
			QBuffer *ckb,
			QFile *f)
	 : blockSize(bs),
	   blockIdOffset(bio),
	   blocks(nb),
	   weakCheckSumBytes(wcksumn),
	   strongCheckSumBytes(scksumn),
	   seqMatches(sm),
	   checkSumBlocks(ckb),
	   targetFile(f)
	 {
		 if(!checkSumBlocks || !targetFile || !seedFile){
			 throw std::runtime_error("invalid memory address given for checksum blocks or target file.");
		 }
		 isEmpty = false;
		 return;
	 }

	 /* Use the default destructor and the copy constructor, 
	  * So no need to state the obvious. 
	 */
    };
    
    struct JobResult {
        bool isErrored = false;
        short errorCode = 0;
        qint32 gotBlocks = 0;
        QVector<QPair<zs_blockid , zs_blockid>> *requiredRanges = nullptr;
    };
    
    enum : short {
        NO_ERROR = -1,
	HASH_TABLE_NOT_ALLOCATED = 100,
        INVALID_TARGET_FILE_CHECKSUM_BLOCKS,
        CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS,
        QBUFFER_IO_READ_ERROR,
        SOURCE_FILE_NOT_FOUND,
        NO_PERMISSION_TO_READ_SOURCE_FILE,
        CANNOT_OPEN_SOURCE_FILE
    } error_code;
    
    explicit ZsyncCoreJobPrivate(const JobInformation&);
    JobResult start(void);
    ~ZsyncCoreJobPrivate();
    
private:
    void writeBlocks(const unsigned char *, zs_blockid, zs_blockid); 
    void addToRanges(zs_blockid);
    void removeBlockFromHash(zs_blockid);
    qint32 submitSourceData(unsigned char*, size_t, off_t);
    qint32 submitSourceFile(QFile*);
    qint32 checkCheckSumsOnHashChain(const hash_entry *, const unsigned char *, qint32 );
    qint32 rangeBeforeBlock(zs_blockid);
    qint32 alreadyGotBlock(zs_blockid);
    qint32 blocksToDo(void);
    qint32 buildHash(void);
    quint32 calcRHash(const hash_entry *const);
    zs_blockid getHashEntryBlockId(const hash_entry *);
    zs_blockid nextKnownBlock(zs_blockid);
    
    QPair<rsum , rsum> _pCurrentWeakCheckSums = qMakePair(rsum({ 0 , 0 }) , rsum({ 0 , 0 }));
    size_t _nBlocks , _nBlockSize;
    qint32 _nBlockShift, // log2(blocksize).
	   _nContext,    // precalculated blocksize * seq_matches.
	   _nWeakCheckSumBytes,
       	   _nStrongCheckSumBytes, // # of bytes available for the strong checksum.
       	   _nSeqMatches,
    	   _nSkip = 0,       // skip forward on next submit_source_data.
	   _nGotBlocks = 0;  // # of blocks that we currently have in the under construction target file.
    unsigned short _pWeakCheckSumMask; // This will be applied to the first 16 bits of the weak checksum.
    
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
    zs_blockid _nBlockIdOffset;
    QFile *_pTargetFile; // Under construction target file.
    QFile *_pSeedFile;
};
}
#endif // ZSYNC_CORE_JOB_PRIVATE_HPP_INCLUDED
