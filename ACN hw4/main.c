#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/time.h>

#include "fill_packet.h"
#include "pcap.h"


#define DEVICE_NAME "ens33"

pid_t pid;

void print_usage(){
	printf("Please enter the following format\n");
	printf("sudo ./ipscanner –i [Network Interface Name] -t [timeout(ms)]\n");
}

int main(int argc, char* argv[])
{
	int sockfd;
	int on = 1;

	pid = getpid();
	struct sockaddr_in dst,src;
	struct ifreq ifr_ip,ifr_mask;
	struct in_addr host_ip,host_netmask;
	myicmp *packet = (myicmp*)malloc(PACKET_SIZE);
	int count = DEFAULT_SEND_COUNT;
	int timeout = DEFAULT_TIMEOUT;

	//printf("%s\n",ifr_ip.ifr_name);

	//in pcap.c, initialize the pcap
	//pcap_init(ifr_ip.ifr_name, timeout);

	if(getuid()!=0 || geteuid()!=0){
		printf("ERROR: You must be root to use this tool!\n");
		exit(1);
	}

	if((sockfd = socket(AF_INET, SOCK_RAW , IPPROTO_RAW)) < 0){
		perror("socket");
		exit(1);
	}

	if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}	

	/*memset(&ifr_ip,0,sizeof(ifr_ip));
	memcpy(ifr_ip.ifr_name,DEVICE_NAME,strlen(DEVICE_NAME));*/	
	struct sockaddr_in *sin_ptr;		

	//get ip address
	strcpy(ifr_ip.ifr_name, argv[2]);
	if(ioctl(sockfd, SIOCGIFADDR, &ifr_ip) == -1){
		perror("ioctl() get ip error\n");
		host_ip.s_addr = 0;
		//exit(1);
	}
	else{
		sin_ptr = (struct sockaddr_in*)&ifr_ip.ifr_addr;
		host_ip = sin_ptr -> sin_addr;
		//printf("%s\n",inet_ntoa(host_ip));
	}

	//get network mask
	memset(&ifr_mask,0,sizeof(ifr_mask));
	strcpy(ifr_mask.ifr_name,argv[2]);

	if(ioctl(sockfd, SIOCGIFNETMASK, &ifr_mask) == -1){
		perror("ioctl() get network mask error\n");
		host_netmask.s_addr = 0;
		//exit(1);
	}
	else{
		sin_ptr = (struct sockaddr_in*)&ifr_mask.ifr_addr;
		host_netmask = sin_ptr -> sin_addr;
		//printf("%s\n",inet_ntoa(host_netmask));
	}

	//f,"%d.%d.d.%d",n
	int ip1,ip2,ip3,ip4;
	sscanf(inet_ntoa(host_ip),"%d.%d.%d.%d",&ip1,&ip2,&ip3,&ip4);
	//printf("%d %d %d %d\n",ip1,ip2,ip3,ip4);

	int mask1,mask2,mask3,mask4;
	sscanf(inet_ntoa(host_netmask),"%d.%d.%d.%d",&mask1,&mask2,&mask3,&mask4);
	//printf("%d %d %d %d\n",mask1,mask2,mask3,mask4);

	//printf("%d\n",packet->ip_hdr.ip_hl);
	char dstip[20];
	//char *dstip;
	struct timeval tv;
	int sequence=0;

	if(strcmp(argv[0],"./ipscanner")==0 && strcmp(argv[1],"-i")==0 && strcmp(argv[3],"-t")==0 ){
		//char buf[512];
		//int dst_len = sizeof(dst);
		int i;
		for(i=1; i<=254; i++){
			sprintf(dstip,"%d.%d.%d.%d",ip1,ip2,ip3,i);
			sequence++;	
			timeout = atoi(argv[4]);
				
			dst.sin_family = AF_INET;

			//dst.sin_addr.s_addr = inet_addr(dstip);
			//printf("dstip = %s\n",*dstip);
			//fill icmp packet
			fill_iphdr(&packet->ip_hdr, dstip, inet_ntoa(host_ip), sizeof(packet->ip_hdr));	
			fill_icmphdr(&packet->icmp_hdr);
			packet->data[0]="M";
			packet->data[1]="0";
			packet->data[2]="8";
			packet->data[3]="3";
			packet->data[4]="0";
			packet->data[5]="4";
			packet->data[6]="0";
			packet->data[7]="0";
			packet->data[8]="1";
			packet->data[9]="5";
			packet->icmp_hdr.checksum = fill_cksum(&(packet->icmp_hdr));
			//printf("%d %d %d\n", packet->ip_hdr.ip_v, packet->ip_hdr.ip_hl, packet->ip_hdr.ip_tos);
			printf("PING %s (data size = %ld, id = 0x%x, seq = %d, timeout = %d ms)\n" , dstip, sizeof(packet->data), pid, sequence, timeout);

			gettimeofday(&tv,NULL);			
			
			/*if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
			{
				perror("setsockopt");
				exit(1);
			}*/	
			//memset(packet->icmp_hdr,0,sizeof(packet->icmp_hdr));
			long int sendtime = tv.tv_sec*1000 + tv.tv_usec/1000;
			if(sendto(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *)&dst, sizeof(dst)) < 0)
			{
				perror("sendto");
				exit(1);
			}			
			
			myicmp recvpkt;//= (myicmp*)malloc(PACKET_SIZE);
			//myicmp recvpkt;
			//if(recvfrom(sockfd, &buf, sizeof(buf), 0, (struct sockaddr *)&dst, (socklen_t *)&dst_len)<0 ){
			int sockfd_recv = 0;
			if( (sockfd_recv = socket(AF_INET, SOCK_RAW , IPPROTO_ICMP)) < 0){
				perror("receive socket error");
				exit(1);
			}
			/*struct timeval tw;
			tw.tv_sec = timeout/1000;
			tw.tv_usec = timeout/100;
			//printf("data = %d\n",*recvpkt.data);
			//printf("value = %ld\n",recvfrom(sockfd_recv, (void *)&recvpkt, sizeof(recvpkt), 0, NULL, NULL));
			if(setsockopt(sockfd_recv, SOL_SOCKET, SO_RCVTIMEO, &tw, sizeof(tw)) < 0)
			{
				perror("setsockopt");
				exit(1);
			}*/
			
			if(recvfrom(sockfd_recv, (void *)&recvpkt, sizeof(recvpkt), 0, NULL, NULL) <0 ){
				perror("recvfrom error");
				exit(1);
			}
			/*else{
				printf("TEST\n");
			}*/			
			
			gettimeofday(&tv,NULL);
			long int recvtime = tv.tv_sec*1000 + tv.tv_usec/1000;
			//long int RTT = tv.tv_sec*1000 + tv.tv_usec/1000;
			long int RTT = recvtime - sendtime;
			if(RTT < timeout && recvpkt.icmp_hdr.type == ICMP_ECHOREPLY){
				printf("Reply from : %s, time : %ld ms\n" , dstip, RTT);
			}
			else{
				printf("Destination unreachable.\n");
			}
			fflush(stdout);
			//free(dstip);
			//fork();
		}	
	}
	else{
		print_usage();
	}
	/*
	 *   Use "sendto" to send packets, and use "pcap_get_reply"(in pcap.c)
		 or use the standard socket like the one in the ARP homework
 	 *   to get the "ICMP echo response" packets
	 *	 You should reset the timer every time before you send a packet.
	 */
	free(packet);

	return 0;
}