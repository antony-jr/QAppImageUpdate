#include <ZsyncCore_p.hpp>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

using namespace AppImageUpdaterBridgePrivate;

#define UPDATE_RSUM(a, b, oldc, newc, bshift) do { (a) += ((unsigned char)(newc)) - ((unsigned char)(oldc)); (b) += (a) - ((oldc) << (bshift)); } while (0)



/* rcksum_calc_rsum_block(data, data_len)
 * Calculate the rsum for a single block of data. */
// calc_rsum_block
static rsum __attribute__ ((pure)) calc_rsum_block(const unsigned char *data, size_t len)
{
    register unsigned short a = 0;
    register unsigned short b = 0;

    while (len) {
        unsigned char c = *data++;
        a += c;
        b += len * c;
        len--;
    }
    {
        struct rsum r = { a, b };
        return r;
    }
}


/* rcksum_calc_checksum(checksum_buf, data, data_len)
 * Returns the MD4 checksum (in checksum_buf) of the given data block */
// calc_checksum
static void calc_checksum(unsigned char *c, const unsigned char *data,
                          size_t len)
{
    QCryptographicHash ctx(QCryptographicHash::Md4);
    ctx.addData((const char*)data, len);
    auto result = ctx.result();
    memmove(c, result.constData(), sizeof(const char) * result.size());
    return;
}

/*
 * Constructor and Destructor
*/
ZsyncCorePrivate::ZsyncCorePrivate(QNetworkAccessManager *NetworkManager)
    : QObject(NetworkManager)
{
    _pControlFileParserThread = QSharedPointer<QThread>(new QThread);
    _pControlFileParser = QSharedPointer<ZsyncRemoteControlFileParserPrivate>(new ZsyncRemoteControlFileParserPrivate(NetworkManager));
    
    connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::receiveControlFile ,
            this , &ZsyncCorePrivate::processControlFile);
    connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::receiveTargetFileBlocks ,
            this , &ZsyncCorePrivate::addTargetFileCheckSumBlocks);
    connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::endOfTargetFileBlocks ,
            this , &ZsyncCorePrivate::completeTargetFileCheckSumBlocks);
    
    _pControlFileParser->moveToThread(_pControlFileParserThread.data());
    _pControlFileParserThread->start();
    return;
}

ZsyncCorePrivate::ZsyncCorePrivate(const QUrl &controlFileUrl , QNetworkAccessManager *NetworkManager)
    : QObject(NetworkManager)
{
    _pControlFileParserThread = QSharedPointer<QThread>(new QThread);
    _pControlFileParser = QSharedPointer<ZsyncRemoteControlFileParserPrivate>(new ZsyncRemoteControlFileParserPrivate(NetworkManager));
    
    connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::receiveControlFile ,
            this , &ZsyncCorePrivate::processControlFile);
    connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::receiveTargetFileBlocks ,
            this , &ZsyncCorePrivate::addTargetFileCheckSumBlocks);
    connect(_pControlFileParser.data() , &ZsyncRemoteControlFileParserPrivate::endOfTargetFileBlocks ,
            this , &ZsyncCorePrivate::completeTargetFileCheckSumBlocks);
    
    _pControlFileParser->moveToThread(_pControlFileParserThread.data());
    _pControlFileParserThread->start();
    setControlFileUrl(controlFileUrl);
    return;
}

void ZsyncCorePrivate::setControlFileUrl(const QUrl &url)
{
    _pControlFileParser->setControlFileUrl(url);
    _pControlFileParser->getControlFile();
    return;
}

#ifdef LOGGING_ENABLED
void ZsyncCorePrivate::setShowLog(bool choose)
{
    _pControlFileParser->setShowLog(choose);
    return;
}
#endif // LOGGING_ENABLED

void ZsyncCorePrivate::clear(void)
{
    return;
}

ZsyncCorePrivate::~ZsyncCorePrivate()
{
    /* Free other allocated memory */
    free(_pRsumHash);
    free(_pRanges);
    free(_pBlockHashes);
    free(_pBitHash);
}


/* ZsyncCorePrivate::add_target_block(self, blockid, rsum, checksum)
 * Sets the stored hash values for the given blockid to the given values.
 */
