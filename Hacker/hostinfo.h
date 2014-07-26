#ifndef GLOBALINFO_H
#define GLOBALINFO_H

#include <QObject>
#include "global_define.h"

class HostInfo{
public:
    static quint8 self_mac[MAC_ADDR_LEN];
    static quint8 self_ip[IP_ADDR_LEN];
    static quint8 gateway_mac[MAC_ADDR_LEN];

    HostInfo();
    void hostInfoInit();

private:
    void getSelfMAC();
    void getSelfIP();
};

#endif // GLOBALINFO_H
