#include "MouseProtocol.h"

#include "KeyboardProtocol.h"

// Encode a mouse HID report (buttons, deltas, wheel) into an AA-prefixed binary frame
// 将鼠标 HID 报告（按键、增量、滚轮）编码为 AA 前缀二进制帧
QByteArray MouseProtocol::encodeReport(quint8 buttons, qint8 dx, qint8 dy, qint8 wheel)
{
    QByteArray payload;
    payload.reserve(4);
    payload.append(static_cast<char>(buttons & ButtonMask));
    payload.append(static_cast<char>(dx));
    payload.append(static_cast<char>(dy));
    payload.append(static_cast<char>(wheel));
    return KeyboardProtocol::buildFrame(Command, payload);
}

// Build a human-readable description of a mouse report
// 构建鼠标报告的人类可读描述
QString MouseProtocol::describeReport(quint8 buttons, qint8 dx, qint8 dy, qint8 wheel)
{
    return QString("MOUSE buttons=0x%1 dx=%2 dy=%3 wheel=%4")
        .arg(buttons & ButtonMask, 2, 16, QChar('0'))
        .arg(static_cast<int>(dx))
        .arg(static_cast<int>(dy))
        .arg(static_cast<int>(wheel));
}