void ZsyncCorePrivate::addTargetFileCheckSumBlocks(zs_blockid b, rsum r, void *checksum)
{
    if (b < _nBlocks) {
        /* Get hash entry with checksums for this block */
        hash_entry *e = &(_pBlockHashes[b]);

        /* Enter checksums */
        memcpy(e->checksum, checksum, _nStrongCheckSumBytes);
        e->r.a = r.a & _pWeakCheckSumMask;
        e->r.b = r.b;

        /* New checksums invalidate any existing checksum hash tables */
        if (_pRsumHash) {
            free(_pRsumHash);
            _pRsumHash = NULL;
            free(_pBitHash);
            _pBitHash = NULL;
        }
    }
}


void ZsyncCorePrivate::addSourceFile(const QString &file)
{
    sourceFiles.append(file);
    return;
}

void ZsyncCorePrivate::completeTargetFileCheckSumBlocks(void)
{
    /*
     * Delete unwanted buffers.
    */
    _pControlFileParser.clear();
    
    for(auto i = 0; i < sourceFiles.size() ; ++i){
        QFile *file = new QFile(sourceFiles.at(i));
        file->open(QIODevice::ReadOnly);
        _nGotBlocks += submitSourceFile(file);
        file->close();
        delete file;
    }
    if(_nGotBlocks >= _nBlocks){
    _pTargetFile->resize(_nTargetFileLength);
    _pTargetFile->close();
    }
    return;
}

/* ZsyncCorePrivate::submitTargetFileBlocks(self, data, startblock, endblock)
 * The data in data[] (which should be (endblock - startblock + 1) * blocksize * bytes)
 * is tested block-by-block as valid data against the target checksums for
 * those blocks and, if valid, accepted and written to the working output.
 *
 * Use this when you have obtained data that you know corresponds to given
 * blocks in the output file (i.e. you've downloaded them from a real copy of
 * the target).
 */
qint32 ZsyncCorePrivate::submitTargetFileBlocks(const unsigned char *data, zs_blockid bfrom, zs_blockid bto)
{
    zs_blockid x;
    unsigned char md4sum[CHECKSUM_SIZE];

    /* Build checksum hash tables if we don't have them yet */
    if (!_pRsumHash)
        if (!buildHash())
            return -1;

    /* Check each block */
    for (x = bfrom; x <= bto; x++) {
        calc_checksum(&md4sum[0], data + ((x - bfrom) << _nBlockShift), _nBlockSize);
        if (memcmp(&md4sum, &(_pBlockHashes[x].checksum[0]), _nStrongCheckSumBytes)) {
            if (x > bfrom)      /* Write any good blocks we did get */
                writeBlocks(data, bfrom, x - 1);
            return -1;
        }
    }

    /* All blocks are valid; write them and update our state */
    writeBlocks( data, bfrom, bto);
    return 0;
}

/* checkCheckSumsOnHashChain(self, &hash_entry, data[], onlyone)
 * Given a hash table entry, check the data in this block against every entry
 * in the linked list for this hash entry, checking the checksums for this
 * block against those recorded in the hash entries.
 *
 * If we get a hit (checksums match a desired block), write the data to that
 * block in the target file and update our state accordingly to indicate that
 * we have got that block successfully.
 *
 * Return the number of blocks successfully obtained.
 */
