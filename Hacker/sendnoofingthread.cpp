#include "sendnoofingthread.h"
#include <QDebug>

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
