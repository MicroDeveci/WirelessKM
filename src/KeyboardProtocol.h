#pragma once

#include <QByteArray>
#include <QChar>
#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include "Packet.h"

struct KeyDefinition
{
    enum class Kind {
        Printable,
        NonPrintable
    };

    QString id;
    quint16 usage = 0;
    Kind kind = Kind::NonPrintable;
    QChar normal;
    QChar shifted;
};

struct TextMappingResult
{
    int totalChars = 0;
    int mappedChars = 0;
    QStringList unmappedCharacters;

    bool ok() const { return unmappedCharacters.isEmpty(); }
};

class KeyboardProtocol
{
public:
    // Encode a text typing command as a UTF-8 string
    // 将文本输入命令编码为 UTF-8 字符串
    static QByteArray encodeTextCommand(const QString &text);
    // Encode a text typing command into a prioritised Packet
    // 将文本输入命令编码为带优先级的 Packet
    static Packet encodeTextPacket(const QString &text, const QString &transportLabel = "AUTO");
    // Encode a raw USB HID keycode press/release into a binary frame
    // 将原始 USB HID 键码按下/释放编码为二进制帧
    static QByteArray encodeRawKeycode(quint16 scanCode, bool pressed);
    // Build an AA-prefixed binary frame with command byte and LE16 length
    // 构建以 AA 为前缀的二进制帧，包含命令字节和 LE16 长度
    static QByteArray buildFrame(quint8 cmd, const QByteArray &payload);

    // Validate that every character in text maps to a known key definition
    // 验证文本中每个字符是否都能映射到已知的按键定义
    static TextMappingResult validateTextMapping(const QString &text);
    // Look up the USB HID usage code for a captured keyboard input
    // 查找捕获的键盘输入对应的 USB HID 用法代码
    static bool keyUsageForCapturedInput(const QString &key, const QString &text, quint16 *usage);
    // Return the full list of key definitions (printable and non-printable)
    // 返回完整的按键定义列表（可打印和不可打印）
    static QList<KeyDefinition> keyDefinitions();
    // Return only key definitions that have both normal and shifted characters
    // 仅返回同时具有常规和 Shift 字符的按键定义
    static QList<KeyDefinition> dualCharacterKeys();

    // Build a human-readable description of a text typing command
    // 构建文本输入命令的人类可读描述
    static QString describeTextCommand(const QString &transport, qsizetype textBytes);
    // Build a human-readable description of a raw keycode event
    // 构建原始键码事件的人类可读描述
    static QString describeRawKeycode(quint16 scanCode, bool pressed);
    // Convert a firmware response byte into a short status string
    // 将固件响应字节转换为简短的状态字符串
    static QString describeResponse(quint8 response);
    // Format a byte array as an uppercase hex dump with spaces
    // 将字节数组格式化为大写十六进制转储，以空格分隔
    static QString hexDump(const QByteArray &data);
};
