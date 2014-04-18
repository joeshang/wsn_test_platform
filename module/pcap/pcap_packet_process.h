/**
 * File: pcap_packet_process.h
 * Author: Joe Shang <shangchuanren@gmail.com>
 * Brief: Packet processing program.
 */

#ifndef _PCAP_PACKET_PROCESS_H_
#define _PCAP_PACKET_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif


void process_packet(u_char *user, 
                    const struct pcap_pkthdr *pkthdr, 
                    const u_char *packet);

#ifdef __cplusplus
}
#endif

#endif
