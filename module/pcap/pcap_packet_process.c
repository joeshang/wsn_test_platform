/**
 * File: pcap_packet_process.c
 * Author: Joe Shang <shangchuanren@gmail.com>
 * Brief: Packet processing program.
 */

#include <pcap.h>
#include <stdio.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>

#define ETHERNET_HDR_LEN    sizeof(struct ether_header)
#define IP_HDR_LEN          sizeof(struct ip)
#define TCP_HDR_LEN         sizeof(struct thdr)
#define UDP_HDR_LEN         sizeof(struct udphdr)

static void print_pcap_packet_info(int count, 
                            unsigned int ref_sec, 
                            unsigned int ref_usec, 
                            unsigned int len)
{
    printf("----------------------- Packet ------------------------\n");
    printf("packet No: %d\n", count);
    printf("packet second: %u.%u\n", ref_sec, ref_usec);
    printf("packet length: %d\n", len);
}

#define MAC_ADDR_LEN    6
static void print_ether_header(const unsigned char *packet)
{
    int i;
    struct ether_header *ehdr = (struct ether_header *)packet;
    unsigned short type = ntohs(ehdr->ether_type);

    printf("------------------- Ethernet Header -------------------\n");

    printf("mac dest address: ");
    for (i = 0; i < MAC_ADDR_LEN; i++)
    {
       printf("%02x", ehdr->ether_dhost[i]);
       if (i != MAC_ADDR_LEN - 1)
       {
           printf(":");
       }
       else
       {
           printf("\n");
       }
    }

    printf("mac source address: ");
    for (i = 0; i < MAC_ADDR_LEN; i++)
    {
       printf("%02x", ehdr->ether_shost[i]);
       if (i != MAC_ADDR_LEN - 1)
       {
           printf(":");
       }
       else
       {
           printf("\n");
       }
    }

    printf("protocol: ");
    switch (type)
    {
        case ETHERTYPE_IP:
            printf("IP protocol\n");
            break;
        case ETHERTYPE_ARP:
            printf("ARP protocol\n");
            break;
        case ETHERTYPE_REVARP:
            printf("RARP protocol\n");
            break;
        default:
            printf("can not recoginize protocol\n");
            break;
    }
}

static void print_ip_header(const unsigned char *packet)
{
    struct ip *iphdr = (struct ip *)(packet + ETHER_HDR_LEN);

    printf("--------------------- IP Header -----------------------\n");

    printf("version: %d\n", iphdr->ip_v);
    printf("header length: %d bytes\n", iphdr->ip_hl * 4);
    printf("type of service: %d\n", iphdr->ip_tos);
    printf("total length: %d\n", ntohs(iphdr->ip_len));
    printf("identification: %d\n", ntohs(iphdr->ip_id));

    printf("flags:");
    if ((iphdr->ip_off & ~IP_OFFMASK) & IP_RF)
    {
        printf(" RESERVED");
    }
    if ((iphdr->ip_off & ~IP_OFFMASK) & IP_DF)
    {
        printf(" DONT\n");
    }
    if ((iphdr->ip_off & ~IP_OFFMASK) & IP_MF)
    {
        printf(" MORE\n");
    }
    printf("\n");

    printf("fragment offset: %d\n", ntohs(iphdr->ip_off));
    printf("time to live: %d\n", iphdr->ip_ttl);

    printf("protocol: [%d]", iphdr->ip_p);
    switch (iphdr->ip_p)
    {
        case IPPROTO_TCP:
            printf("TCP protocol\n");
            break;
        case IPPROTO_UDP:
            printf("UDP protocol\n");
            break;
        case IPPROTO_ICMP:
            printf("ICMP protocol\n");
            break;
        case IPPROTO_IGMP:
            printf("IGMP protocol\n");
            break;
        default:
            printf("other protocols\n");
            break;
    }

    printf("checksum: %d\n", ntohs(iphdr->ip_sum));

    char addr_buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(iphdr->ip_src), addr_buf, INET_ADDRSTRLEN);
    printf("source address: %s\n", addr_buf);
    inet_ntop(AF_INET, &(iphdr->ip_src), addr_buf, INET_ADDRSTRLEN);
    printf("dest address: %s\n", addr_buf);
}

