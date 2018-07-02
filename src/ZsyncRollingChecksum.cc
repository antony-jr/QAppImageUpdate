#include <QCryptographicHash>
#include <ZsyncRollingChecksum.hpp>
#include <cstdlib>

#define min(x , y) ((x < y) ? x : y)
#define max(x , y) ((x > y) ? x : y)

#define UPDATE_RSUM(a, b, oldc, newc, bshift) do { (a) += ((unsigned char)(newc)) - ((unsigned char)(oldc)); (b) += (a) - ((oldc) << (bshift)); } while (0)

using namespace AppImageUpdaterBridge;

static void freeHashEntry(hash_entry *entry)
{
    if(entry != NULL) {
        free(entry);
    }
    return;
}

static void freeRanges(zs_blockid *ranges)
{
    if(ranges != NULL) {
        free(ranges);
    }
    return;
}


ZsyncRollingChecksum::ZsyncRollingChecksum(QObject *parent)
    : QObject(parent)
{
    StrongHasher = QSharedPointer<QCryptographicHash>(new QCryptographicHash(QCryptographicHash::Md4));
    return;
}

ZsyncRollingChecksum::~ZsyncRollingChecksum()
{
    _pBlockHashes.clear();
    return;
}

void ZsyncRollingChecksum::setConfiguration(int nblocks,
        size_t blocksize,
        unsigned int rsum_bytes,
        unsigned int checksum_bytes,
        unsigned int seq_matches,
        const QString &filename)
{
    QMutexLocker locker(&_pMutex);

    _nBlocks = nblocks ;
    _nBlockSize = blocksize;
    _nRsumBytes = rsum_bytes;
    _nChecksumBytes = checksum_bytes;
    _nSeqMatches = seq_matches;

    _nRsumMaskA = _nRsumBytes < 3 ? 0 : _nRsumBytes == 3 ? 0xff : 0xffff;
    _nRsumBits = _nRsumBytes * 8;

    _nContext = _nBlockSize * _nSeqMatches;

    if (!(_nBlockSize & (_nBlockSize - 1)) && !filename.isEmpty() &&_nBlocks) {
        _pTargetFile = QSharedPointer<QFile>(new QFile(filename));

        if(!_pTargetFile->open(QIODevice::ReadWrite)) {
            _pTargetFile.clear();
            /*
             * Cannot open for write and read error.
            */
            return;
        } else {
            /* Calculate bit-shift for blocksize */
            for (int i = 0; i < 32; i++) {
                if (_nBlockSize == (1u << i)) {
                    _nBlockShift = i;
                    break;
                }
            }

            _pBlockHashes = QSharedPointer<hash_entry>(
                                (hash_entry*)calloc( _nBlocks + _nSeqMatches,
                                        sizeof((_pBlockHashes.data())[0])), freeHashEntry);
            if (_pBlockHashes == NULL) {
                /*
                 * Memory error.
                */
            }

        }
    }
    return;
}


rsum __attribute__((pure)) ZsyncRollingChecksum::calculateRollingChecksum(const unsigned char *data, size_t len)
{
    register unsigned short a = 0;
    register unsigned short b = 0;
    for (quint64 i = 0; i < len; i++) {
        unsigned char c = data[i];
        a += c;
        b += a;
    }
    {
        rsum r = { a, b };
        return r;
    }
}

void ZsyncRollingChecksum::calculateStrongChecksum(unsigned char *buffer, const unsigned char *data, size_t len)
{
    StrongHasher->reset();
    StrongHasher->addData((const char*)data, len);
    qstrcpy((char*)buffer, StrongHasher->result().constData());
    return;
}

void  ZsyncRollingChecksum::writeBlocks(const unsigned char *data,  zs_blockid bfrom, zs_blockid bto)
{
    off_t len = (bto - bfrom + 1) << _nBlockShift;
    off_t offset = (bfrom) << _nBlockShift;

    if(_pTargetFile != nullptr) {
        _pTargetFile->seek(offset);
        _pTargetFile->write((const char*)data, len);
    }

    /* Having written those blocks, discard them from the rsum hashes (as
     * we don't need to identify data for those blocks again, and this may
     * speed up lookups (in particular if there are lots of identical
     * blocks), and add the written blocks to the record of blocks that we
     * have received and stored the data for */
    for (zs_blockid id = bfrom; id <= bto; id++) {
        removeBlockFromHash(id);
        addToRanges(id);
    }
    return;
}

