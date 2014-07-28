#include "querymacthread.h"
#include "arpsender.h"

QueryMacThread::QueryMacThread(ARPSender *arpSender, QObject *parent) :
    QThread(parent), arpSender(arpSender)
{
}

void QueryMacThread::setDestIP(const QString &dest_ip)
{
    this->dest_ip = dest_ip;
}

void QueryMacThread::run()
{\
    arpSender->queryMacAddress(dest_ip);
}
