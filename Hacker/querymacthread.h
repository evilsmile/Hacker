#ifndef QUERYMACTHREAD_H
#define QUERYMACTHREAD_H

#include <QThread>

class ARPSender;

class QueryMacThread : public QThread
{
    Q_OBJECT
public:
    explicit QueryMacThread(ARPSender *arpSender, QObject *parent = 0);
    void setDestIP(const QString& dest_ip);

protected:
    void run();
private:
    ARPSender *arpSender;
    QString dest_ip;
};

#endif // QUERYMACTHREAD_H
