#ifndef HACKER_H
#define HACKER_H

#include "icmpsocket.h"
#include "ipresolver.h"
#include <QThread>

class MainWindow;
class GetOnlineIPsThread;
class ARPSender;
class SendArpSnoofingThread;
class GetPacketsThread;
class QueryMacThread;
class GetHostnameThread;
class HostInfo;

class Hacker : public QObject{
    Q_OBJECT
public:
    explicit Hacker(QObject *parent = NULL);
    ~Hacker();

    void getOnlineIPs(int startIP, int endIP);
    void getHostname(const QString& ip);
    void queryMacAddress(const QString& dest_ip);
    void makeHostRedirectToMe(const QString &dest_ip, bool block_host_surf);

private slots:
    void startPackerForwarding(const QString& mac_addr);

private:
    MainWindow *main_ui;

    HostInfo *hostInfo;
    IPTools *ipResolver;
    ARPSender *arpSender;

    GetOnlineIPsThread *getOnlineIPsThread;
    SendArpSnoofingThread *sendArpSnoofingThread;
    GetPacketsThread *getPacketsThread;
    QueryMacThread *queryMacThread;
    GetHostnameThread *getHostnameThread;
};

#endif // HACKER_H
