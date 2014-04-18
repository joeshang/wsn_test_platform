#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pcap_header_des.h"

int main(int argc, char *argv[])
{
    int fd;
    PcapFileHeader file_header;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((fd = open(argv[1], O_RDWR)) == -1)
    {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    read(fd, (char *)&file_header, sizeof(PcapFileHeader));

    printf("magic: %x\n", file_header.magic);
    printf("major version: %u\n", file_header.version_major);
    printf("minor version: %u\n", file_header.version_minor);
    printf("this zone: %u\n", file_header.thiszone);
    printf("accuracy of timestamps: %u\n", file_header.sigfigs);
    printf("snap length: %u\n", file_header.snaplen);
    printf("link type: %u\n", file_header.linktype);

    return 0;
}
