#include "hacker.h"
#include "arpsender.h"
#include "mainwindow.h"
#include <QDebug>

Hacker::Hacker(QObject *parent)
    : main_ui((MainWindow*)parent),
      getOnlineIPsThread(NULL),
      arpSender(NULL)
{
}

Hacker::~Hacker()
{
    if(getOnlineIPsThread != NULL){
        delete getOnlineIPsThread;
    }
    if(arpSender != NULL){
        delete arpSender;
    }
}

//Called by UI thread.
//create a thread to get online ips.
void Hacker::getOnlineIPs(int startIP, int endIP)
{
    if(getOnlineIPsThread == NULL)
        getOnlineIPsThread = new GetOnlineIPsThread(this, startIP, endIP);

    //When one ip is found or scan is finished, tell the UI.
    connect(getOnlineIPsThread, SIGNAL(foundOneOnlineIP(QString)), main_ui, SLOT(oneOnlineIPFound(QString)));
    connect(getOnlineIPsThread, SIGNAL(scanHasFinished()), main_ui, SLOT(scanDone()));
    connect(getOnlineIPsThread, SIGNAL(scanProgressForward(int)), main_ui, SLOT(updateProgress(int)));

    getOnlineIPsThread->start();
}

QString Hacker::getHostname(const QString &ip)
{
    ipResolver = new IPResolver();
    QString hostname = ipResolver->getHostname(ip);
    return hostname;
}

void Hacker::queryARP()
{
    if(arpSender == NULL){
        arpSender = new ARPSender();
    }
    arpSender->queryARP();
}

//----------------------------------GetOnlineIPsThread------------------------------------
GetOnlineIPsThread::GetOnlineIPsThread(QObject *parent, int startIP, int endIP)
    : QThread(parent), icmpSocket(new IcmpSocket()),
      startIP(startIP), endIP(endIP)
{
}

GetOnlineIPsThread::~GetOnlineIPsThread()
{
    delete icmpSocket;
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
