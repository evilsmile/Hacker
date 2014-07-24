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

class Hacker : public QObject{
    Q_OBJECT
public:
    explicit Hacker(QObject *parent = NULL);
    ~Hacker();

    void getOnlineIPs(int startIP, int endIP);
    QString getHostname(const QString& ip);
    void queryMacAddress(const QString& dest_ip);
    void makeHostRedirectToMe(const QString &dest_ip);

private:
    MainWindow *main_ui;

    IPResolver *ipResolver;
    ARPSender *arpSender;

    GetOnlineIPsThread *getOnlineIPsThread;
    SendArpSnoofingThread *sendArpSnoofingThread;
    GetPacketsThread *getPacketsThread;
};


class GetOnlineIPsThread : public QThread{
    Q_OBJECT

public:
    explicit GetOnlineIPsThread(QObject *parent, int startIP, int endIP);
    ~GetOnlineIPsThread();

signals:
    void foundOneOnlineIP(QString ip);
    void scanHasFinished();
    void scanProgressForward(int value);

protected:
    void run();

private:
    IcmpSocket *icmpSocket;
    int startIP;
    int endIP;
};

class SendArpSnoofingThread : public QThread{
    Q_OBJECT
public:
    explicit SendArpSnoofingThread(ARPSender *arpSender, const QString& dest_ip);

private slots:
    void stopSendFakedARP();

protected:
    void run();

private:
    ARPSender *arpSender;
    QString dest_ip;
};

class GetPacketsThread : public QThread{
    Q_OBJECT
public:
    explicit GetPacketsThread(){};
    \
protected:
    void run();

private:
    void getPackets();
};

#endif // HACKER_H
