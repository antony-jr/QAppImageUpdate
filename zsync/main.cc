#include <ZsyncCore_p.hpp>
#include <arpa/inet.h>

using namespace AppImageUpdaterBridge_p;

int main(int ac, char **av)
{
    if(ac == 1) {
        qInfo().noquote() << "Usage: " << av[0] << " [zsync meta file]";
        return 0;
    }
    QString filename(av[1]);
    QFile zsyncFile(filename);
    if(!zsyncFile.open(QIODevice::ReadOnly)) {
        qCritical().noquote() << "Failed to open zsync file.";
        return -1;
    }
    FILE *f = fdopen(zsyncFile.handle(), "r");
    int checksum_bytes = 16, rsum_bytes = 4, seq_matches = 1;
    size_t filesize, blocksize, blocks;

    for (;;) {
        char buf[1024];
        char *p = NULL;
        int l;

        if (fgets(buf, sizeof(buf), f) != NULL) {
            if (buf[0] == '\n')
                break;
            l = strlen(buf) - 1;
            while (l >= 0
                   && (buf[l] == '\n' || buf[l] == '\r' || buf[l] == ' '))
                buf[l--] = 0;

            p = strchr(buf, ':');
        }
        if (p && *(p + 1) == ' ') {
            *p++ = 0;
            p++;
            if (!strcmp(buf, "Length")) {
                filesize = atoll(p);
            } else if (!strcmp(buf, "Blocksize")) {
                blocksize = (size_t)atol(p);
            } else if (!strcmp(buf, "Hash-Lengths")) {
                if (sscanf
                    (p, "%d,%d,%d", &seq_matches, &rsum_bytes,
                     &checksum_bytes) != 3 || rsum_bytes < 1 || rsum_bytes > 4
                    || checksum_bytes < 3 || checksum_bytes > 16
                    || seq_matches > 2 || seq_matches < 1) {
                    fprintf(stderr, "nonsensical hash lengths line %s\n", p);
                }
            } else {
            }
        }
    }

    blocks = (filesize + blocksize - 1) / blocksize;


    qInfo().noquote() << "blocks = " << blocks << " , blocksize = " << blocksize << " , rsum_bytes = " << rsum_bytes
                      << " , checksum_bytes = " << checksum_bytes << " , seq_matches = " << seq_matches;

    ZsyncCoreWorker rstate(blocks , blocksize , rsum_bytes , checksum_bytes , seq_matches);
    /* Now read in and store the checksums */

    zs_blockid id = 0;
    for (; id < blocks; id++) {
        rsum r = { 0, 0 };
        unsigned char checksum[16];

        /* Read in */
        if (fread(((char *)&r) + 4 - rsum_bytes, rsum_bytes, 1, f) < 1
            || fread((void *)&checksum, checksum_bytes, 1, f) < 1) {

            /* Error - free the rcksum_state and tell the caller to bail */
            fprintf(stderr, "short read on control file; %s\n",
                    strerror(ferror(f)));
            return -1;
        }

        /* Convert to host endian and store */
        r.a = ntohs(r.a);
        r.b = ntohs(r.b);
	rstate.add_target_block(id , r , checksum);
    }

    FILE *seed = fopen(av[0] , "r");
    qInfo() << "GOT BLOCKS:: " << rstate.submit_source_file(seed);
    char *rfilename = rstate.get_filename();
    int fd = rstate.filehandle();

    qInfo() << "FILE WRITTEN AT:: " << rfilename;


    return 0;
}
