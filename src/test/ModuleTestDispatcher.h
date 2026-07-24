#pragma once

#include <QJsonObject>

class ModuleTestDispatcher
{
public:
    static QJsonObject dispatch(const QJsonObject &request);
    static QJsonObject describe();
};
