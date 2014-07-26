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
class HostInfo;

class Hacker : public QObject{
    Q_OBJECT
public:
    explicit Hacker(QObject *parent = NULL);
    ~Hacker();

    void getOnlineIPs(int startIP, int endIP);
    QString getHostname(const QString& ip);
    void queryMacAddress(const QString& dest_ip);
    void makeHostRedirectToMe(const QString &dest_ip);

private slots:
    void startPackerForwarding(const QString& mac_addr);

private:
    MainWindow *main_ui;

    IPTools *ipResolver;
    ARPSender *arpSender;
    HostInfo *hostInfo;

    GetOnlineIPsThread *getOnlineIPsThread;
    SendArpSnoofingThread *sendArpSnoofingThread;
    GetPacketsThread *getPacketsThread;
};

#endif // HACKER_H
