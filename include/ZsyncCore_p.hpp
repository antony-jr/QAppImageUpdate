#ifndef ZSYNC_CORE_WORKER_INCLUDED
#define ZSYNC_CORE_WORKER_INCLUDED
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
    void getNeededRanges(void);
    void addSourceFile(const QString&);

private Q_SLOTS:
    void processControlFile(void);
    void addTargetFileCheckSumBlocks(zs_blockid, rsum, void*);
    void completeTargetFileCheckSumBlocks(void);
    void writeBlocks(const unsigned char *, zs_blockid, zs_blockid); 
    void addToRanges(zs_blockid);
    void removeBlockFromHash(zs_blockid);
    qint32 submitTargetFileBlocks(const unsigned char*, zs_blockid, zs_blockid );
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

Q_SIGNALS:
    void receiveNeededBlockRanges(QVector<QPair<zs_blockid, zs_blockid>>);
    void addedSourceFile(QString);
    void logger(QString);

private:
    QPair<rsum , rsum> _pCurrentWeakCheckSums = qMakePair(rsum({ 0 , 0 }) , rsum({ 0 , 0 }));
    size_t _nBlocks = 0 , 
	       _nBlockSize = 0;
    qint32 _nBlockShift = 0, // log2(blocksize).
	   _nContext = 0,    // precalculated blocksize * seq_matches.
	   _nSkip = 0,       // skip forward on next submit_source_data.
	   _nGotBlocks = 0,  // # of blocks that we currently have in the under construction target file.
       _nStrongCheckSumBytes = 0, // # of bytes available for the strong checksum.
       _nSeqMatches = 0,
       _nTargetFileLength = 0; // Length of the target file , used to truncate the temporary file.
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

    QStringList sourceFiles;
    qint32 _nRanges = 0;
    zs_blockid *_pRanges = nullptr; // Ranges needed to finish the under construction target file.
    QSharedPointer<QTemporaryFile> _pTargetFile = nullptr; // Under construction target file.
    QSharedPointer<QThread> _pControlFileParserThread = nullptr; // Thread affinity of the control file parser.
    QSharedPointer<ZsyncRemoteControlFileParserPrivate> _pControlFileParser = nullptr; // Zsync control file.
};
}
#endif // ZSYNC_CORE_PRIVATE_INCLUDED