void ZsyncRollingChecksum::addTargetBlock(zs_blockid b, rsum r, void *checksum) {
    if (b < _nBlocks) {
        /* Get hash entry with checksums for this block */
        hash_entry *e = &((_pBlockHashes.data())[b]);

        /* Enter checksums */
        memcpy(e->checksum, checksum, _nChecksumBytes);
        e->r.a = r.a & _nRsumMaskA;
        e->r.b = r.b;

        /* New checksums invalidate any existing checksum hash tables */
        if (_pRsumHash) {
            free(_pRsumHash);
            _pRsumHash = NULL;
            free(_cBitHash);
            _cBitHash = NULL;
        }
    }
}

ssize_t ZsyncRollingChecksum::readKnownData(unsigned char *buffer, off_t offset, size_t len)
{
    ssize_t ret = 0;
    if(_pTargetFile != nullptr) {
        _pTargetFile->seek(offset);
        ret = _pTargetFile->read((char*)buffer, len);
    }
    return ret;
}


int ZsyncRollingChecksum::submitBlocks(const unsigned char *data, zs_blockid bfrom, zs_blockid bto)
{
    unsigned char md4sum[STRONG_CHECKSUM_SIZE];

    /* Build checksum hash tables if we don't have them yet */
    if (!_pRsumHash) {
        if (!buildHash())
            return -1;
    }

    /* Check each block */
    for (zs_blockid x = bfrom; x <= bto; x++) {
        calculateStrongChecksum(&md4sum[0], data + ((x - bfrom) << _nBlockShift), _nBlockSize);
        if (memcmp(&md4sum, &((_pBlockHashes.data())[x].checksum[0]), _nChecksumBytes)) {
            if (x > bfrom){      /* Write any good blocks we did get */
                writeBlocks(data, bfrom, x - 1);
            }
            return -1;
        }
    }

    /* All blocks are valid; write them and update our state */
    writeBlocks(data, bfrom, bto);
    return 0;
}

int ZsyncRollingChecksum::submitSourceData(unsigned char *data, size_t len, off_t offset)
{
   /* The window in data[] currently being considered is 
     * [x, x+bs)
     */
    call_count++;
    int x = 0;
    register int bs = _nBlockSize;
    int got_blocks = 0;

    if (offset) {
        x = _nSkip;
    }
    else {
        _pNextMatch = NULL;
    }

    if (x || !offset) {
        _pCurrentSums[0] = calculateRollingChecksum(data + x, bs);
        if (_nSeqMatches > 1)
            _pCurrentSums[1] = calculateRollingChecksum(data + x + bs, bs);
    }
    _nSkip = 0;

    /* Work through the block until the current blocksize bytes being
     * considered, starting at x, is at the end of the buffer */
    for (;;) {
        if (x + _nContext == len) {
            return got_blocks;
        }

#if 0
        {   /* Catch rolling checksum failure */
            int k = 0;
            struct rsum c = rcksum_calc_rsum_block(data + x + bs * k, bs);
            if (c.a != z->r[k].a || c.b != z->r[k].b) {
                fprintf(stderr, "rsum miscalc (%d) at %lld\n", k, offset + x);
                exit(3);
            }
        }
#endif

        {
            /* # of blocks of the output file we got from this data */
            int thismatch = 0;
            /* # of blocks to advance if thismatch > 0. Can be less than
             * thismatch as thismatch could be N*blocks_matched, if a block was
             * duplicated to multiple locations in the output file. */
            int blocks_matched = 0; 

            /* If the previous block was a match, but we're looking for
             * sequential matches, then test this block against the block in
             * the target immediately after our previous hit. */
            if (_pNextMatch && _nSeqMatches > 1) {
                if (0 != (thismatch = checkChecksumOnHashChain(_pNextMatch, data + x, 1))) {
                    blocks_matched = 1;
                }
            }
            if (!thismatch) {
                const struct hash_entry *e;

                /* Do a hash table lookup - first in the bithash (fast negative
                 * check) and then in the rsum hash */
                unsigned hash = _pCurrentSums[0].b;
                hash ^= ((_nSeqMatches > 1) ? _pCurrentSums[1].b
                        : _pCurrentSums[0].a & _nRsumMaskA) << BIT_HASH_BITS;
		if ((_cBitHash[(hash & _nBitHashMask) >> 3] & (1 << (hash & 7))) != 0
                    && (e = _pRsumHash[hash & _nHashMask]) != NULL) {

                    /* Okay, we have a hash hit. Follow the hash chain and
                     * check our block against all the entries. */
                    thismatch = checkChecksumOnHashChain(e, data + x, 0);
                    if (thismatch)
                        blocks_matched = _nSeqMatches;
                }
            }
            got_blocks += thismatch;

            /* If we got a hit, skip forward (if a block in the target matches
             * at x, it's highly unlikely to get a hit at x+1 as all the
             * target's blocks are multiples of the blocksize apart. */
            if (blocks_matched) {
                x += bs + (blocks_matched > 1 ? bs : 0);

                if (x + _nContext > len) {
                    /* can't calculate rsum for block after this one, because
                     * it's not in the buffer. So leave a hint for next time so
                     * we know we need to recalculate */
                    _nSkip = x +_nContext - len;
                    return got_blocks;
                }

                /* If we are moving forward just 1 block, we already have the
                 * following block rsum. If we are skipping both, then
                 * recalculate both */
                if (_nSeqMatches > 1 && blocks_matched == 1)
                    _pCurrentSums[0] = _pCurrentSums[1];
                else
                    _pCurrentSums[0] = calculateRollingChecksum(data + x, bs);
                if (_nSeqMatches > 1)
                    _pCurrentSums[1] = calculateRollingChecksum(data + x + bs, bs);
                continue;
            }
        }

        /* Else - advance the window by 1 byte - update the rolling checksum
         * and our offset in the buffer */
        {
            unsigned char Nc = data[x + bs * 2];
            unsigned char nc = data[x + bs];
            unsigned char oc = data[x];
            UPDATE_RSUM(_pCurrentSums[0].a, _pCurrentSums[0].b, oc, nc, _nBlockShift);
            if (_nSeqMatches > 1)
                UPDATE_RSUM(_pCurrentSums[1].a, _pCurrentSums[1].b, nc, Nc, _nBlockShift);
        }
        x++;
    }
}


