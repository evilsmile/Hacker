#include "hostinfo.h"

#include <ifaddrs.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <QDebug>

quint8 HostInfo::self_mac[] = {0};
quint8 HostInfo::self_ip[] = {0};

quint8 HostInfo::gateway_mac[MAC_ADDR_LEN] = {0xBC, 0xD1, 0x77, 0x56, 0x73, 0xDC};

HostInfo::HostInfo()
{

}

void HostInfo::hostInfoInit()
{
    getSelfMAC();
    getSelfIP();
}

void HostInfo::getSelfIP()
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
        memcpy(HostInfo::self_ip, &(req.ifr_addr.sa_data[2]),IP_ADDR_LEN);
    }else{
        qDebug() << "Get IP error";
    }
}

void HostInfo::getSelfMAC()
{
    int sock_tmp = -1;
    struct ifreq req;

    sock_tmp = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(req.ifr_name, NIC_NAME, strlen(NIC_NAME) + 1);

    int err;
    err = ioctl(sock_tmp, SIOCGIFHWADDR, &req);

    close(sock_tmp);

    if(err != -1){
        memcpy(HostInfo::self_mac, &req.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
    }else{
        qDebug() << "Get MAC error";
    }
}



