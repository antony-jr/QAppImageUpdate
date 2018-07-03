#ifndef ZSYNC_CORE_WORKER_PRIVATE_INCLUDED
#define ZSYNC_CORE_WORKER_PRIVATE_INCLUDED
#include <QtCore>

namespace AppImageUpdaterBridge_p { 
static constexpr unsigned short CHECKSUM_SIZE = 16;
static constexpr unsigned short BITHASHBITS = 3;
typedef qint32 zs_blockid;

struct rsum {
	unsigned short	a;
	unsigned short	b;
} __attribute__((packed));

struct hash_entry {
    struct hash_entry *next;    /* next entry with the same rsum */
    struct rsum r;
    unsigned char checksum[CHECKSUM_SIZE];
};

class ZsyncCoreWorker : public QObject {
	Q_OBJECT
public:
    explicit ZsyncCoreWorker(zs_blockid , size_t , int , int , int , QObject *parent = nullptr );
    
    char* get_filename(void);
    int filehandle(void);
    void add_target_block(zs_blockid , rsum , void* );
    int submit_blocks(const unsigned char* , zs_blockid , zs_blockid );
    int submit_source_data(unsigned char* , size_t , off_t );
    int submit_source_file(QFile*);
    int read_known_data(unsigned char* , off_t , size_t );
    QVector<QPair<zs_blockid , zs_blockid>> needed_block_ranges(zs_blockid , zs_blockid);
    int blocks_todo(void);
    
    ~ZsyncCoreWorker();
private Q_SLOTS:
    int check_checksums_on_hash_chain(const hash_entry *, const unsigned char * ,int );
    void write_blocks(const unsigned char *, zs_blockid , zs_blockid);
    zs_blockid get_HE_blockid(const struct hash_entry *);
    void add_to_ranges(zs_blockid);
    int range_before_block(zs_blockid);
    int already_got_block(zs_blockid);
    zs_blockid next_known_block(zs_blockid);
    unsigned calc_rhash(const struct hash_entry *const);
    int build_hash(void);
    void remove_block_from_hash(zs_blockid);
    
private:
    rsum r[2];           /* Current rsums */

    zs_blockid blocks;          /* Number of blocks in the target file */
    size_t blocksize;           /* And how many bytes per block */
    int blockshift;             /* log2(blocksize) */
    unsigned short rsum_a_mask; /* The mask to apply to rsum values before looking up */
    int checksum_bytes;         /* How many bytes of the MD4 checksum are available */
    int seq_matches;

    unsigned int context;       /* precalculated blocksize * seq_matches */

    /* These are used by the library. Note, not thread safe. */
    const hash_entry *rover;
    int skip;                   /* skip forward on next submit_source_data */

    /* Internal; hint to rcksum_submit_source_data that it should try matching
     * the following block of input data against the block ->next_match.
     * next_known is a cached lookup of the id of the next block after that
     * that we already have data for. */
    const hash_entry *next_match;
    zs_blockid next_known;

    /* Hash table for rsync algorithm */
    unsigned int hashmask;
    hash_entry *blockhashes;
    hash_entry **rsum_hash;

    /* And a 1-bit per rsum value table to allow fast negative lookups for hash
     * values that don't occur in the target file. */
    unsigned int bithashmask;
    unsigned char *bithash;

    /* Current state and stats for data collected by algorithm */
    int numranges;
    zs_blockid *ranges;
    int gotblocks;
    struct {
        int hashhit, weakhit, stronghit, checksummed;
    } stats;

    /* Temp file for output */
    char *filename;
    int fd;

};
}
#endif // ZSYNC_CORE_WORKER_PRIVATE_INCLUDED
