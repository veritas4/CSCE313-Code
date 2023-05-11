#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pcap.h>
#include <ctype.h>

#include "common.h"
#include <iostream>
using std::cout,std::endl,std::cerr;

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

int main()
{
	pcap_t *handle; 
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filter_exp[] = "udp";
	bpf_u_int32 net;

	// Step 1: Open live PCAP session handle on NIC using your interface name
	// TODO
	handle = pcap_open_live("br-477a75ee6d79", BUFSIZ, 1, 1000, errbuf);
	if (handle == nullptr) {
		cerr << "Could not open PCAP session" << endl;
	}

	// Step 2: Compile filter_exp into BPF pseudo-code
	// TODO
	int p_compile = pcap_compile(handle, &fp, filter_exp, 1, net);
	if (p_compile == -1) {
		cerr << "Failure compiling the filter into BPF" << endl;
	}
	
	int p_filter = pcap_setfilter(handle, &fp);
	if (p_filter < 0) {
		cerr << "Failure installing filter"  << endl;
	}

	// Step 3: Capture packets
	printf("Sniffing...\n");
	// TODO
	if (pcap_loop(handle,-1,got_packet,nullptr) < 0) {
		cerr << "No packets sniffed" << endl;
	}

	// Close the PCAP session handle
	// TODO
	pcap_close(handle);

	return 0;
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{

	struct ethheader *eth = (struct ethheader *)packet;

	if (ntohs(eth->ether_type) == 0x800)
	{
		printf("\nReceived packet\n");
		struct ipheader *ip = (struct ipheader *)(packet + sizeof(struct ethheader));
		printf("	From:%s\n", inet_ntoa(ip->iph_sourceip));
		printf("	To:%s\n", inet_ntoa(ip->iph_destip));

		char *data = (char *)packet + sizeof(struct ethheader) + sizeof(struct ipheader) + sizeof(struct udpheader);
		int size_data = ntohs(ip->iph_len) - (sizeof(struct ipheader) + sizeof(struct udpheader));
		if (size_data > 0)
		{
			printf("   Payload (%d bytes):\n", size_data);
			for (int i = 0; i < size_data; i++)
			{
				if (isprint(*data))
					printf("%c", *data);
				else
					printf(".");
				data++;
			}
		}
	}
	return;
}
