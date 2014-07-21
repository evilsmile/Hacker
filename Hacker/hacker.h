#ifndef HACKER_H
#define HACKER_H

#include "icmpsocket.h"
#include "ipresolver.h"
#include <QThread>

class MainWindow;
class GetOnlineIPsThread;
class ARPSender;

class Hacker : public QObject{
    Q_OBJECT
public:
    explicit Hacker(QObject *parent = NULL);
    ~Hacker();

    void getOnlineIPs(int startIP, int endIP);
    QString getHostname(const QString& ip);
    void queryARP();

private:
    MainWindow *main_ui;

    GetOnlineIPsThread *getOnlineIPsThread;
    IPResolver *ipResolver;
    ARPSender *arpSender;
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



#endif // HACKER_H
