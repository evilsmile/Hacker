#ifndef GETONLINEIPSTHREAD_H
#define GETONLINEIPSTHREAD_H

#include <QThread>

class IcmpSocket;
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
#endif // GETONLINEIPSTHREAD_H
