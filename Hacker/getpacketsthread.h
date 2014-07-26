#ifndef GETPACKETTHREAD_H
#define GETPACKETTHREAD_H

#include <QThread>
#include "global_define.h"

class GetPacketsThread : public QThread{
    Q_OBJECT
public:
    explicit GetPacketsThread(){}
    void setVictimMacAddr(const QString& mac_addr);
    void setVictimIpAddr(const QString& ip_addr);
    \
protected:
    void run();

private:
    quint8 victim_mac_addr[MAC_ADDR_LEN];
    QString victim_ip_addr;

    void getPackets();
};

#endif // GETPACKETTHREAD_H
