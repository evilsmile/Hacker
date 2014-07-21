#ifndef ARPSENDER_H
#define ARPSENDER_H

#include <QString>
#include <sys/socket.h>
#include <sys/types.h>

#define ETH_HW_ADDR_LEN 6
#define IP_ADDR_LEN 4
#define MAC_ADDR_LEN 6

#define NIC_NAME "wlan0"

#define ETH_PROTO_IP 0x0800

#define ETH_FRAME_TYPE_ARP 0x0806
#define ETH_FRAME_TYPE_RARP 0x8035

#define HW_ADDR_TYPE_MAC 0x0001 //hw address type of MAC

#define ARP_REQUEST 0x0001
#define ARP_REPLY 0x0002

struct EthHeader{
    quint8 target_hw_addr[ETH_HW_ADDR_LEN];
    quint8 src_hw_addr[ETH_HW_ADDR_LEN];
    quint16 frametype;
};

struct Arp{
    quint16 hw_type;
    quint16 proto_type;
    quint8  hw_addr_size;
    quint8  proto_addr_size;
    quint16 op;
    quint8 send_hw_addr[ETH_HW_ADDR_LEN];
    quint8 send_ip_addr[IP_ADDR_LEN];
    quint8 recv_hw_addr[ETH_HW_ADDR_LEN];
    quint8 recv_ip_addr[IP_ADDR_LEN];
    quint8 padding[18];
};

struct ARPPacket{
    struct EthHeader ethHeader;
    struct Arp arp;
};

class ARPSender
{
public:
    ARPSender();
    ~ARPSender();

    void queryARP();
    void sendARP(const QString& ip);
    void receiveARP();

private:
    int sockfd;
    ARPPacket packet;

    static quint8 self_mac[MAC_ADDR_LEN];
    static quint8 self_ip[IP_ADDR_LEN];

    static void getSelfMAC();
    static void getSelfIP();

    void pack(const QString& ip);
    void unpack();
};



#endif // ARPSENDER_H
