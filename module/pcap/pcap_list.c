/*
 * File: pcap_list.c
 * Author: Joe Shang (shangchuanren@gmail.com)
 * Brief: list all network interface with pcap.
 */

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    int ret;
    char *dev;
    char net[INET_ADDRSTRLEN];
    char mask[INET_ADDRSTRLEN];
    char errbuf[PCAP_ERRBUF_SIZE];
    bpf_u_int32 netp;
    bpf_u_int32 maskp;

    pcap_if_t *alldevsp;
    pcap_if_t *interface;
    
    printf("list first non-loopback network interface available for libpcap:\n");

    dev = pcap_lookupdev(errbuf);
    if (dev == NULL)
    {
        fprintf(stderr, "%s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    printf("interface: %s\n", dev);

    ret = pcap_lookupnet(dev, &netp, &maskp, errbuf);
    if (ret == -1)
    {
        fprintf(stderr, "%s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    if (inet_ntop(AF_INET, &netp, net, INET_ADDRSTRLEN) == NULL)
    {
        perror("inet_ntop(net)");
        exit(EXIT_FAILURE);
    }
    printf("net: %s\n", net);

    if (inet_ntop(AF_INET, &maskp, mask, INET_ADDRSTRLEN) == NULL)
    {
        perror("inet_ntoa(mask)");
        exit(EXIT_FAILURE);
    }
    printf("mask: %s\n", mask);

    printf("\n");
    printf("list all network interface:\n");
    pcap_findalldevs(&alldevsp, errbuf);
    
    interface = alldevsp;
    while (interface != NULL)
    {
        printf("name: %s\n", interface->name);

        interface = interface->next;  
    }

    pcap_freealldevs(alldevsp);

    return 0;
}
