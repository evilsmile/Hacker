#include "arpsender.h"
#include <QDebug>

#include <cstdlib>
#include <cstdio>

#include <unistd.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

ARPSender::ARPSender()
{
    getSelfIP();
    getSelfMAC();
}

ARPSender::~ARPSender()
{

}

void ARPSender::queryARP()
{
   sendARP("192.168.1.102");
   receiveARP();
}

void ARPSender::sendARP(const QString &ip)
{
    pack(ip);

    struct sockaddr_ll eth_info;
    memset(&eth_info, 0, sizeof(eth_info));
    eth_info.sll_family = PF_PACKET;
    eth_info.sll_ifindex = if_nametoindex(NIC_NAME);
    eth_info.sll_pkttype = PACKET_BROADCAST;
    eth_info.sll_halen = ETH_ALEN;
    eth_info.sll_protocol = htons(ETH_P_ARP);
    for(int i = 0; i < ETH_ALEN; i++){
        eth_info.sll_addr[i] = self_mac[i];
    }
 //   eth_info.sll_hatype = ARPHRD_ETHER;

    sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if(sockfd < 0){
        qDebug() << "socket create error";
        return;
    }

    if(bind(sockfd, (struct sockaddr*)(&eth_info), sizeof(eth_info))){
        perror("bind");
        return;
    }

    int num = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)(&eth_info), sizeof(eth_info));

    if(num < 0){
        qDebug() << "arp send error. " ;
        perror("sendto");
        return;
    }

}


void ARPSender::pack(const QString &ip)
{
    //-----Fill ethernet header

    //Target MAC address
    for(int i = 0; i < MAC_ADDR_LEN; i++){
        packet.ethHeader.target_hw_addr[i] = 0xFF;
    }
    //Source MAC address
    for(int i = 0; i < MAC_ADDR_LEN; i++){
        packet.ethHeader.src_hw_addr[i] = self_mac[i];
    }
    //Frame type
    packet.ethHeader.frametype = htons(ETH_FRAME_TYPE_ARP);

    //------Fill ARP header

    //HW address type of MAC
    packet.arp.hw_type = htons(HW_ADDR_TYPE_MAC);

    //Protocol type
    packet.arp.proto_type = htons(ETH_PROTO_IP);

    //HW address length
    packet.arp.hw_addr_size = MAC_ADDR_LEN;
    //Protocol length
    packet.arp.proto_addr_size = IP_ADDR_LEN;

    //Operation type
    packet.arp.op = htons(ARP_REQUEST);

    for(int i = 0; i < MAC_ADDR_LEN; i++){
        packet.arp.send_hw_addr[i] = self_mac[i];
        packet.arp.recv_hw_addr[i] = 0x00;
    }

    struct in_addr dest_ip;
    quint8 dest_ip_array[IP_ADDR_LEN];

    inet_aton(ip.toLatin1().data(), &dest_ip);
    dest_ip_array[0] = (dest_ip.s_addr) & 0xff;
    dest_ip_array[1] = (dest_ip.s_addr >> 8) & 0xff;
    dest_ip_array[2] = (dest_ip.s_addr >> 16) & 0xff;
    dest_ip_array[3] = (dest_ip.s_addr >> 24) & 0xff;


    for(int i = 0 ;i < IP_ADDR_LEN; i++){
        packet.arp.send_ip_addr[i] = self_ip[i];
        packet.arp.recv_ip_addr[i] = dest_ip_array[i];
    }

    bzero(&packet.arp.padding, sizeof(packet.arp.padding));
}


void ARPSender::receiveARP()
{
    int num = -1;
    num = recvfrom(sockfd, &packet, sizeof(packet), 0, NULL, 0);
    if(num < 0){
        qDebug() << "recvfrom error";
        return;
    }else{
        qDebug()  << "received " << num << " bytes. mac address: ";
        unpack();
    }
    close(sockfd);
}

void ARPSender::unpack()
{
    for(int i = 0; i < 6; i++){
        qDebug() << hex << packet.arp.send_hw_addr[i] << ":";
    }
}


quint8 ARPSender::self_ip[] = {0};

void ARPSender::getSelfIP()
{
/* //Solution1: by getifaddr()
    struct ifaddrs *if_addresses = NULL;

    getifaddrs(&if_addresses);

    while(if_addresses != NULL){
           //If it's wlan0
           if((if_addresses->ifa_addr->sa_family == AF_INET)
                   && (strncmp(NIC_NAME, if_addresses->ifa_name, strlen(NIC_NAME)) == 0)){
               quint32 sock_addr = 0;

               sock_addr = (((struct sockaddr_in*)if_addresses->ifa_addr)->sin_addr.s_addr);
               self_ip[0] = sock_addr & 0xff;
               self_ip[1] = (sock_addr >> 8) & 0xff;
               self_ip[2] = (sock_addr >> 16) & 0xff;
               self_ip[3] = (sock_addr >> 24) & 0xff;

               //Debug info
               char buf[100];
               inet_ntop(AF_INET, &sock_addr, buf, sizeof(buf));
               qDebug() << "get ip: " << buf << if_addresses->ifa_name;
           }
           if_addresses = if_addresses->ifa_next;
    }
    freeifaddrs(if_addresses);
    */

    //Solution2: by ioctl()
    int sock_tmp = -1;
    struct ifreq req;

    sock_tmp = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(req.ifr_name, NIC_NAME, strlen(NIC_NAME) + 1);

    int err;
    err = ioctl(sock_tmp, SIOCGIFADDR, &req);

    close(sock_tmp);

    if(err != -1){
        memcpy(self_ip, &(req.ifr_addr.sa_data[2]),IP_ADDR_LEN);
        qDebug() << hex << self_ip[0] << "." << self_ip[1] << "." << self_ip[2] << "." << self_ip[3];
    }else{
        qDebug() << "Get IP error";
    }
}

quint8 ARPSender::self_mac[] = {0};
void ARPSender::getSelfMAC()
{
    int sock_tmp = -1;
    struct ifreq req;

    sock_tmp = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(req.ifr_name, NIC_NAME, strlen(NIC_NAME) + 1);


    int err;
    err = ioctl(sock_tmp, SIOCGIFHWADDR, &req);

    close(sock_tmp);

    if(err != -1){
        memcpy(self_mac, &req.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
    }else{
        qDebug() << "Get MAC error";
    }
}