void ZsyncRollingChecksum::removeBlockFromHash(zs_blockid id)
{
    hash_entry *t = &((_pBlockHashes.data())[id]);
    hash_entry **p = &(_pRsumHash[calcRHash(t) & _nHashMask]);

    while (*p != NULL) {
        if (*p == t) {
            if (t == _pRover) {
                _pRover = t->next;
            }
            *p = (*p)->next;
            return;
        } else {
            p = &((*p)->next);
        }
    }
}


int ZsyncRollingChecksum::submitSourceFile(FILE *f) {
    /* Track progress */
    int got_blocks = 0;
    off_t in = 0;
    int in_mb = 0;

    /* Allocate buffer of 16 blocks */
    register size_t bufsize = _nBlockSize * 16;
    unsigned char *buf = (unsigned char*)malloc(bufsize + _nContext);
    if (!buf)
        return 0;

    /* Build checksum hash tables ready to analyse the blocks we find */
    if (!_pRsumHash)
        if (!buildHash()) {
            free(buf);
            return 0;
        }

    while (!feof(f)) {
        size_t len;
        off_t start_in = in;

        /* If this is the start, fill the buffer for the first time */
        if (!in) {
            len = fread(buf, 1, bufsize, f);
            in += len;
        }

        /* Else, move the last context bytes from the end of the buffer to the
         * start, and refill the rest of the buffer from the stream. */
        else {
            memcpy(buf, buf + (bufsize - _nContext), _nContext);
            in += bufsize - _nContext;
            len = _nContext + fread(buf + _nContext, 1, bufsize - _nContext, f);
        }

        /* If either fread above failed, or EOFed */
        if (ferror(f)) {
            perror("fread");
            free(buf);
            return got_blocks;
        }
        if (feof(f)) {          /* 0 pad to complete a block */
            memset(buf + len, 0, _nContext);
            len += _nContext;
        }

        /* Process the data in the buffer, and report progress */
        got_blocks += submitSourceData(buf, len, start_in);
    }
    free(buf);
    return got_blocks;
}


/* int ZsyncRollingChecksum::submitSourceFile(QFile *file)
 {
    int got_blocks = 0;
    off_t in = 0;
    off_t size = file->size();

    /* Allocate buffer of 16 blocks */
