#include "gethostnamethread.h"

#include "ipresolver.h"

GetHostnameThread::GetHostnameThread(IPTools *ipTools, const QString& ip)
    : ipTools(ipTools), ip(ip)
{
}

void GetHostnameThread::run()
{
    QString hostname = ipTools->getHostname(ip);
    QString ip_info = ip.append(" : " ).append(hostname);
    emit hostnameReturned(ip_info);
}
