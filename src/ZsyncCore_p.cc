#include <ZsyncCore_p.hpp>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

using namespace AppImageUpdaterBridge_p;

#define UPDATE_RSUM(a, b, oldc, newc, bshift) do { (a) += ((unsigned char)(newc)) - ((unsigned char)(oldc)); (b) += (a) - ((oldc) << (bshift)); } while (0)



/* rcksum_calc_rsum_block(data, data_len)
 * Calculate the rsum for a single block of data. */
static rsum __attribute__ ((pure)) calc_rsum_block(const unsigned char *data, size_t len) {
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
static void calc_checksum(unsigned char *c, const unsigned char *data,
                          size_t len) {
    QCryptographicHash ctx(QCryptographicHash::Md4);
    ctx.addData((const char*)data , len);
    auto result = ctx.result();
    memmove(c, result.constData() , sizeof(const char) * result.size());
}

/*
 * Constructor and Destructor 
*/
ZsyncCoreWorker::ZsyncCoreWorker(zs_blockid nblocks, size_t blocksize, int rsum_bytes, int checksum_bytes, int require_consecutive_matches , QObject *parent)
            : QObject(parent),
              blocksize(blocksize),
              blocks(nblocks),
              rsum_a_mask(rsum_bytes < 3 ? 0 : rsum_bytes == 3 ? 0xff : 0xffff),
              checksum_bytes(checksum_bytes),
              seq_matches(require_consecutive_matches)
{
    /* require_consecutive_matches is 1 if true; and if true we need 1 block of
     * context to do block matching */
    context = blocksize * require_consecutive_matches;

    /* Temporary file to hold the target file as we get blocks for it */
    filename = strdup("rcksum-XXXXXX");

    /* Initialise to 0 various state & stats */
    gotblocks = 0;
    memset(&(stats), 0, sizeof(stats));
    ranges = NULL;
    numranges = 0;

    /* Hashes for looking up checksums are generated when needed.
     * So initially store NULL so we know there's nothing there yet.
     */
    rsum_hash = NULL;
    bithash = NULL;

    if (!(blocksize & (blocksize - 1)) && filename != NULL
            && blocks) {
        /* Create temporary file */
        fd = mkstemp(filename);
        if (fd == -1) {
            perror("open");
        }
        else {
            {   /* Calculate bit-shift for blocksize */
                int i;
                for (i = 0; i < 32; i++)
                    if (blocksize == (1u << i)) {
                        blockshift = i;
                        break;
                    }
            }

            blockhashes = ( hash_entry*)
                malloc(sizeof(blockhashes[0]) *
                        (blocks + seq_matches));
            if (blockhashes != NULL){
                /*
                 * Error.
                */
            }

            /* All below is error handling */
        }
    }
    free(filename);
    return;
}


ZsyncCoreWorker::~ZsyncCoreWorker()
{
    /* Free temporary file resources */
    if (fd != -1){
        close(fd);
    }
    if (filename) {
        unlink(filename);
        free(filename);
    }

    /* Free other allocated memory */
    free(rsum_hash);
    free(blockhashes);
    free(bithash);
    free(ranges);            // Should be NULL already
}


/*
 * Public Methods.
*/



/* filename(self)
 * Returns temporary filename to caller as malloced string.
 * Ownership of the file passes to the caller - the function returns NULL if
 * called again, and it is up to the caller to deal with the file. */
char *ZsyncCoreWorker::get_filename(void) {
    char *p = filename;
    filename = NULL;
    return p;
}

/* filehandle(self)
 * Returns the filehandle for the temporary file.
 * Ownership of the handle passes to the caller - the function returns -1 if
 * called again, and it is up to the caller to close it. */
int ZsyncCoreWorker::filehandle(void) {
    int h = fd;
    fd = -1;
    return h;
}


/* ZsyncCoreWorker::add_target_block(self, blockid, rsum, checksum)
 * Sets the stored hash values for the given blockid to the given values.
 */
void ZsyncCoreWorker::add_target_block(zs_blockid b, rsum r, void *checksum) {
    if (b < blocks) {
        /* Get hash entry with checksums for this block */
        hash_entry *e = &(blockhashes[b]);

        /* Enter checksums */
        memcpy(e->checksum, checksum, checksum_bytes);
        e->r.a = r.a & rsum_a_mask;
        e->r.b = r.b;

        /* New checksums invalidate any existing checksum hash tables */
        if (rsum_hash) {
            free(rsum_hash);
            rsum_hash = NULL;
            free(bithash);
            bithash = NULL;
        }
    }
}



/* ZsyncCoreWorker::submit_blocks(self, data, startblock, endblock)
 * The data in data[] (which should be (endblock - startblock + 1) * blocksize * bytes)
 * is tested block-by-block as valid data against the target checksums for
 * those blocks and, if valid, accepted and written to the working output.
 *
 * Use this when you have obtained data that you know corresponds to given
 * blocks in the output file (i.e. you've downloaded them from a real copy of
 * the target).
 */
int ZsyncCoreWorker::submit_blocks(const unsigned char *data, zs_blockid bfrom, zs_blockid bto) {
    zs_blockid x;
    unsigned char md4sum[CHECKSUM_SIZE];

    /* Build checksum hash tables if we don't have them yet */
    if (!rsum_hash)
        if (!build_hash())
            return -1;

    /* Check each block */
    for (x = bfrom; x <= bto; x++) {
        calc_checksum(&md4sum[0], data + ((x - bfrom) << blockshift),
                             blocksize);
        if (memcmp(&md4sum, &(blockhashes[x].checksum[0]), checksum_bytes)) {
            if (x > bfrom)      /* Write any good blocks we did get */
                write_blocks( data, bfrom, x - 1);
            return -1;
        }
    }

    /* All blocks are valid; write them and update our state */
    write_blocks( data, bfrom, bto);
    return 0;
}

/* check_checksums_on_hash_chain(self, &hash_entry, data[], onlyone)
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
int ZsyncCoreWorker::check_checksums_on_hash_chain(const struct hash_entry *e, const unsigned char *data,int onlyone) {
    unsigned char md4sum[2][CHECKSUM_SIZE];
    signed int done_md4 = -1;
    int got_blocks = 0;
    register rsum rs = r[0];

    /* This is a hint to the caller that they should try matching the next
     * block against a particular hash entry (because at least seq_matches
     * prior blocks to it matched in sequence). Clear it here and set it below
     * if and when we get such a set of matches. */
    next_match = NULL;

    /* This is essentially a for (;e;e=e->next), but we want to remove links from
     * the list as we find matches, without keeping too many temp variables.
     */
    rover = e;
    while (rover) {
        zs_blockid id;

        e = rover;
        rover = onlyone ? NULL : e->next;

        /* Check weak checksum first */

        stats.hashhit++;
        if (e->r.a != (rs.a & rsum_a_mask) || e->r.b != rs.b) {
            continue;
        }

        id = get_HE_blockid( e);

        if (!onlyone && seq_matches > 1
            && (blockhashes[id + 1].r.a != (r[1].a & rsum_a_mask)
                || blockhashes[id + 1].r.b != r[1].b))
            continue;

        stats.weakhit++;

        {
            int ok = 1;
            signed int check_md4 = 0;
            zs_blockid next_known = -1;

            /* This block at least must match; we must match at least
             * seq_matches-1 others, which could either be trailing stuff,
             * or these could be preceding blocks that we have verified
             * already. */
            do {
                /* We only calculate the MD4 once we need it; but need not do so twice */
                if (check_md4 > done_md4) {
                    calc_checksum(&md4sum[check_md4][0],
                                         data + blocksize * check_md4,
                                         blocksize);
                    done_md4 = check_md4;
                    stats.checksummed++;
                }

                /* Now check the strong checksum for this block */
                if (memcmp(&md4sum[check_md4],
                     blockhashes[id + check_md4].checksum,
                     checksum_bytes)){
		    printf("Strong checksum mismatch.\n");
                    ok = 0;
		}
                else if (next_known == -1){
		}
              	check_md4++;
            } while (ok && !onlyone && check_md4 < seq_matches);

            if (ok) {
                int num_write_blocks;

                /* Find the next block that we already have data for. If this
                 * is part of a run of matches then we have this stored already
                 * as ->next_known. */
                zs_blockid next_known = onlyone ? next_known : next_known_block( id);

                stats.stronghit += check_md4;

                if (next_known > id + check_md4) {
                    num_write_blocks = check_md4;

                    /* Save state for this run of matches */
                    next_match = &(blockhashes[id + check_md4]);
                    if (!onlyone) next_known = next_known;
                }
                else {
                    /* We've reached the EOF, or data we already know. Just
                     * write out the blocks we don't know, and that's the end
                     * of this run of matches. */
                    num_write_blocks = next_known - id;
                }

                /* Write out the matched blocks that we don't yet know */
                write_blocks( data, id, id + num_write_blocks - 1);
                got_blocks += num_write_blocks;
            }
        }
    }
    return got_blocks;
}

