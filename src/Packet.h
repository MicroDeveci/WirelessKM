#pragma once

#include <QByteArray>
#include <QString>

struct Packet
{
    QByteArray bytes;
    QString label;
    int priority = 0;
};
