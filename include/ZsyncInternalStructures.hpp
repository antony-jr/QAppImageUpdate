#ifndef ZSYNC_INTERNAL_STRUCTURES_HPP_INCLUDED
#define ZSYNC_INTERNAL_STRUCTURES_HPP_INCLUDED
#include <cstdio>
namespace AppImageUpdaterBridge
{
static constexpr unsigned short STRONG_CHECKSUM_SIZE = 16;
static constexpr unsigned short BIT_HASH_BITS = 3;
typedef int zs_blockid;
struct rsum {
    unsigned short a;
    unsigned short b;
} __attribute__((packed));
struct hash_entry {
    struct hash_entry *next;
    rsum r;
    unsigned char checksum[STRONG_CHECKSUM_SIZE];
};
}
#endif // ZSYNC_INTERNAL_STRUCTURES_HPP_INCLUDED
