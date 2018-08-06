#ifndef ZSYNC_CORE_JOB_PRIVATE_HPP_INCLUDED
#define ZSYNC_CORE_JOB_PRIVATE_HPP_INCLUDED
#include <QtCore>
#include <ZsyncInternalStructures_p.hpp>

namespace AppImageUpdaterBridge
{
class ZsyncCoreJobPrivate
{
public:

	struct Information {
	 size_t blockSize , blocks;
	 zs_blockid blockIdOffset;
	 qint32 weakCheckSumBytes,
		strongCheckSumBytes,
		seqMatches;
	 QBuffer *checkSumBlocks;
	 QFile *targetFile;
	 QString seedFilePath;
	
	 Information(void)
	 {
		 return;
	 } 

	 Information(size_t bs, zs_blockid bio, size_t nb,
			qint32 wcksumn, qint32 scksumn, qint32 sm,
			QBuffer *ckb, QFile *f, const QString &s)
	 {
 		blockSize = bs;
	   	blockIdOffset = bio;
	   	blocks = nb;
	   	weakCheckSumBytes = wcksumn;
	   	strongCheckSumBytes = scksumn;
	   	seqMatches = sm;
	   	checkSumBlocks = ckb;
	   	targetFile = f;
	   	seedFilePath = s;
		return;
	 }

    };

    struct Result {
        short errorCode = 0;
        qint32 gotBlocks = 0;
        QVector<QPair<QPair<zs_blockid , zs_blockid> , QVector<QByteArray>>> *requiredRanges = nullptr;
    };

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
    
    explicit ZsyncCoreJobPrivate(const Information&);
    ~ZsyncCoreJobPrivate();
    
    Result operator () (void);
private:
    void addToRanges(zs_blockid);
    qint32 alreadyGotBlock(zs_blockid); 
    qint32 blocksToDo(void);
    qint32 buildHash(void);
    qint32 checkCheckSumsOnHashChain(const hash_entry *, const unsigned char *, qint32 );
    quint32 calcRHash(const hash_entry *const);
    QVector<QPair<QPair<zs_blockid , zs_blockid> , QVector<QByteArray>>> *getRequiredRanges(void);
    zs_blockid getHashEntryBlockId(const hash_entry *);
    short tryOpenSeedFile(QFile**);
    short parseTargetFileCheckSumBlocks(void);
    void writeBlocks(const unsigned char *, zs_blockid, zs_blockid); 
    void removeBlockFromHash(zs_blockid);
    qint32 submitSourceData(unsigned char*, size_t, off_t);
    qint32 submitSourceFile(QFile*);
    qint32 rangeBeforeBlock(zs_blockid);
    zs_blockid nextKnownBlock(zs_blockid);
    
    QPair<rsum , rsum> _pCurrentWeakCheckSums = qMakePair(rsum({ 0 , 0 }) , rsum({ 0 , 0 }));
    size_t _nBlocks = 0, _nBlockSize = 0;
    qint32 _nBlockShift = 0, // log2(blocksize).
	   _nContext = 0,    // precalculated blocksize * seq_matches.
	   _nWeakCheckSumBytes = 0,
       	   _nStrongCheckSumBytes = 0, // # of bytes available for the strong checksum.
       	   _nSeqMatches = 0,
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

    qint32 _nRanges = 0;
    zs_blockid *_pRanges = nullptr; // Ranges needed to finish the under construction target file.
    QBuffer *_pTargetFileCheckSumBlocks = nullptr; // Checksum blocks that needs to be loaded into the memory.
    zs_blockid _nBlockIdOffset = 0;
    QFile *_pTargetFile = nullptr; // Under construction target file.
    QString _sSeedFilePath;
};
}
#endif // ZSYNC_CORE_JOB_PRIVATE_HPP_INCLUDED
