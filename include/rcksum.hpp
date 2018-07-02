#include <stdio.h>

struct rcksum_state;

typedef int zs_blockid;

struct rsum {
	unsigned short	a;
	unsigned short	b;
} __attribute__((packed));

#define CHECKSUM_SIZE 16

struct rcksum_state* rcksum_init(zs_blockid nblocks, size_t blocksize, int rsum_butes, int checksum_bytes, int require_consecutive_matches);
void rcksum_end(struct rcksum_state* z);

char* rcksum_filename(struct rcksum_state* z);
int rcksum_filehandle(struct rcksum_state* z);

void rcksum_add_target_block(struct rcksum_state* z, zs_blockid b, struct rsum r, void* checksum);

int rcksum_submit_blocks(struct rcksum_state* z, const unsigned char* data, zs_blockid bfrom, zs_blockid bto);
int rcksum_submit_source_data(struct rcksum_state* z, unsigned char* data, size_t len, off_t offset);
int rcksum_submit_source_file(struct rcksum_state* z, FILE* f, int progress);

/* This reads back in data which is already known. */
int rcksum_read_known_data(struct rcksum_state* z, unsigned char* buf, off_t offset, size_t len);

/* rcksum_needed_block_ranges tells you what blocks, within the given range,
 * are still unknown. It returns a list of block ranges in r[]
 * (at most max ranges, so spece for 2*max elements must be there)
 * these are half-open ranges, so r[0] <= x < r[1], r[2] <= x < r[3] etc are needed */
zs_blockid* rcksum_needed_block_ranges(const struct rcksum_state* z, int* num, zs_blockid from, zs_blockid to);
int rcksum_blocks_todo(const struct rcksum_state*);

/* For preparing rcksum control files - in both cases len is the block size. */
struct rsum __attribute__((pure)) rcksum_calc_rsum_block(const unsigned char* data, size_t len);
void rcksum_calc_checksum(unsigned char *c, const unsigned char* data, size_t len);