/*    register size_t bufsize = _nBlockSize * 16;
    unsigned char *buf = (unsigned char*)calloc(bufsize + _nContext, 1);
    if (!buf) {
        return 0;
    }

    /* Build checksum hash tables ready to analyse the blocks we find */
/*    if (!_pRsumHash)
        if (!buildHash()) {
            free(buf);
            return 0;
        }

    while (!file->atEnd()) {
        size_t len;
        off_t start_in = in;

        /* If this is the start, fill the buffer for the first time */
 /*       if (!in) {
            len = file->read((char*)buf, bufsize);
            in += len;
        }

        /* Else, move the last context bytes from the end of the buffer to the
         * start, and refill the rest of the buffer from the stream. */
/*        else {
            memcpy(buf, buf + (bufsize - _nContext), _nContext);
            in += bufsize - _nContext;
            len = _nContext + file->read((char*)(buf + _nContext), bufsize - _nContext);
        }

        /* If either read above failed, or EOFed 
        if (len_buf == -1) {
            free(buf);
            return got_blocks;
        }
        */
/*        if (file->atEnd()) {          /* 0 pad to complete a block */
/*            memset(buf + len, 0, _nContext);
            len += _nContext;
        }

        /* Process the data in the buffer, and report progress */
//         got_blocks += submitSourceData(buf, len, start_in);
        /*
         if (progress && in_mb != in / 1000000) {
             do_progress(p, 100.0 * in / size, in);
             in_mb = in / 1000000;
         }
         */
/*    }
    free(buf);
    return got_blocks;
}*/


unsigned ZsyncRollingChecksum::calcRHash(const hash_entry *const e)
{
    unsigned h = e[0].r.b;
    h ^= (_nSeqMatches > 1) ? e[1].r.b : (e[0].r.a & _nRsumMaskA) << _nHashFuncShift;
    return h;
}

int ZsyncRollingChecksum::rangeBeforeBlock(zs_blockid x)
{
    /* Lowest number and highest number block that it could be inside (0 based) */
    register int min = 0, max = _nNumRanges-1;

    /* By bisection */
    for (; min<=max;) {
        /* Range number to compare against */
        register int r = (max+min)/2;

        if (x > (_pRanges.data())[2*r+1]) {
            min = r+1;  /* After range r */
        } else if (x < (_pRanges.data())[2*r]) {
            max = r-1;/* Before range r */
        } else {
            return -1;
        }
    }

    /* If we reach here, we know min = max + 1 and we were below range max+1
     * and above range min-1.
     * So we're between range max and max + 1
     * So we return max + 1  (return value is 1 based)  ( = min )
     */
    return min;
}

void ZsyncRollingChecksum::addToRanges(zs_blockid x)
{
    int r = rangeBeforeBlock(x);

    if (r == -1) {
        /* Already have this block */
    } else {
        _nGotBlocks++;

        /* If between two ranges and exactly filling the hole between them,
         * merge them */
        if (r > 0 && r < _nNumRanges
            && (_pRanges.data())[2 * (r - 1) + 1] == x - 1
            && (_pRanges.data())[2 * r] == x + 1) {

            // This block fills the gap between two areas that we have got completely. Merge the adjacent ranges
            (_pRanges.data())[2 * (r - 1) + 1] = (_pRanges.data())[2 * r + 1];
            memmove(&(_pRanges.data())[2 * r], &(_pRanges.data())[2 * r + 2],
                    (_nNumRanges - r - 1) * sizeof((_pRanges.data())[0]) * 2);
            _nNumRanges--;
        }

        /* If adjoining a range below, add to it */
        else if (r > 0 && _nNumRanges && (_pRanges.data())[2 * (r - 1) + 1] == x - 1) {
            (_pRanges.data())[2 * (r - 1) + 1] = x;
        }

        /* If adjoining a range above, add to it */
        else if (r < _nNumRanges && (_pRanges.data())[2 * r] == x + 1) {
            (_pRanges.data())[2 * r] = x;
        }

        else { /* New range for this block alone */
            auto newRanges = QSharedPointer<zs_blockid>((zs_blockid*)realloc(_pRanges.data(), (_nNumRanges + 1) * 2 * sizeof((_pRanges.data())[0])), freeRanges );
            if(newRanges != nullptr) {
                _pRanges.clear();
                _pRanges = newRanges;
            } else {
                /*
                 * Memory error.
                */
            }

            memmove(&(_pRanges.data())[2 * r + 2], &(_pRanges.data())[2 * r],
                    (_nNumRanges - r) * 2 * sizeof((_pRanges.data())[0]));
            (_pRanges.data())[2 * r] = (_pRanges.data())[2 * r + 1] = x;
            _nNumRanges++;
        }
    }
}

