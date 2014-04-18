CC = gcc
CFLAGS = -Wall -g
LIBS = -lpcap
TARGETS = pcap_list\
		  pcap_open_live\
		  pcap_open_offline\
		  pcap_read_file_header\
		  test_pcap_save_file

all: $(TARGETS)

pcap_list: pcap_list.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

pcap_open_live: pcap_open_live.c pcap_packet_process.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

pcap_open_offline: pcap_open_offline.c pcap_packet_process.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

pcap_read_file_header: pcap_read_file_header.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

test_pcap_save_file: test_pcap_save_file.c pcap_save_file.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean

clean:
	rm -rf *.o *.swp ~* *~ $(TARGETS)
