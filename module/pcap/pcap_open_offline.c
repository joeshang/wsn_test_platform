/*
 * File: pcap_open_offline.c
 * Author: Joe Shang (shangchuanren@gmail.com)
 * Brief: Open a .pcap file to sniff.
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

#include "pcap_packet_process.h"

void packet_callback(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
    process_packet(user, pkthdr, packet);
}

int main(int argc, char *argv[])
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *pd;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s pcap_file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pd = pcap_open_offline(argv[1], errbuf);
    if (pd == NULL)
    {
        fprintf(stderr, "%s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    pcap_loop(pd, -1, packet_callback, NULL);

    pcap_close(pd);

    return 0;
}
