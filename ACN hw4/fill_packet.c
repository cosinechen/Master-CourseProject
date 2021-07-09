#include "fill_packet.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>

void
fill_iphdr (struct ip *ip_hdr , const char* dst_ip ,const char* src_ip, int totalLen)
{
	ip_hdr -> ip_v = 4;
	ip_hdr -> ip_hl = 5;
	ip_hdr -> ip_tos = 0;
	ip_hdr -> ip_len = htons(totalLen);
	ip_hdr -> ip_id = 0;
	ip_hdr -> ip_off = htons(0x4000); //don't fragment
	ip_hdr -> ip_ttl = 1;
	ip_hdr -> ip_p = IPPROTO_ICMP;
	//ip_hdr -> ip_sum = ;
	inet_aton(src_ip, &(ip_hdr -> ip_src));
	inet_aton(dst_ip, &(ip_hdr -> ip_dst));
	//printf("%s",inet_ntoa(ip_hdr -> ip_dst));
}

void
fill_icmphdr (struct icmphdr* icmp_hdr)
{
	icmp_hdr -> type = ICMP_ECHO;
	icmp_hdr -> code = 0;
	icmp_hdr -> checksum = 0;
	icmp_hdr -> un.echo.id = getpid();
	icmp_hdr -> un.echo.sequence = 0;
}

u16 fill_cksum(struct icmphdr* icmp_hdr)
{
	return (u16)cksum((unsigned short *)icmp_hdr, ICMP_PACKET_SIZE);
}

unsigned short cksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }

    if (nleft == 1)
    {
      *(unsigned char *)(&answer) = *(unsigned char *)w;
      	sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return (answer);
}
