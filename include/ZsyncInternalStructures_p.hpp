#ifndef ZSYNC_INTERNAL_STRUCTURES_HPP_INCLUDED
#define ZSYNC_INTERNAL_STRUCTURES_HPP_INCLUDED
#include <QtGlobal>

namespace AppImageUpdaterBridge
{
namespace Private {
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
}
}
#endif // ZSYNC_INTERNAL_STRUCTURES_HPP_INCLUDED
