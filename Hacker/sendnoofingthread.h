#ifndef SENDNOOFINGTHREAD_H
#define SENDNOOFINGTHREAD_H

#include <QThread>

#include "arpsender.h"
#include "hostinfo.h"

class SendArpSnoofingThread : public QThread{
    Q_OBJECT
public:
    explicit SendArpSnoofingThread(ARPSender *arpSender);
    void setDestIP(const QString& dest_ip);
private slots:
    void stopSendFakedARP();

protected:
    void run();

private:
    ARPSender *arpSender;
    QString dest_ip;
};

#endif // SENDNOOFINGTHREAD_H