zs_blockid ZsyncRollingChecksum::nextKnownBlock(zs_blockid x)
{
    int r = rangeBeforeBlock(x);
    if (r == -1)
        return x;
    if (r == _nNumRanges) {
        return _nBlocks;
    }
    /* Else return first block of next known range. */
    return (_pRanges.data())[2*r];
}

int ZsyncRollingChecksum::buildHash(void)
{
    zs_blockid id;
    int avail_bits = _nSeqMatches > 1 ? min(_nRsumBits, 16)*2 : _nRsumBits;
    int hash_bits = avail_bits;

    /* Pick a hash size that is a power of two and gives a load factor of <1 */
    while ((1U << (hash_bits-1)) > _nBlocks && hash_bits > 5) {
        hash_bits--;
    }

    /* Allocate hash based on rsum */
    _nHashMask = (1U << hash_bits) - 1;
    _pRsumHash = (hash_entry**) calloc(_nHashMask + 1, sizeof *(_pRsumHash));
    if (!_pRsumHash) {
        return 0;
    }

    /* Allocate bit-table based on rsum. Aim is for 1/(1<<BITHASHBITS) load
     * factor, so hash_bits should be hash_bits + BITHASHBITS if we have that
     * many bits available. */
    hash_bits = min(hash_bits + BIT_HASH_BITS, avail_bits);
    _nBitHashMask = (1U << hash_bits) - 1;
    _cBitHash =(unsigned char*) calloc(_nBitHashMask + 1, 1);
    if (!_cBitHash) {
        free(_pRsumHash);
        _pRsumHash = NULL;
        return 0;
    }

    /* We want the hash function to return hash_bits bits. We will xor one
     * number with a second number that may have fewer than 16 bits of
     * available data; set up an appropriate bit shift for the second number.
     * This is closely tied to calc_rhash().
     */
    if (_nSeqMatches > 1 && avail_bits < 24) {
        /* second number has (avail_bits/2) bits available. */
        _nHashFuncShift = max(0, hash_bits - (avail_bits / 2));
    } else {
        /* second number has avail_bits - 16 bits available. */
        _nHashFuncShift = max(0, hash_bits - (avail_bits - 16));
    }

    /* Now fill in the hash tables.
     * Minor point: We do this in reverse order, because we're adding entries
     * to the hash chains by prepending, so if we iterate over the data in
     * reverse then the resulting hash chains have the blocks in normal order.
     * That's improves our pattern of I/O when writing out identical blocks
     * once we are processing data; we will write them in order. */
    for (id = _nBlocks; id > 0;) {
        /* Decrement the loop variable here, and get the hash entry. */
        hash_entry *e = (_pBlockHashes.data()) + (--id);

        /* Prepend to linked list for this hash entry */
        unsigned h = calcRHash(e);
        e->next = _pRsumHash[h & _nHashMask];
        _pRsumHash[h & _nHashMask] = e;

        /* And set relevant bit in the bithash to 1 */
        _cBitHash[(h & _nBitHashMask) >> 3] |= 1 << (h & 7);
    }
    return 1;
}

