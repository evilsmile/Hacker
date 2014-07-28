#ifndef GETHOSTNAMETHREAD_H
#define GETHOSTNAMETHREAD_H

#include <QThread>

class IPTools;

class GetHostnameThread : public QThread
{
    Q_OBJECT
public:
    GetHostnameThread(IPTools *ipTools, const QString& ip);

signals:
    void hostnameReturned(const QString& hostname);

protected:
    void run();

private:
    IPTools *ipTools;
    QString ip;
};

#endif // GETHOSTNAMETHREAD_H
