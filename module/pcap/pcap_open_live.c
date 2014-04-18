/*
 * File: pcap_open_live.c
 * Author: Joe Shang (shangchuanren@gmail.com)
 * Brief: Open a live network interface to sniff.
 */

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>

#include "pcap_packet_process.h"

#define SNAP_LEN    65535

void packet_callback(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
    process_packet(user, pkthdr, packet);
}

int main(int argc, char *argv[])
{
    char *dev;
    char errbuf[PCAP_ERRBUF_SIZE];
    char *filter_setting = NULL;
    pcap_t *pd;

    if (argc == 1)
    {
        dev = pcap_lookupdev(errbuf);
        if (dev == NULL)
        {
            fprintf(stderr, "%s\n", errbuf);
            exit(EXIT_FAILURE);
        }
    }
    else if (argc == 2)
    {
        dev = argv[1];
    }
    else if (argc == 3)
    {
        dev = argv[1];
        filter_setting = argv[2];
    }
    else
    {
        fprintf(stderr, "Usage: %s [interface] [filter]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* pcap_t *pcap_open_live(const char *device, int snaplen, int promisc, int to_ms, char *errbuf); */ 
    pd = pcap_open_live(dev, SNAP_LEN, 0, 0, errbuf);
    if (pd == NULL)
    {
        fprintf(stderr, "%s\n", errbuf);
        exit(EXIT_FAILURE);
    }
    printf("open interface %s for capturing\n", dev);

    struct bpf_program filter;
    if (filter_setting)
    {
        /* int pcap_compile(pcap *p, struct bpf_program filter, char *str, int optimize, bpf_u_int32 netmask); */
        pcap_compile(pd, &filter, filter_setting, 1, 0);
        pcap_setfilter(pd, &filter);
    }

    pcap_loop(pd, -1, packet_callback, NULL);

    pcap_close(pd);

    return 0;
}