/* ZsyncCoreWorker::submit_source_data(self, data, datalen, offset)
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
 * skip - the number of bytes to skip next time we enter ZsyncCoreWorker::submit_source_data
 *        e.g. because we've just matched a block and the forward jump takes 
 *        us past the end of the buffer
 * r[0] - rolling checksum of the first blocksize bytes of the buffer
 * r[1] - rolling checksum of the next blocksize bytes of the buffer (if seq_matches > 1)
 */
int ZsyncCoreWorker::submit_source_data(unsigned char *data,size_t len, off_t offset) {
    /* The window in data[] currently being considered is 
     * [x, x+bs)
     */
    int x = 0;
    register int bs = blocksize;
    int got_blocks = 0;

    if (offset) {
        x = skip;
    }
    else {
        next_match = NULL;
    }

    if (x || !offset) {
        r[0] = calc_rsum_block(data + x, bs);
        if (seq_matches > 1)
            r[1] = calc_rsum_block(data + x + bs, bs);
    }
    skip = 0;

    /* Work through the block until the current blocksize bytes being
     * considered, starting at x, is at the end of the buffer */
    for (;;) {
        if (x + context == len) {
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
            int thismatch = 0;
            /* # of blocks to advance if thismatch > 0. Can be less than
             * thismatch as thismatch could be N*blocks_matched, if a block was
             * duplicated to multiple locations in the output file. */
            int blocks_matched = 0; 

            /* If the previous block was a match, but we're looking for
             * sequential matches, then test this block against the block in
             * the target immediately after our previous hit. */
            if (next_match && seq_matches > 1) {
                if (0 != (thismatch = check_checksums_on_hash_chain( next_match, data + x, 1))) {
                    blocks_matched = 1;
                }
            }
            if (!thismatch) {
                const struct hash_entry *e;

                /* Do a hash table lookup - first in the bithash (fast negative
                 * check) and then in the rsum hash */
                unsigned hash = r[0].b;
                hash ^= ((seq_matches > 1) ? r[1].b
                        : r[0].a & rsum_a_mask) << BITHASHBITS;
		if ((bithash[(hash & bithashmask) >> 3] & (1 << (hash & 7))) != 0
                    && (e = rsum_hash[hash & hashmask]) != NULL) {

                    /* Okay, we have a hash hit. Follow the hash chain and
                     * check our block against all the entries. */
                    thismatch = check_checksums_on_hash_chain( e, data + x, 0);
                    if (thismatch)
                        blocks_matched = seq_matches;
                }
            }
            got_blocks += thismatch;

            /* If we got a hit, skip forward (if a block in the target matches
             * at x, it's highly unlikely to get a hit at x+1 as all the
             * target's blocks are multiples of the blocksize apart. */
            if (blocks_matched) {
                x += bs + (blocks_matched > 1 ? bs : 0);

                if (x + context > len) {
                    /* can't calculate rsum for block after this one, because
                     * it's not in the buffer. So leave a hint for next time so
                     * we know we need to recalculate */
                    skip = x + context - len;
                    return got_blocks;
                }

                /* If we are moving forward just 1 block, we already have the
                 * following block rsum. If we are skipping both, then
                 * recalculate both */
                if (seq_matches > 1 && blocks_matched == 1)
                    r[0] = r[1];
                else
                    r[0] = calc_rsum_block(data + x, bs);
                if (seq_matches > 1)
                    r[1] = calc_rsum_block(data + x + bs, bs);
                continue;
            }
        }

        /* Else - advance the window by 1 byte - update the rolling checksum
         * and our offset in the buffer */
        {
            unsigned char Nc = data[x + bs * 2];
            unsigned char nc = data[x + bs];
            unsigned char oc = data[x];
            UPDATE_RSUM(r[0].a, r[0].b, oc, nc, blockshift);
            if (seq_matches > 1)
                UPDATE_RSUM(r[1].a, r[1].b, nc, Nc, blockshift);
        }
        x++;
    }
}

