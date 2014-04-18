/**
 * File: pcap_save_file.c
 * Author: Joe Shang <shangchuanren@gmail.com>
 * Brief: Save packet into .pcap format file without dependency.
 */

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "pcap_save_file.h"
#include "pcap_header_des.h"

#define OPEN_FLAG   O_CREAT | O_TRUNC | O_RDWR
#define OPEN_MODE   S_IRUSR | S_IWUSR

static int pcap_file_write_file_header(int fd, int linktype)
{
    int ret;
    PcapFileHeader file_header;

    file_header.magic = FILE_MAGIC;
    file_header.version_major = MAJOR_VERSION;
    file_header.version_minor = MINOR_VERSION;
    file_header.thiszone = 0;
    file_header.sigfigs = 0;
    file_header.snaplen = MAX_SNAP_LEN;
    file_header.linktype = linktype;

    ret = write(fd, (char *)&file_header, sizeof(PcapFileHeader));
    return ret;
}

static int pcap_file_write_packet_header(int fd, uint32_t packet_len)
{
    int ret;
    struct timeval tv;
    struct timezone tz;
    PcapPacketHeader packet_header;
    uint32_t cap_len = packet_len;

    if (packet_len > MAX_SNAP_LEN)
    {
        cap_len = MAX_SNAP_LEN;
    }

    gettimeofday(&tv, &tz); 

    packet_header.ts = tv;
    packet_header.caplen = cap_len;
    packet_header.len = packet_len;

    ret = write(fd, (char *)&packet_header, sizeof(PcapPacketHeader));

    return ret;
}

int pcap_file_create(char *dir, int linktype)
{
    int fd;
    char *file_name = NULL;
    char buffer[30];
    time_t t;
    struct tm *time_struct;

    time(&t);
    time_struct = localtime(&t);
    sprintf(buffer, "%d_%d_%d_%d_%d_%d.pcap",
            time_struct->tm_year + 1900,
            time_struct->tm_mon + 1,
            time_struct->tm_mday,
            time_struct->tm_hour,
            time_struct->tm_min,
            time_struct->tm_sec);

    if (dir == NULL)
    {
        file_name = (char *)malloc(strlen(buffer) + 1);
        strcpy(file_name, buffer);
    }
    else
    {
        file_name = (char *)malloc(strlen(dir) + 1 + strlen(buffer) + 1);
        strcpy(file_name, dir);
        strncat(file_name, "/", strlen("/"));
        strncat(file_name, buffer, strlen(buffer));
    }

    printf("%s\n", file_name);

    fd = open(file_name, OPEN_FLAG, OPEN_MODE);
    free(file_name);
    if (fd == -1)
    {
        return -1;
    }

    pcap_file_write_file_header(fd, linktype);

    return fd;
}

int pcap_file_write_packet(int fd, const unsigned char *packet, int packet_len)
{
    int ret;
    ret = pcap_file_write_packet_header(fd, packet_len);
    if (ret == -1)
    {
        return ret;
    }

    ret = write(fd, packet, packet_len);
    return ret;
}

void pcap_file_close(int fd)
{
    close(fd);
}
