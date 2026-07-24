#pragma once

#include <QByteArray>
#include <QString>
#include <QtGlobal>

class MouseProtocol
{
public:
    enum Button : quint8 {
        NoButton = 0x00,
        LeftButton = 0x01,
        RightButton = 0x02,
        MiddleButton = 0x04,
        ButtonMask = LeftButton | RightButton | MiddleButton
    };

    static constexpr quint8 Command = 0x06;
    static constexpr int MinimumDelta = -127;
    static constexpr int MaximumDelta = 127;

    // Encode a mouse HID report (buttons, deltas, wheel) into an AA-prefixed binary frame
    // 将鼠标 HID 报告（按键、增量、滚轮）编码为 AA 前缀二进制帧
    static QByteArray encodeReport(quint8 buttons, qint8 dx, qint8 dy, qint8 wheel = 0);
    // Build a human-readable description of a mouse report
    // 构建鼠标报告的人类可读描述
    static QString describeReport(quint8 buttons, qint8 dx, qint8 dy, qint8 wheel = 0);
};