/* ZsyncCoreWorker::submit_source_file(self, stream, progress)
 * Read the given stream, applying the rsync rolling checksum algorithm to
 * identify any blocks of data in common with the target file. Blocks found are
 * written to our working target output. Progress reports if progress != 0
 */
int ZsyncCoreWorker::submit_source_file(QFile *file) {
    /* Track progress */
    int got_blocks = 0;
    off_t in = 0;

    /* Allocate buffer of 16 blocks */
    register int bufsize = blocksize * 16;
    unsigned char *buf = (unsigned char*)malloc(bufsize + context);
    if (!buf)
        return 0;

    /* Build checksum hash tables ready to analyse the blocks we find */
    if (!rsum_hash)
        if (!build_hash()) {
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

        /* Else, move the last context bytes from the end of the buffer to the
         * start, and refill the rest of the buffer from the stream. */
        else {
            memcpy(buf, buf + (bufsize - context), context);
            in += bufsize - context;
            len = context + file->read((char*)(buf + context), (bufsize - context));
        }

        if (file->atEnd()) {          /* 0 pad to complete a block */
            memset(buf + len, 0, context);
            len += context;
        }

        /* Process the data in the buffer, and report progress */
        got_blocks += submit_source_data( buf, len, start_in);
    }
    free(buf);
    return got_blocks;
}