int ZsyncRollingChecksum::checkChecksumOnHashChain(const hash_entry *e, const unsigned char *data, int onlyone)
{
    unsigned char md4sum[2][STRONG_CHECKSUM_SIZE];
    signed int done_md4 = -1;
    int got_blocks = 0;
    register rsum r = _pCurrentSums[0];

    /* This is a hint to the caller that they should try matching the next
     * block against a particular hash entry (because at least z->seq_matches
     * prior blocks to it matched in sequence). Clear it here and set it below
     * if and when we get such a set of matches. */
    _pNextMatch = NULL;

    /* This is essentially a for (;e;e=e->next), but we want to remove links from
     * the list as we find matches, without keeping too many temp variables.
     */
    _pRover = e;
    while (_pRover) {
        zs_blockid id;

        e = _pRover;
        _pRover = onlyone ? NULL : e->next;

        /* Check weak checksum first */

        _nHashHit++;
        if (e->r.a != (r.a & _nRsumMaskA) || e->r.b != r.b) {
            continue;
        }

        id = getHashEntryBlockID(e);

        if (!onlyone && _nSeqMatches > 1
            && (e[1].r.a != (_pCurrentSums[1].a & _nRsumMaskA)
                || e[1].r.b != _pCurrentSums[1].b)) {
            continue;
        }

        _nWeakHit++;

        {
            int ok = 1;
            signed int check_md4 = 0;
            /* This block at least must match; we must match at least
             * z->seq_matches-1 others, which could either be trailing stuff,
             * or these could be preceding blocks that we have verified
             * already. */
            do {
                /* We only calculate the MD4 once we need it; but need not do so twice */
                if (check_md4 > done_md4) {
                    calculateStrongChecksum(&md4sum[check_md4][0],
                                            data + (_nBlockSize * check_md4),
                                            _nBlockSize);
                    done_md4 = check_md4;
                    _nCheckSummed++;
                }

                /* Now check the strong checksum for this block */
                if (memcmp(&md4sum[check_md4],
                           e[check_md4].checksum,
                           _nChecksumBytes)){
                ok = 0;
                }
                
                check_md4++;
            } while (ok && !onlyone && check_md4 < _nSeqMatches);       

            if (ok) {
                int num_write_blocks;

                /* Find the next block that we already have data for. If this
                 * is part of a run of matches then we have this stored already
                 * as ->next_known. */
                zs_blockid next_known = onlyone ? _nNextKnown : nextKnownBlock(id);

                _nStrongHit += check_md4;

                if (next_known > id + check_md4) {
                    num_write_blocks = check_md4;

                    /* Save state for this run of matches */
                    _pNextMatch = &((_pBlockHashes.data())[id + check_md4]);
                    if (!onlyone) {
                        _nNextKnown = next_known;
                    }
                } else {
                    /* We've reached the EOF, or data we already know. Just
                     * write out the blocks we don't know, and that's the end
                     * of this run of matches. */
                    num_write_blocks = next_known - id;
                }

                /* Write out the matched blocks that we don't yet know */
                writeBlocks(data, id, id + num_write_blocks - 1);
                got_blocks += num_write_blocks;
            }
        }
    }
    return got_blocks;
}

zs_blockid ZsyncRollingChecksum::getHashEntryBlockID(const hash_entry *e)
{
    return e - _pBlockHashes.data();
}

QVector<QPair<zs_blockid, zs_blockid>> ZsyncRollingChecksum::neededBlockRanges(zs_blockid from, zs_blockid to)
{
    QVector<QPair<zs_blockid, zs_blockid>> ranges;
    qint64 n;
    QVector<zs_blockid> buffer;
    if (to >= _nBlocks) {
        to = _nBlocks;
    }
    ranges.append(qMakePair(from, to));
    n = 1;
    /* Note r[2*n-1] is the last range in our prospective list */

    for (qint64 i = 0; i < _nNumRanges; i++) {
        if ((_pRanges.data())[2 * i] > ranges[n - 1].first) {
            continue;
        }
        if ((_pRanges.data())[2 * i + 1] < from) {
            continue;
        }

        /* Okay, they intersect */
        if (n == 1 && (_pRanges.data())[2 * i] <= from) {       /* Overlaps the start of our window */
            ranges[0].first =  (_pRanges.data())[2 * i + 1] + 1;
        } else {
            /* If the last block that we still (which is the last window end -1, due
             * to half-openness) then this range just cuts the end of our window */
            if ((_pRanges.data())[2 * i + 1] >= ranges[n - 1].second - 1) {
                ranges[n - 1].second = (_pRanges.data())[2 * i];
            } else {
                /* In the middle of our range, split it */
                ranges[n - 1].second =  (_pRanges.data())[2 * i + 1] + 1;
                ranges[n + 1].first =  ranges[n - 1].second;
                ranges[n - 1].second =  (_pRanges.data())[2 * i];
                n++;

            }
        }
        ranges.append(qMakePair(0, 0));
    }
    if (n == 1 && ranges.value(0).first >= ranges.value(0).second) {
        ranges.clear();
    }

    return ranges;
}