qint32 ZsyncCorePrivate::checkCheckSumsOnHashChain(const struct hash_entry *e, const unsigned char *data,int onlyone)
{
    unsigned char md4sum[2][CHECKSUM_SIZE];
    signed int done_md4 = -1;
    qint32 got_blocks = 0;
    register rsum rs = _pCurrentWeakCheckSums.first;

    /* This is a hint to the caller that they should try matching the next
     * block against a particular hash entry (because at least _nSeqMatches
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

        // HashHit++
        if (e->r.a != (rs.a & _pWeakCheckSumMask) || e->r.b != rs.b) {
            continue;
        }

        id = getHashEntryBlockId( e);

        if (!onlyone && _nSeqMatches > 1
            && (_pBlockHashes[id + 1].r.a != (_pCurrentWeakCheckSums.second.a & _pWeakCheckSumMask)
                || _pBlockHashes[id + 1].r.b != _pCurrentWeakCheckSums.second.b))
            continue;

        // WeakHit++

        {
            int ok = 1;
            signed int check_md4 = 0;
            zs_blockid next_known = -1;

            /* This block at least must match; we must match at least
             * _nSeqMatches-1 others, which could either be trailing stuff,
             * or these could be preceding blocks that we have verified
             * already. */
            do {
                /* We only calculate the MD4 once we need it; but need not do so twice */
                if (check_md4 > done_md4) {
                    calc_checksum(&md4sum[check_md4][0],
                                  data + _nBlockSize * check_md4,
                                  _nBlockSize);
                    done_md4 = check_md4;
                    // Checksummed++
                }

                /* Now check the strong checksum for this block */
                if (memcmp(&md4sum[check_md4],
                           _pBlockHashes[id + check_md4].checksum,
                           _nStrongCheckSumBytes)) {
                    ok = 0;
                } else if (next_known == -1) {
                }
                check_md4++;
            } while (ok && !onlyone && check_md4 < _nSeqMatches);

            if (ok) {
                qint32 num_write_blocks;

                /* Find the next block that we already have data for. If this
                 * is part of a run of matches then we have this stored already
                 * as ->next_known. */
                zs_blockid next_known = onlyone ? _nNextKnown : nextKnownBlock( id);

                // stronghit++

                if (next_known > id + check_md4) {
                    num_write_blocks = check_md4;

                    /* Save state for this run of matches */
                    _pNextMatch = &(_pBlockHashes[id + check_md4]);
                    if (!onlyone) _nNextKnown = next_known;
                } else {
                    /* We've reached the EOF, or data we already know. Just
                     * write out the blocks we don't know, and that's the end
                     * of this run of matches. */
                    num_write_blocks = next_known - id;
                }

                /* Write out the matched blocks that we don't yet know */
                writeBlocks( data, id, id + num_write_blocks - 1);
                got_blocks += num_write_blocks;
            }
        }
    }
    return got_blocks;
}

/* ZsyncCorePrivate::submitSourceData(self, data, datalen, offset)
 * Reads the supplied data (length datalen) and identifies any contained blocks
 * of data that can be used to make up the target file.
 *
 * offset should be 0 for a new data stream (or if our position in the data
 * stream has been changed and does not match the last call) or should be the
 * offset in the whole source stream otherwise.
 *
 * Returns the number of blocks in the target file that we obtained as a result
 * of reading this buffer.
 *
 * IMPLEMENTATION:
 * We maintain the following state:
 * _nSkip - the number of bytes to skip next time we enter ZsyncCorePrivate::submitSourceData
 *        e.g. because we've just matched a block and the forward jump takes
 *        us past the end of the buffer
 * _pCurrentWeakCheckSums.first - rolling checksum of the first blocksize bytes of the buffer
 * _pCurrentWeakCheckSums.second - rolling checksum of the next blocksize bytes of the buffer (if _nSeqMatches > 1)
 */