/* ZsyncCoreWorker::needed_block_ranges
 * Return the block ranges needed to complete the target file */
zs_blockid *ZsyncCoreWorker::needed_block_ranges(int *num, zs_blockid from, zs_blockid to) {
    int i, n;
    int alloc_n = 100;
    zs_blockid *r = (zs_blockid*)malloc(2 * alloc_n * sizeof(zs_blockid));

    if (!r)
        return NULL;

    if (to >= blocks)
        to = blocks;
    r[0] = from;
    r[1] = to;
    n = 1;
    /* Note r[2*n-1] is the last range in our prospective list */

    for (i = 0; i < numranges; i++) {
        if (ranges[2 * i] > r[2 * n - 1])
            continue;
        if (ranges[2 * i + 1] < from)
            continue;

        /* Okay, they intersect */
        if (n == 1 && ranges[2 * i] <= from) {       /* Overlaps the start of our window */
            r[0] = ranges[2 * i + 1] + 1;
        }
        else {
            /* If the last block that we still (which is the last window end -1, due
             * to half-openness) then this range just cuts the end of our window */
            if (ranges[2 * i + 1] >= r[2 * n - 1] - 1) {
                r[2 * n - 1] = ranges[2 * i];
            }
            else {
                /* In the middle of our range, split it */
                r[2 * n] = ranges[2 * i + 1] + 1;
                r[2 * n + 1] = r[2 * n - 1];
                r[2 * n - 1] = ranges[2 * i];
                n++;
                if (n == alloc_n) {
                    zs_blockid *r2;
                    alloc_n += 100;
                    r2 = (zs_blockid*)realloc(r, 2 * alloc_n * sizeof *r);
                    if (!r2) {
                        free(r);
                        return NULL;
                    }
                    r = r2;
                }
            }
        }
    }
    r = (zs_blockid*)realloc(r, 2 * n * sizeof *r);
    if (n == 1 && r[0] >= r[1])
        n = 0;

    *num = n;
    return r;
}