static void print_tcp_header(const unsigned char *packet)
{
    struct tcphdr *thdr = (struct tcphdr *)(packet + ETHERNET_HDR_LEN + IP_HDR_LEN);

    printf("--------------------- TCP Header ----------------------\n");

    printf("source port: %d\n", htons(thdr->source));
    printf("destination port: %d\n", htons(thdr->dest));
    printf("sequence number: %d\n", htonl(thdr->seq));
    printf("acknowledgement number: %d\n", htonl(thdr->ack_seq));
    printf("data offset: %d bytes\n", thdr->doff * 4);
    printf("window size: %d\n", htons(thdr->window));

    printf("flags:");
    if (thdr->fin)
    {
        printf(" FIN");
    }
    if (thdr->syn)
    {
        printf(" SYN");
    }
    if (thdr->rst)
    {
        printf(" RST");
    }
    if (thdr->psh)
    {
        printf(" PUSH");
    }
    if (thdr->ack)
    {
        printf(" ACK");
    }
    if (thdr->urg)
    {
        printf(" URG");
    }
    printf("\n");

    printf("urgent pointer: %d\n", htons(thdr->urg_ptr));
}

static void print_hex_ascii_line(const unsigned char *payload, int len, int offset)
{
    int i;
    int gap;
    const unsigned char *ch;

    printf("%04x   ", offset);

    ch = payload;
    for (i = 0; i < len; i++, ch++)
    {
        printf("%02x ", *ch);

        /* print extra space after 8th byte for visual aid */
        if (i == 7)
        {
            printf(" ");
        }
    }

    /* fill hex gap with spaces if not full line */
    if (len < 16)
    {
        if (len < 8)
        {
            printf(" ");
        }

        gap = 16 - len;
        for (i = 0; i < gap; i++)
        {
            printf("   ");
        }
    }
    printf("   ");

    /* ascii (if printable) */
    ch = payload;
    for (i = 0; i < len; i++, ch++)
    {
        if (isprint(*ch))
        {
            printf("%c", *ch);
        }
        else
        {
            printf(".");
        }
    }

    printf("\n");
} 

static void print_payload(const unsigned char *payload, int len)
{
    int len_remain = len;
    int line_width = 16;
    int offset = 0;
    const unsigned char *line_start = payload;

    if (len <= 0)
    {
        return;
    }

    printf("---------------------- Payload ------------------------\n");

    while (len_remain > line_width)
    {
        print_hex_ascii_line(line_start, line_width, offset);
        len_remain = len_remain - line_width;
        line_start = line_start + line_width;
        offset = offset + line_width;
    }

    print_hex_ascii_line(line_start, len_remain, offset);
}

void process_packet(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
    static int packet_count = 0;
    static struct timeval first_packet_ts;

    time_t ref_tv_sec;
    time_t ref_tv_usec;

    if (packet_count == 0)
    {
        first_packet_ts = pkthdr->ts;
        ref_tv_sec = 0;
        ref_tv_usec = 0;
    }
    else
    {
        if (first_packet_ts.tv_usec > pkthdr->ts.tv_usec)
        {
            ref_tv_sec = pkthdr->ts.tv_sec - first_packet_ts.tv_sec - 1;
            ref_tv_usec = 1000000 - first_packet_ts.tv_usec + pkthdr->ts.tv_usec;
        }
        else
        {
            ref_tv_sec = pkthdr->ts.tv_sec - first_packet_ts.tv_sec;
            ref_tv_usec = pkthdr->ts.tv_usec - first_packet_ts.tv_usec;
        }
    }

    printf("=======================================================\n");

    /* print headers */
    print_pcap_packet_info(packet_count,
                           ref_tv_sec,
                           ref_tv_usec,
                           pkthdr->len);

    /* print payload */
    print_payload(packet, pkthdr->len);

    /* print headers */
    print_ether_header(packet);
    print_ip_header(packet);
    print_tcp_header(packet);
 
    printf("=======================================================\n\n");

    packet_count++;
}

