#include "ipresolver.h"

#include <qdebug.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

IPTools::IPTools()
{
}

QString IPTools::getHostname(const QString &ip)
{
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    struct addrinfo hints;
    struct addrinfo *result, *result_pointer;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME | AI_NUMERICHOST;
    hints.ai_protocol = 0;

    //Get ip related info
    ret = getaddrinfo(ip.toLatin1().data(), NULL, &hints, &result);
    if(ret != 0){
        qDebug() << "Error in getaddrinfo" << gai_strerror(ret);
        return "Unknown";
    }

    //Query hostname info
    for(result_pointer = result; result_pointer != NULL; result_pointer = result_pointer->ai_next){
        if(getnameinfo(result_pointer->ai_addr, result_pointer->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NAMEREQD) == 0){
            qDebug() << "Query result: host:" << hbuf << " service: " << sbuf;
            return QString(hbuf);
        }else{
            return QString("Unknown");
        }
    }
}

quint16 IPTools::ipChecksum(quint16 *buffer, int size)
{
    unsigned long cksum = 0;
    while(size>1)
    {
        cksum += *buffer++;
        size -= 2;
    }
    if(size)
    {
        cksum += *(quint8*)buffer;
    }
    cksum = (cksum>>16) + (cksum&0xffff);   // 将高 16bit 与低 16bit 相加
    cksum += (cksum>>16);              // 将进位到高位的 16bit 与低 16bit 再相加

    return (quint16)(~cksum);
}