/* ZsyncCoreWorker::blocks_todo
 * Return the number of blocks still needed to complete the target file */
int ZsyncCoreWorker::blocks_todo(void) {
    int i, n = blocks;
    for (i = 0; i < numranges; i++) {
        n -= 1 + ranges[2 * i + 1] - ranges[2 * i];
    }
    return n;
}


/*
 * Private Slots.
*/


/* build_hash(self)
 * Build hash tables to quickly lookup a block based on its rsum value.
 * Returns non-zero if successful.
 */
int ZsyncCoreWorker::build_hash(void) {
    zs_blockid id;
    int i = 16;

    /* Try hash size of 2^i; step down the value of i until we find a good size
     */
    while ((2 << (i - 1)) > blocks && i > 4)
        i--;

    /* Allocate hash based on rsum */
    hashmask = (2 << i) - 1;
    rsum_hash = (hash_entry**)calloc(hashmask + 1, sizeof *(rsum_hash));
    if (!rsum_hash)
        return 0;

    /* Allocate bit-table based on rsum */
    bithashmask = (2 << (i + BITHASHBITS)) - 1;
    bithash = (unsigned char*)calloc(bithashmask + 1, 1);
    if (!bithash) {
        free(rsum_hash);
        rsum_hash = NULL;
        return 0;
    }

    /* Now fill in the hash tables.
     * Minor point: We do this in reverse order, because we're adding entries
     * to the hash chains by prepending, so if we iterate over the data in
     * reverse then the resulting hash chains have the blocks in normal order.
     * That's improves our pattern of I/O when writing out identical blocks
     * once we are processing data; we will write them in order. */
    for (id = blocks; id > 0;) {
        /* Decrement the loop variable here, and get the hash entry. */
         hash_entry *e = blockhashes + (--id);

        /* Prepend to linked list for this hash entry */
        unsigned h = calc_rhash( e);
        e->next = rsum_hash[h & hashmask];
        rsum_hash[h & hashmask] = e;

        /* And set relevant bit in the bithash to 1 */
        bithash[(h & bithashmask) >> 3] |= 1 << (h & 7);
    }
    return 1;
}

/* remove_block_from_hash(self, block_id)
 * Remove the given data block from the rsum hash table, so it won't be
 * returned in a hash lookup again (e.g. because we now have the data)
 */
void ZsyncCoreWorker::remove_block_from_hash(zs_blockid id) {
     hash_entry *t = &(blockhashes[id]);

     hash_entry **p = &(rsum_hash[calc_rhash( t) & hashmask]);

    while (*p != NULL) {
        if (*p == t) {
            if (t == rover) {
                rover = t->next;
            }
            *p = (*p)->next;
            return;
        }
        else {
            p = &((*p)->next);
        }
    }
}


/* r = range_before_block(self, x)
 * This determines which of the existing known ranges x falls in.
 * It returns -1 if it is inside an existing range (it doesn't tell you which
 *  one; if you already have it, that usually is enough to know).
 * Or it returns 0 if x is before the 1st range;
 * 1 if it is between ranges 1 and 2 (array indexes 0 and 1)
 * ...
 * numranges if it is after the last range
 */
int ZsyncCoreWorker::range_before_block(zs_blockid x) {
    /* Lowest number and highest number block that it could be inside (0 based) */
    register int min = 0, max = numranges-1;

    /* By bisection */
    for (; min<=max;) {
        /* Range number to compare against */
        register int r = (max+min)/2;

        if (x > ranges[2*r+1]) min = r+1;  /* After range r */
        else if (x < ranges[2*r]) max = r-1;/* Before range r */
            else return -1;                     /* In range r */
    }

    /* If we reach here, we know min = max + 1 and we were below range max+1
     * and above range min-1.
     * So we're between range max and max + 1
     * So we return max + 1  (return value is 1 based)  ( = min )
     */
    return min;
}

/* add_to_ranges(rs, blockid)
 * Mark the given blockid as known, updating the stored known ranges
 * appropriately */