qint32 ZsyncCorePrivate::submitSourceData(unsigned char *data,size_t len, off_t offset)
{
    /* The window in data[] currently being considered is
     * [x, x+bs)
     */
    qint32 x = 0;
    register qint32 bs = _nBlockSize;
    qint32 got_blocks = 0;

    if (offset) {
        x = _nSkip;
    } else {
        _pNextMatch = NULL;
    }

    if (x || !offset) {
        _pCurrentWeakCheckSums.first = calc_rsum_block(data + x, bs);
        if (_nSeqMatches > 1)
            _pCurrentWeakCheckSums.second = calc_rsum_block(data + x + bs, bs);
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
            struct rsum c = calc_rsum_block(data + x + bs * k, bs);
            if (c.a != r[k].a || c.b != r[k].b) {
                fprintf(stderr, "rsum miscalc (%d) at %lld\n", k, offset + x);
                exit(3);
            }
        }
#endif

        {
            /* # of blocks of the output file we got from this data */
            qint32 thismatch = 0;
            /* # of blocks to advance if thismatch > 0. Can be less than
             * thismatch as thismatch could be N*blocks_matched, if a block was
             * duplicated to multiple locations in the output file. */
            qint32 blocks_matched = 0;

            /* If the previous block was a match, but we're looking for
             * sequential matches, then test this block against the block in
             * the target immediately after our previous hit. */
            if (_pNextMatch && _nSeqMatches > 1) {
                if (0 != (thismatch = checkCheckSumsOnHashChain( _pNextMatch, data + x, 1))) {
                    blocks_matched = 1;
                }
            }
            if (!thismatch) {
                const struct hash_entry *e;

                /* Do a hash table lookup - first in the _pBitHash (fast negative
                 * check) and then in the rsum hash */
                unsigned hash = _pCurrentWeakCheckSums.first.b;
                hash ^= ((_nSeqMatches > 1) ? _pCurrentWeakCheckSums.second.b
                         : _pCurrentWeakCheckSums.first.a & _pWeakCheckSumMask) << BITHASHBITS;
                if ((_pBitHash[(hash & _pBitHashMask) >> 3] & (1 << (hash & 7))) != 0
                    && (e = _pRsumHash[hash & _pHashMask]) != NULL) {

                    /* Okay, we have a hash hit. Follow the hash chain and
                     * check our block against all the entries. */
                    thismatch = checkCheckSumsOnHashChain( e, data + x, 0);
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
                    _nSkip = x + _nContext - len;
                    return got_blocks;
                }

                /* If we are moving forward just 1 block, we already have the
                 * following block rsum. If we are skipping both, then
                 * recalculate both */
                if (_nSeqMatches > 1 && blocks_matched == 1)
                    _pCurrentWeakCheckSums.first = _pCurrentWeakCheckSums.second;
                else
                    _pCurrentWeakCheckSums.first = calc_rsum_block(data + x, bs);
                if (_nSeqMatches > 1)
                    _pCurrentWeakCheckSums.second = calc_rsum_block(data + x + bs, bs);
                continue;
            }
        }

        /* Else - advance the window by 1 byte - update the rolling checksum
         * and our offset in the buffer */
        {
            unsigned char Nc = data[x + bs * 2];
            unsigned char nc = data[x + bs];
            unsigned char oc = data[x];
            UPDATE_RSUM(_pCurrentWeakCheckSums.first.a, _pCurrentWeakCheckSums.first.b, oc, nc, _nBlockShift);
            if (_nSeqMatches > 1)
                UPDATE_RSUM(_pCurrentWeakCheckSums.second.a, _pCurrentWeakCheckSums.second.b, nc, Nc, _nBlockShift);
        }
        x++;
    }
}

/* ZsyncCorePrivate::submitSourceFile(self, stream, progress)
 * Read the given stream, applying the rsync rolling checksum algorithm to
 * identify any blocks of data in common with the target file. Blocks found are
 * written to our working target output. Progress reports if progress != 0
 */
qint32 ZsyncCorePrivate::submitSourceFile(QFile *file)
{
    /* Track progress */
    qint32 got_blocks = 0;
    off_t in = 0;

    /* Allocate buffer of 16 blocks */
    register qint32 bufsize = _nBlockSize * 16;
    unsigned char *buf = (unsigned char*)malloc(bufsize + _nContext);
    if (!buf)
        return 0;

    /* Build checksum hash tables ready to analyse the blocks we find */
    if (!_pRsumHash)
        if (!buildHash()) {
            free(buf);
            return 0;
        }

    while (!file->atEnd()) {
        size_t len;
        off_t start_in = in;

        /* If this is the start, fill the buffer for the first time */
        if (!in) {
            len = file->read((char*)buf, bufsize);
            in += len;
        }

        /* Else, move the last _nContext bytes from the end of the buffer to the
         * start, and refill the rest of the buffer from the stream. */
        else {
            memcpy(buf, buf + (bufsize - _nContext), _nContext);
            in += bufsize - _nContext;
            len = _nContext + file->read((char*)(buf + _nContext), (bufsize - _nContext));
        }

        if (file->atEnd()) {          /* 0 pad to complete a block */
            memset(buf + len, 0, _nContext);
            len += _nContext;
        }

        /* Process the data in the buffer, and report progress */
        got_blocks += submitSourceData( buf, len, start_in);
    }
    file->close();
    free(buf);
    return got_blocks;
}


/* ZsyncCorePrivate::needed_block_ranges
 * Return the block ranges needed to complete the target file */
