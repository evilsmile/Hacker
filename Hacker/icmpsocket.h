#ifndef ICMPSOCKET_H
#define ICMPSOCKET_H

#include <QString>
#include <QObject>
#include <QTimer>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define PACKET_SIZE 4096
#define MAX_WAIT_TIME 5000
#define MAX_NO_PACKETS 3

const int datalen = 56;

class IcmpSocket : public QObject
{
    Q_OBJECT
public:
    IcmpSocket();
    ~IcmpSocket();
    bool ping(const QString& ip);
    void send_packet(void);
    bool recv_packet(void);

private slots:
    void timeout_handle();

private:
    void tv_sub(struct timeval* out, struct timeval *in);
    unsigned short cal_chksum(unsigned short *addr, int len);
    int pack(int pack_no);
    int unpack(char *buf, int len);

    int nsend;
    int nreceived;
    int sockfd;
    char sendpacket[PACKET_SIZE];
    char recvpacket[PACKET_SIZE];
    struct sockaddr_in dest_addr;
    pid_t pid;
    struct sockaddr_in src_addr;
    struct timeval tvrecv;

    QTimer *timer;
};

#endif // ICMPSOCKET_H
