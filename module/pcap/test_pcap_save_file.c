/*
 * File: test_pcap_save_file.c
 * Author: Joe Shang (shangchuanren@gmail.com)
 * Brief: Test saving data into .pcap format file.
 */

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pcap_save_file.h"
#include "pcap_header_des.h"

void packet_callback(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
    int fd = *(int *)user;
    pcap_file_write_packet(fd, packet, pkthdr->len);
}

int main(int argc, char *argv[])
{
    int fd;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *pd;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s pcap_file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = pcap_file_create("output", LINKTYPE_ETHERNET);
    if (fd == -1)
    {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    pd = pcap_open_offline(argv[1], errbuf);
    if (pd == NULL)
    {
        fprintf(stderr, "%s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    pcap_loop(pd, -1, packet_callback, (u_char *)&fd);

    pcap_close(pd);
    pcap_file_close(fd);

    return 0;
}