void ZsyncCoreWorker::add_to_ranges(zs_blockid x) {
    int r = range_before_block(x);

    if (r == -1) {
        /* Already have this block */
    }
    else {
        gotblocks++;

        /* If between two ranges and exactly filling the hole between them,
         * merge them */
        if (r > 0 && r < numranges
            && ranges[2 * (r - 1) + 1] == x - 1
            && ranges[2 * r] == x + 1) {

            // This block fills the gap between two areas that we have got completely. Merge the adjacent ranges
            ranges[2 * (r - 1) + 1] = ranges[2 * r + 1];
            memmove(&ranges[2 * r], &ranges[2 * r + 2],
                    (numranges - r - 1) * sizeof(ranges[0]) * 2);
            numranges--;
        }

        /* If adjoining a range below, add to it */
        else if (r > 0 && numranges && ranges[2 * (r - 1) + 1] == x - 1) {
            ranges[2 * (r - 1) + 1] = x;
        }

        /* If adjoining a range above, add to it */
        else if (r < numranges && ranges[2 * r] == x + 1) {
            ranges[2 * r] = x;
        }

        else { /* New range for this block alone */
            ranges = (zs_blockid*)
                realloc(ranges,
                        (numranges + 1) * 2 * sizeof(ranges[0]));
            memmove(&ranges[2 * r + 2], &ranges[2 * r],
                    (numranges - r) * 2 * sizeof(ranges[0]));
            ranges[2 * r] = ranges[2 * r + 1] = x;
            numranges++;
        }
    }
}

/* already_got_block
 * Return true iff blockid x of the target file is already known */
int ZsyncCoreWorker::already_got_block(zs_blockid x) {
    return (range_before_block(x) == -1);
}

/* next_blockid = next_known_block(rs, blockid)
 * Returns the blockid of the next block which we already have data for.
 * If we know the requested block, it returns the blockid given; otherwise it
 * will return a later blockid.
 * If no later blocks are known, it returns numblocks (i.e. the block after
 * the end of the file).
 */
zs_blockid ZsyncCoreWorker::next_known_block(zs_blockid x) {
    int r = range_before_block(x);
    if (r == -1)
        return x;
    if (r == numranges) {
        return blocks;
    }
    /* Else return first block of next known range. */
    return ranges[2*r];
}


unsigned ZsyncCoreWorker::calc_rhash(const struct hash_entry *const e) {
    unsigned h = e[0].r.b;

    h ^= ((seq_matches > 1) ? e[1].r.b
        : e[0].r.a & rsum_a_mask) << BITHASHBITS;

    return h;
}

zs_blockid ZsyncCoreWorker::get_HE_blockid(const struct hash_entry *e) {
    return e - blockhashes;
}


/* write_blocks(rcksum_state, buf, startblock, endblock)
 * Writes the block range (inclusive) from the supplied buffer to our
 * under-construction output file */
void ZsyncCoreWorker::write_blocks(const unsigned char *data, zs_blockid bfrom, zs_blockid bto) {
    off_t len = ((off_t) (bto - bfrom + 1)) << blockshift;
    off_t offset = ((off_t) bfrom) << blockshift;

    while (len) {
        size_t l = len;
        int rc;

        /* On some platforms, the bytes-to-write could be more than pwrite(2)
         * will accept. Write in blocks of 2^31 bytes in that case. */
        if ((off_t) l < len)
            l = 0x8000000;

        /* Write */
        rc = pwrite(fd, data, l, offset);
        if (rc == -1) {
            fprintf(stderr, "IO error: %s\n", strerror(errno));
            exit(-1);
        }

        /* Keep track of any data still to do */
        len -= rc;
        if (len) {              /* More to write */
            data += rc;
            offset += rc;
        }
    }

    {   /* Having written those blocks, discard them from the rsum hashes (as
         * we don't need to identify data for those blocks again, and this may
         * speed up lookups (in particular if there are lots of identical
         * blocks), and add the written blocks to the record of blocks that we
         * have received and stored the data for */
        int id;
        for (id = bfrom; id <= bto; id++) {
            remove_block_from_hash( id);
            add_to_ranges( id);
        }
    }
}