void ZsyncCorePrivate::getNeededRanges(void)
{
    qint32 i, n;
    zs_blockid from = 0 , to = _nBlocks;
    QVector<QPair<zs_blockid, zs_blockid>> ret_ranges;

    if (to >= _nBlocks)
        to = _nBlocks;

    ret_ranges.append(qMakePair(from, to));
    ret_ranges.append(qMakePair(0, 0));
    n = 1;
    /* Note r[2*n-1] is the last range in our prospective list */

    for (i = 0; i < _nRanges; i++) {
        if(n == 1) {
            ret_ranges.append(qMakePair(from, to));
        } else {
            ret_ranges.append(qMakePair(0, 0));
        }
        if (_pRanges[2 * i] > ret_ranges.at(n - 1).second) // (2 * n - 1) -> second.
            continue;
        if (_pRanges[2 * i + 1] < from)
            continue;

        /* Okay, they intersect */
        if (n == 1 && _pRanges[2 * i] <= from) {       /* Overlaps the start of our window */
            ret_ranges[0].first = _pRanges[2 * i + 1] + 1;
        } else {
            /* If the last block that we still (which is the last window end -1, due
             * to half-openness) then this range just cuts the end of our window */
            if (_pRanges[2 * i + 1] >= ret_ranges.at(n - 1).second - 1) {
                ret_ranges[n - 1].second = _pRanges[2 * i];
            } else {
                /* In the middle of our range, split it */
                ret_ranges[n].first = _pRanges[2 * i + 1] + 1;
                ret_ranges[n].second = ret_ranges.at(n-1).second;
                ret_ranges[n-1].second = _pRanges[2 * i];
                n++;
            }
        }
    }
    if (n == 1 && ret_ranges.at(0).first >= ret_ranges.at(0).second) {
        n = 0;
        ret_ranges.clear();
    }

    ret_ranges.removeAll(qMakePair(0, 0));
    
    emit receiveNeededBlockRanges(ret_ranges);
    return;
}

/* ZsyncCorePrivate::blocksToDo
 * Return the number of blocks still needed to complete the target file */
qint32 ZsyncCorePrivate::blocksToDo(void)
{
    qint32 i, n = _nBlocks;
    for (i = 0; i < _nRanges; i++) {
        n -= 1 + _pRanges[2 * i + 1] - _pRanges[2 * i];
    }
    return n;
}


/*
 * Private Slots.
*/


/* buildHash(self)
 * Build hash tables to quickly lookup a block based on its rsum value.
 * Returns non-zero if successful.
 */
qint32 ZsyncCorePrivate::buildHash(void)
{
    zs_blockid id;
    qint32 i = 16;

    /* Try hash size of 2^i; step down the value of i until we find a good size
     */
    while ((2 << (i - 1)) > _nBlocks && i > 4)
        i--;

    /* Allocate hash based on rsum */
    _pHashMask = (2 << i) - 1;
    _pRsumHash = (hash_entry**)calloc(_pHashMask + 1, sizeof *(_pRsumHash));
    if (!_pRsumHash)
        return 0;

    /* Allocate bit-table based on rsum */
    _pBitHashMask = (2 << (i + BITHASHBITS)) - 1;
    _pBitHash = (unsigned char*)calloc(_pBitHashMask + 1, 1);
    if (!_pBitHash) {
        free(_pRsumHash);
        _pRsumHash = NULL;
        return 0;
    }

    /* Now fill in the hash tables.
     * Minor point: We do this in reverse order, because we're adding entries
     * to the hash chains by prepending, so if we iterate over the data in
     * reverse then the resulting hash chains have the blocks in normal order.
     * That's improves our pattern of I/O when writing out identical blocks
     * once we are processing data; we will write them in order. */
    for (id = _nBlocks; id > 0;) {
        /* Decrement the loop variable here, and get the hash entry. */
        hash_entry *e = _pBlockHashes + (--id);

        /* Prepend to linked list for this hash entry */
        unsigned h = calcRHash( e);
        e->next = _pRsumHash[h & _pHashMask];
        _pRsumHash[h & _pHashMask] = e;

        /* And set relevant bit in the _pBitHash to 1 */
        _pBitHash[(h & _pBitHashMask) >> 3] |= 1 << (h & 7);
    }
    return 1;
}

/* removeBlockFromHash(self, block_id)
 * Remove the given data block from the rsum hash table, so it won't be
 * returned in a hash lookup again (e.g. because we now have the data)
 */
