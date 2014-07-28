#include "getonlineipsthread.h"
#include "icmpsocket.h"

//----------------------------------GetOnlineIPsThread------------------------------------
GetOnlineIPsThread::GetOnlineIPsThread(QObject *parent)
    : QThread(parent), icmpSocket(new IcmpSocket())
{
}

GetOnlineIPsThread::~GetOnlineIPsThread()
{
    delete icmpSocket;
}

void GetOnlineIPsThread::setIpRange(int startIP, int endIP)
{
    this->startIP = startIP;
    this->endIP = endIP;
}

//The function of getting online ip list is implemented here.
//The IcmpSocket is used to send ICMP packets to corresponding ip for reachable test.
//And if one online ip is found it'll emit foundOneOnlineIP() signal to update UI.
//And when all ips are scaned it'll emit scanHasFinished() signal.
void GetOnlineIPsThread::run()
{
    int pace = 100 / (endIP - startIP + 1);
    int progress = 0;
    for(int i = startIP; i <= endIP; i++){
        QString testIP= "192.168.1.";
        testIP.append((QString::number(i)));
        if(icmpSocket->ping(testIP)){
            emit foundOneOnlineIP(testIP);
        }
        //update progress information
        progress += pace;
        emit scanProgressForward(progress);
    }
    //In case that (endIP - startIP + 1) won't be able to be divided by 100.
    if(progress < 100)
        emit scanProgressForward(100);

    emit scanHasFinished();
}
