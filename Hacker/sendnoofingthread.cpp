#include "sendnoofingthread.h"
#include <QDebug>

//----------------------------------SendARPSnoofingThread-------------------
SendArpSnoofingThread::SendArpSnoofingThread(ARPSender *arpSender)
    : arpSender(arpSender)
{
}

void SendArpSnoofingThread::run()
{
    if(arpSender != NULL){
        arpSender->startSnoofingAsMonitor(dest_ip, GATEWAY_IP);
    }else{
        qDebug() << "Need ArpSender instance.";
    }
}

void SendArpSnoofingThread::setDestIP(const QString &dest_ip)
{
    this->dest_ip = dest_ip;
}

void SendArpSnoofingThread::stopSendFakedARP()
{
    arpSender->stopSnoofing();
}
