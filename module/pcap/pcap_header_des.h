/**
 * File: pcap_header_des.h
 * Author: Joe Shang <shangchuanren@gmail.com>
 * Brief: Header descriptions of .pcap format file.
 */

#ifndef _PCAP_HEADER_DES_H_
#define _PCAP_HEADER_DES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define FILE_MAGIC      0xa1b2c3d4
#define MAX_SNAP_LEN    65535
#define MAJOR_VERSION   2
#define MINOR_VERSION   4

#define LINKTYPE_NULL           0
#define LINKTYPE_ETHERNET       1
#define LINKTYPE_PPP            9
#define LINKTYPE_IEEE802_11     105
#define LINKTYPE_IEEE802_15_4   195

typedef struct _PcapFileHeader
{
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t thiszone;      /*gmt to local correction */
    uint32_t sigfigs;       /* accuracy of timestamps */
    uint32_t snaplen;       /* max length saved portion of each packet */
    uint32_t linktype;      /* data link type */
}PcapFileHeader;

typedef struct _PcapPacketHeader
{
    struct timeval ts;      /* time stamp */
    uint32_t caplen;        /* length of portion present */
    uint32_t len;           /* length of this packet */
}PcapPacketHeader;

#ifdef __cplusplus
}
#endif

#endif
