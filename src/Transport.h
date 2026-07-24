#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>

class Transport : public QObject
{
    Q_OBJECT

public:
    explicit Transport(QObject *parent = nullptr) : QObject(parent) {}
    ~Transport() override = default;

    // Pure virtual: get transport name identifier
    // 纯虚函数: 获取传输层名称标识符
    virtual QString name() const = 0;
    // Pure virtual: check if transport is connected
    // 纯虚函数: 检查传输层是否已连接
    virtual bool connected() const = 0;
    // Pure virtual: send a data frame over this transport
    // 纯虚函数: 通过此传输层发送数据帧
    virtual void write(const QByteArray &frame) = 0;

signals:
    void connectedChanged();
    void readyRead(const QByteArray &data);
    void errorOccurred(const QString &error);
};
