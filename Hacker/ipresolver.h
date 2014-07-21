#ifndef IPRESOLVER_H
#define IPRESOLVER_H

#include <QString>

class IPResolver
{
public:
    IPResolver();
    QString getHostname(const QString& ip);
};

#endif // IPRESOLVER_H
