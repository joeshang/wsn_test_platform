/**
 * File: pcap_save_file.c
 * Author: Joe Shang <shangchuanren@gmail.com>
 * Brief: Save packet into .pcap format file without dependency.
 */

#ifndef _PCAP_SAVE_FILE_H_
#define _PCAP_SAVE_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

int pcap_file_create(char *dir, int linktype);
int pcap_file_write_packet(int fd, 
        const unsigned char *packet, int packet_len);
void pcap_file_close(int fd);

#ifdef __cplusplus
}
#endif

#endif