void ZsyncCorePrivate::removeBlockFromHash(zs_blockid id)
{
    hash_entry *t = &(_pBlockHashes[id]);

    hash_entry **p = &(_pRsumHash[calcRHash( t) & _pHashMask]);

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


/* r = rangeBeforeBlock(self, x)
 * This determines which of the existing known ranges x falls in.
 * It returns -1 if it is inside an existing range (it doesn't tell you which
 *  one; if you already have it, that usually is enough to know).
 * Or it returns 0 if x is before the 1st range;
 * 1 if it is between ranges 1 and 2 (array indexes 0 and 1)
 * ...
 * _nRanges if it is after the last range
 */
qint32 ZsyncCorePrivate::rangeBeforeBlock(zs_blockid x)
{
    /* Lowest number and highest number block that it could be inside (0 based) */
    register qint32 min = 0, max = _nRanges-1;

    /* By bisection */
    for (; min<=max;) {
        /* Range number to compare against */
        register qint32 r = (max+min)/2;

        if (x > _pRanges[2*r+1]) min = r+1;  /* After range r */
        else if (x < _pRanges[2*r]) max = r-1;/* Before range r */
        else return -1;                     /* In range r */
    }

    /* If we reach here, we know min = max + 1 and we were below range max+1
     * and above range min-1.
     * So we're between range max and max + 1
     * So we return max + 1  (return value is 1 based)  ( = min )
     */
    return min;
}

/* addToRanges(rs, blockid)
 * Mark the given blockid as known, updating the stored known ranges
 * appropriately */
void ZsyncCorePrivate::addToRanges(zs_blockid x)
{
    qint32 r = rangeBeforeBlock(x);

    if (r == -1) {
        /* Already have this block */
    } else {
        _nGotBlocks++;

        /* If between two ranges and exactly filling the hole between them,
         * merge them */
        if (r > 0 && r < _nRanges
            && _pRanges[2 * (r - 1) + 1] == x - 1
            && _pRanges[2 * r] == x + 1) {

            // This block fills the gap between two areas that we have got completely. Merge the adjacent ranges
            _pRanges[2 * (r - 1) + 1] = _pRanges[2 * r + 1];
            memmove(&_pRanges[2 * r], &_pRanges[2 * r + 2],
                    (_nRanges - r - 1) * sizeof(_pRanges[0]) * 2);
            _nRanges--;
        }

        /* If adjoining a range below, add to it */
        else if (r > 0 && _nRanges && _pRanges[2 * (r - 1) + 1] == x - 1) {
            _pRanges[2 * (r - 1) + 1] = x;
        }

        /* If adjoining a range above, add to it */
        else if (r < _nRanges && _pRanges[2 * r] == x + 1) {
            _pRanges[2 * r] = x;
        }

        else { /* New range for this block alone */
            _pRanges = (zs_blockid*)
                     realloc(_pRanges,
                             (_nRanges + 1) * 2 * sizeof(_pRanges[0]));
            memmove(&_pRanges[2 * r + 2], &_pRanges[2 * r],
                    (_nRanges - r) * 2 * sizeof(_pRanges[0]));
            _pRanges[2 * r] = _pRanges[2 * r + 1] = x;
            _nRanges++;
        }
    }
}

/* alreadyGotBlock
 * Return true iff blockid x of the target file is already known */
qint32 ZsyncCorePrivate::alreadyGotBlock(zs_blockid x)
{
    return (rangeBeforeBlock(x) == -1);
}

/* next_blockid = nextKnownBlock(rs, blockid)
 * Returns the blockid of the next block which we already have data for.
 * If we know the requested block, it returns the blockid given; otherwise it
 * will return a later blockid.
 * If no later blocks are known, it returns numblocks (i.e. the block after
 * the end of the file).
 */
zs_blockid ZsyncCorePrivate::nextKnownBlock(zs_blockid x)
{
    qint32 r = rangeBeforeBlock(x);
    if (r == -1)
        return x;
    if (r == _nRanges) {
        return _nBlocks;
    }
    /* Else return first block of next known range. */
    return _pRanges[2*r];
}


unsigned ZsyncCorePrivate::calcRHash(const struct hash_entry *const e)
{
    unsigned h = e[0].r.b;

    h ^= ((_nSeqMatches > 1) ? e[1].r.b
          : e[0].r.a & _pWeakCheckSumMask) << BITHASHBITS;

    return h;
}

zs_blockid ZsyncCorePrivate::getHashEntryBlockId(const hash_entry *e)
{
    return e - _pBlockHashes;
}


/* writeBlocks(rcksum_state, buf, startblock, endblock)
 * Writes the block range (inclusive) from the supplied buffer to our
 * under-construction output file */
void ZsyncCorePrivate::writeBlocks(const unsigned char *data, zs_blockid bfrom, zs_blockid bto)
{
    off_t len = ((off_t) (bto - bfrom + 1)) << _nBlockShift;
    off_t offset = ((off_t) bfrom) << _nBlockShift;

    auto pos = _pTargetFile->pos();
    _pTargetFile->seek(offset);
    _pTargetFile->write((char*)data, len);
    _pTargetFile->seek(pos);


    {   /* Having written those blocks, discard them from the rsum hashes (as
         * we don't need to identify data for those blocks again, and this may
         * speed up lookups (in particular if there are lots of identical
         * blocks), and add the written blocks to the record of blocks that we
         * have received and stored the data for */
        int id;
        for (id = bfrom; id <= bto; id++) {
            removeBlockFromHash( id);
            addToRanges( id);
        }
    }
}

void ZsyncCorePrivate::processControlFile(void)
{
    ZsyncRemoteControlFileParserPrivate *controlFile = (ZsyncRemoteControlFileParserPrivate*)QObject::sender();
    _nBlockSize = controlFile->getTargetFileBlockSize();
    _nBlocks    = controlFile->getTargetFileBlocksCount();
    _pWeakCheckSumMask = (controlFile->getWeakCheckSumBytes() < 3 ? 0 : controlFile->getWeakCheckSumBytes() == 3 ? 0xff : 0xffff);
    _nStrongCheckSumBytes = controlFile->getStrongCheckSumBytes();
    _nSeqMatches = controlFile->getConsecutiveMatchNeeded();
    _nTargetFileLength = controlFile->getTargetFileLength();
    
    /* _nSeqMatches is 1 if true; and if true we need 1 block of
     * _nContext to do block matching */
    _nContext = _nBlockSize * _nSeqMatches;

    /* Temporary file to hold the target file as we get blocks for it */
    _pTargetFile = QSharedPointer<QTemporaryFile>(new QTemporaryFile("./" + controlFile->getTargetFileName() + ".xxxxxxxx.part"));
    
    if (!(_nBlockSize & (_nBlockSize - 1)) && _nBlocks) {
        if (!_pTargetFile->open()) {
            /*
             * Error.
             */
            qCritical() << "cannot create temporary file.";
        } else {
            _pTargetFile->setAutoRemove(false);
            /*
             * Before we actually calculate the bit-shift for 
             * the blocksize which is expensive , We can reduce 
             * the computation here with an if control flow.
             * Since zsync is an old program and it uses a static
             * blocksizes for most cases , We can assume that if
             * the blocksize is 2048 bytes , then the bit-shift must be 11 and
             * if the blocksize is 1024 bytes , then the bit-shift must be 10.
             * If those common cases are not met then we move on to the expensive
             * computation to find the bit-shift.
            */
            if(_nBlockSize == 1024){
                _nBlockShift = 10; // log2(1024) = 10.0
            }else if(_nBlockSize == 2048){
                _nBlockShift = 11; // log2(2048) = 11.0
            }else{
            /* Calculate bit-shift for blocksize */
                for (int i = 0; i < 32 ; i++){ // sizeof(int) * 8 = 32 
                    if (_nBlockSize == (1u << i)) {
                        _nBlockShift = i;
                        break;
                    }
                }
            }

            _pBlockHashes = ( hash_entry*)calloc(_nBlocks + _nSeqMatches , sizeof(_pBlockHashes[0]));
            if (_pBlockHashes != NULL) {
                /*
                 * Error.
                */
            }
        }
    }
    
    /*
     * Shoot the control file to get the target file checksum
     * blocks.
    */
    qInfo() << "TargetFile Name:: " << _pTargetFile->fileName();
    controlFile->getTargetFileBlocks();
    return;
}
