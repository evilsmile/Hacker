#include "icmpsocket.h"
#include <qdebug.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

IcmpSocket::IcmpSocket() : nsend(0), nreceived(0), timer(new QTimer(this))
{
    pid = getpid();
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout_handle()));
}

IcmpSocket::~IcmpSocket()
{
    delete timer;
}

bool IcmpSocket::ping(const QString &ip)
{
    struct hostent *host;
    struct protoent *protocol;
    unsigned long inaddr = 0l;
    int waittime = MAX_WAIT_TIME;
    int size = 50 * 1024;
    if((protocol = getprotobyname("icmp")) == NULL){
           qDebug() << "getprotobyname error";
           return false;
    }

    if((sockfd = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0){
            qDebug() << "socket error";
            return false;
    }

    setuid(getuid());
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;

    inaddr = inet_addr(ip.toLatin1().data());

    memcpy((char*)&dest_addr.sin_addr, (char*)&inaddr, sizeof(inaddr));
    send_packet();
    return recv_packet();
}

/* Checksum calculation */
unsigned short IcmpSocket::cal_chksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while(nleft > 1){
        sum += *w++;
        nleft -= 2;
    }

    if(nleft == 1){
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

/* Set ICMP Header */
int IcmpSocket::pack(int pack_no)
{
    int packsize;
    struct icmp *icmp_packet;
    struct timeval *tval;
    icmp_packet = (struct icmp*)sendpacket;
    icmp_packet->icmp_type = ICMP_ECHO;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_cksum = 0;
    icmp_packet->icmp_seq = pack_no;
    icmp_packet->icmp_id = pid;
    packsize = 8 + datalen;
    tval = (struct timeval*)icmp_packet->icmp_data;
    gettimeofday(tval, 0);
    icmp_packet->icmp_cksum = cal_chksum((unsigned short*)icmp_packet, packsize);
    return packsize;
}

void IcmpSocket::send_packet()
{
    int packetsize;
    int send_count = 0;
    while(send_count < MAX_NO_PACKETS){
        send_count++;
        nsend++;
        packetsize = pack(nsend);
        if(sendto(sockfd, sendpacket, packetsize, 0,
                  (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0){
            qDebug() << "sendto error";
        }
        sleep(1);
    }
}

void IcmpSocket::timeout_handle()
{
    char buf[] = "Timeout";
    qDebug() << buf;
    if(sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0){
        qDebug() << "sendto timeout error";
    }else{
        qDebug() << "release recvfomr";
    }
}

bool IcmpSocket::recv_packet()
{
    int n;
    socklen_t fromlen;
    extern int errno;
    bool reachable = false;

    fromlen = sizeof(src_addr);

    while (nreceived < nsend) {
        //Restart timer
        timer->start(MAX_WAIT_TIME);
        if((n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr*)&src_addr,
                         &fromlen)) < 0){
            qDebug() << "recvfrom error";
            continue;
        }
        //If get response in time, cancel the timer.
        timer->stop();
        gettimeofday(&tvrecv, 0);
        if(unpack(recvpacket, n) == -1)
            reachable |= false;
        else
            reachable |= true;
        nreceived++;
    }
    close(sockfd);
    return reachable;
}

int IcmpSocket::unpack(char *buf, int len)
{
    int iphdrlen;
    struct ip* ip;
    struct icmp* icmp;
    struct timeval* tvsend;
    double rtt;

    ip = (struct ip*)buf;
    iphdrlen = ip->ip_hl << 2;
    icmp = (struct icmp*)(buf + iphdrlen);
    len -= iphdrlen;
    if(len < 8){
        qDebug() << "ICMP packets\'s length is less than 8";
        return -1;
    }

    if((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid)){
        tvsend = (struct timeval*)icmp->icmp_data;
        tv_sub(&tvrecv, tvsend);
        rtt = tvrecv.tv_sec * 1000 + tvrecv.tv_usec/1000;
    //    qDebug() << "icmp_seq: " << icmp->icmp_seq;
    }else{
        return -1;
    }
}

void IcmpSocket::tv_sub(timeval *out, timeval *in)
{
    if((out->tv_usec -= in->tv_usec) < 0){
        --out->tv_sec;
        out->tv_usec += 100000;
    }
    out->tv_sec -= in->tv_sec;
}
