#ifndef IPRESOLVER_H
#define IPRESOLVER_H

#include <QString>

class IPTools
{
public:
    IPTools();
    QString getHostname(const QString& ip);
    static  quint16 ipChecksum(quint16* buffer, int size);
};

#endif // IPRESOLVER_H
