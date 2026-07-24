#include "KeyboardProtocol.h"
#include <QHash>
#include <QSet>

namespace {

// Create a printable key definition with normal and optional shifted characters
// 创建可打印按键定义，包含常规字符和可选的 Shift 字符
KeyDefinition printable(const char *id, quint16 usage, QChar normal, QChar shifted = QChar())
{
    return {QString::fromLatin1(id), usage, KeyDefinition::Kind::Printable, normal, shifted};
}

// Create a non-printable key definition (e.g. modifier, function key)
// 创建不可打印的按键定义（如修饰键、功能键）
KeyDefinition nonPrintable(const char *id, quint16 usage)
{
    return {QString::fromLatin1(id), usage, KeyDefinition::Kind::NonPrintable, QChar(), QChar()};
}

// Build the complete list of all supported keyboard key definitions
// 构建所有支持的键盘按键定义的完整列表
QList<KeyDefinition> buildKeyDefinitions()
{
    return {
        nonPrintable("Esc", 0x29),
        nonPrintable("F1", 0x3A),
        nonPrintable("F2", 0x3B),
        nonPrintable("F3", 0x3C),
        nonPrintable("F4", 0x3D),
        nonPrintable("F5", 0x3E),
        nonPrintable("F6", 0x3F),
        nonPrintable("F7", 0x40),
        nonPrintable("F8", 0x41),
        nonPrintable("F9", 0x42),
        nonPrintable("F10", 0x43),
        nonPrintable("F11", 0x44),
        nonPrintable("F12", 0x45),
        nonPrintable("PrintScreen", 0x46),
        nonPrintable("ScrollLock", 0x47),
        nonPrintable("Pause", 0x48),

        printable("Backquote", 0x35, '`', '~'),
        printable("Digit1", 0x1E, '1', '!'),
        printable("Digit2", 0x1F, '2', '@'),
        printable("Digit3", 0x20, '3', '#'),
        printable("Digit4", 0x21, '4', '$'),
        printable("Digit5", 0x22, '5', '%'),
        printable("Digit6", 0x23, '6', '^'),
        printable("Digit7", 0x24, '7', '&'),
        printable("Digit8", 0x25, '8', '*'),
        printable("Digit9", 0x26, '9', '('),
        printable("Digit0", 0x27, '0', ')'),
        printable("Minus", 0x2D, '-', '_'),
        printable("Equal", 0x2E, '=', '+'),
        nonPrintable("Backspace", 0x2A),

        nonPrintable("Tab", 0x2B),
        printable("KeyQ", 0x14, 'q', 'Q'),
        printable("KeyW", 0x1A, 'w', 'W'),
        printable("KeyE", 0x08, 'e', 'E'),
        printable("KeyR", 0x15, 'r', 'R'),
        printable("KeyT", 0x17, 't', 'T'),
        printable("KeyY", 0x1C, 'y', 'Y'),
        printable("KeyU", 0x18, 'u', 'U'),
        printable("KeyI", 0x0C, 'i', 'I'),
        printable("KeyO", 0x12, 'o', 'O'),
        printable("KeyP", 0x13, 'p', 'P'),
        printable("BracketLeft", 0x2F, '[', '{'),
        printable("BracketRight", 0x30, ']', '}'),
        printable("Backslash", 0x31, '\\', '|'),

        nonPrintable("CapsLock", 0x39),
        printable("KeyA", 0x04, 'a', 'A'),
        printable("KeyS", 0x16, 's', 'S'),
        printable("KeyD", 0x07, 'd', 'D'),
        printable("KeyF", 0x09, 'f', 'F'),
        printable("KeyG", 0x0A, 'g', 'G'),
        printable("KeyH", 0x0B, 'h', 'H'),
        printable("KeyJ", 0x0D, 'j', 'J'),
        printable("KeyK", 0x0E, 'k', 'K'),
        printable("KeyL", 0x0F, 'l', 'L'),
        printable("Semicolon", 0x33, ';', ':'),
        printable("Quote", 0x34, '\'', '"'),
        nonPrintable("Enter", 0x28),

        nonPrintable("ShiftLeft", 0xE1),
        printable("KeyZ", 0x1D, 'z', 'Z'),
        printable("KeyX", 0x1B, 'x', 'X'),
        printable("KeyC", 0x06, 'c', 'C'),
        printable("KeyV", 0x19, 'v', 'V'),
        printable("KeyB", 0x05, 'b', 'B'),
        printable("KeyN", 0x11, 'n', 'N'),
        printable("KeyM", 0x10, 'm', 'M'),
        printable("Comma", 0x36, ',', '<'),
        printable("Period", 0x37, '.', '>'),
        printable("Slash", 0x38, '/', '?'),
        nonPrintable("ShiftRight", 0xE5),

        nonPrintable("ControlLeft", 0xE0),
        nonPrintable("MetaLeft", 0xE3),
        nonPrintable("AltLeft", 0xE2),
        printable("Space", 0x2C, ' '),
        nonPrintable("AltRight", 0xE6),
        nonPrintable("MetaRight", 0xE7),
        nonPrintable("ContextMenu", 0x65),
        nonPrintable("ControlRight", 0xE4),

        nonPrintable("Insert", 0x49),
        nonPrintable("Home", 0x4A),
        nonPrintable("PageUp", 0x4B),
        nonPrintable("Delete", 0x4C),
        nonPrintable("End", 0x4D),
        nonPrintable("PageDown", 0x4E),
        nonPrintable("ArrowUp", 0x52),
        nonPrintable("ArrowLeft", 0x50),
        nonPrintable("ArrowDown", 0x51),
        nonPrintable("ArrowRight", 0x4F),

        nonPrintable("NumLock", 0x53),
        printable("NumpadDivide", 0x54, '/'),
        printable("NumpadMultiply", 0x55, '*'),
        printable("NumpadSubtract", 0x56, '-'),
        printable("Numpad7", 0x5F, '7'),
        printable("Numpad8", 0x60, '8'),
        printable("Numpad9", 0x61, '9'),
        printable("NumpadAdd", 0x57, '+'),
        printable("Numpad4", 0x5C, '4'),
        printable("Numpad5", 0x5D, '5'),
        printable("Numpad6", 0x5E, '6'),
        printable("Numpad1", 0x59, '1'),
        printable("Numpad2", 0x5A, '2'),
        printable("Numpad3", 0x5B, '3'),
        nonPrintable("NumpadEnter", 0x58),
        printable("Numpad0", 0x62, '0'),
        printable("NumpadDecimal", 0x63, '.'),

        nonPrintable("IntlBackslash", 0x64),
        nonPrintable("IntlYen", 0x89),
        nonPrintable("IntlRo", 0x87),
        nonPrintable("KanaMode", 0x88)
    };
}

// Build a lookup table mapping printable characters to their key definitions
// 构建可打印字符到按键定义的查找表
QHash<QChar, KeyDefinition> buildPrintableLookup()
{
    QHash<QChar, KeyDefinition> lookup;
    const QList<KeyDefinition> defs = buildKeyDefinitions();
    for (const KeyDefinition &def : defs) {
        if (def.kind != KeyDefinition::Kind::Printable)
            continue;
        if (!def.normal.isNull() && !lookup.contains(def.normal))
            lookup.insert(def.normal, def);
        if (!def.shifted.isNull() && !lookup.contains(def.shifted))
            lookup.insert(def.shifted, def);
    }
    return lookup;
}

// Build a map from alternate key names to canonical key IDs
// 构建从备用按键名称到规范按键 ID 的映射
QHash<QString, QString> buildCapturedKeyAliases()
{
    QHash<QString, QString> aliases;
    aliases.insert("ESC", "Esc");
    aliases.insert("ESCAPE", "Esc");
    aliases.insert("RETURN", "Enter");
    aliases.insert("ENTER", "Enter");
    aliases.insert("BACKSPACE", "Backspace");
    aliases.insert("TAB", "Tab");
    aliases.insert("SPACE", "Space");
    aliases.insert("DEL", "Delete");
    aliases.insert("DELETE", "Delete");
    aliases.insert("INS", "Insert");
    aliases.insert("INSERT", "Insert");
    aliases.insert("HOME", "Home");
    aliases.insert("END", "End");
    aliases.insert("PGUP", "PageUp");
    aliases.insert("PAGE UP", "PageUp");
    aliases.insert("PAGEUP", "PageUp");
    aliases.insert("PGDOWN", "PageDown");
    aliases.insert("PAGE DOWN", "PageDown");
    aliases.insert("PAGEDOWN", "PageDown");
    aliases.insert("LEFT", "ArrowLeft");
    aliases.insert("RIGHT", "ArrowRight");
    aliases.insert("UP", "ArrowUp");
    aliases.insert("DOWN", "ArrowDown");
    aliases.insert("SHIFT", "ShiftLeft");
    aliases.insert("CTRL", "ControlLeft");
    aliases.insert("CONTROL", "ControlLeft");
    aliases.insert("ALT", "AltLeft");
    aliases.insert("META", "MetaLeft");
    aliases.insert("WIN", "MetaLeft");
    aliases.insert("BACKQUOTE", "Backquote");
    aliases.insert("`", "Backquote");
    aliases.insert("~", "Backquote");
    aliases.insert("-", "Minus");
    aliases.insert("_", "Minus");
    aliases.insert("=", "Equal");
    aliases.insert("+", "Equal");
    aliases.insert("[", "BracketLeft");
    aliases.insert("{", "BracketLeft");
    aliases.insert("]", "BracketRight");
    aliases.insert("}", "BracketRight");
    aliases.insert("\\", "Backslash");
    aliases.insert("|", "Backslash");
    aliases.insert(";", "Semicolon");
    aliases.insert(":", "Semicolon");
    aliases.insert("'", "Quote");
    aliases.insert("\"", "Quote");
    aliases.insert(",", "Comma");
    aliases.insert("<", "Comma");
    aliases.insert(".", "Period");
    aliases.insert(">", "Period");
    aliases.insert("/", "Slash");
    aliases.insert("?", "Slash");

    for (int i = 1; i <= 12; ++i)
        aliases.insert(QString("F%1").arg(i), QString("F%1").arg(i));
    for (int i = 0; i <= 9; ++i)
        aliases.insert(QString::number(i), QString("Digit%1").arg(i));
    for (ushort code = 'A'; code <= 'Z'; ++code) {
        const QChar ch(code);
        aliases.insert(QString(ch), "Key" + QString(ch));
    }

    return aliases;
}

// Return a human-readable name for a printable character (escape special chars)
// 返回可打印字符的人类可读名称（转义特殊字符）
QString printableName(QChar ch)
{
    if (ch == '\n')
        return "\\n";
    if (ch == '\r')
        return "\\r";
    if (ch == '\t')
        return "\\t";
    if (ch == ' ')
        return "space";
    return QString(ch);
}

}

// Encode a text typing command as a UTF-8 byte array
// 将文本输入命令编码为 UTF-8 字节数组
QByteArray KeyboardProtocol::encodeTextCommand(const QString &text)
{
    return QString("type %1\n").arg(text).toUtf8();
}

// Encode a text typing command into a prioritised Packet for transmission
// 将文本输入命令编码为带优先级的 Packet 用于传输
Packet KeyboardProtocol::encodeTextPacket(const QString &text, const QString &transportLabel)
{
    const QByteArray utf8 = text.toUtf8();
    Packet packet;
    packet.bytes = encodeTextCommand(text);
    packet.label = describeTextCommand(transportLabel, utf8.size());
    packet.priority = 0;
    return packet;
}

// Encode a raw USB HID keycode press/release into a 3-byte payload wrapped in a frame
// 将原始 USB HID 键码按下/释放编码为3字节负载并包装为帧
QByteArray KeyboardProtocol::encodeRawKeycode(quint16 scanCode, bool pressed)
{
    QByteArray payload(3, '\0');
    payload[0] = static_cast<char>(scanCode & 0xFF);
    payload[1] = static_cast<char>((scanCode >> 8) & 0xFF);
    payload[2] = pressed ? '\x01' : '\x00';
    return buildFrame(0x02, payload);
}

// Build an AA-prefixed binary frame with command byte and LE16 payload length
// 构建以 AA 为前缀的二进制帧，包含命令字节和 LE16 负载长度
QByteArray KeyboardProtocol::buildFrame(quint8 cmd, const QByteArray &payload)
{
    QByteArray frame;
    frame.reserve(4 + payload.size());
    frame.append('\xAA');
    frame.append(static_cast<char>(cmd));
    const quint16 len = static_cast<quint16>(payload.size());
    frame.append(static_cast<char>(len & 0xFF));
    frame.append(static_cast<char>((len >> 8) & 0xFF));
    frame.append(payload);
    return frame;
}

// Validate that every character in text has a known keyboard mapping
// 验证文本中每个字符是否都有已知的键盘映射
TextMappingResult KeyboardProtocol::validateTextMapping(const QString &text)
{
    const QHash<QChar, KeyDefinition> lookup = buildPrintableLookup();
    QSet<QChar> missing;
    TextMappingResult result;
    result.totalChars = text.size();

    for (QChar ch : text) {
        if (lookup.contains(ch)) {
            ++result.mappedChars;
        } else {
            missing.insert(ch);
        }
    }

    for (QChar ch : missing)
        result.unmappedCharacters.append(printableName(ch));
    result.unmappedCharacters.sort();
    return result;
}

// Resolve a captured key input to its USB HID usage code
// 将捕获的键盘输入解析为对应的 USB HID 用法代码
bool KeyboardProtocol::keyUsageForCapturedInput(const QString &key, const QString &text, quint16 *usage)
{
    if (!usage)
        return false;

    const QList<KeyDefinition> defs = buildKeyDefinitions();
    if (text.size() == 1) {
        const QChar ch = text.at(0);
        for (const KeyDefinition &def : defs) {
            if (def.kind == KeyDefinition::Kind::Printable
                && (def.normal == ch || def.shifted == ch)) {
                *usage = def.usage;
                return true;
            }
        }
    }

    const QHash<QString, QString> aliases = buildCapturedKeyAliases();
    const QString aliasKey = key.trimmed().toUpper();
    const QString id = aliases.value(aliasKey);
    if (id.isEmpty())
        return false;

    for (const KeyDefinition &def : defs) {
        if (def.id == id) {
            *usage = def.usage;
            return true;
        }
    }

    return false;
}

// Return the full list of all supported keyboard key definitions
// 返回所有支持的键盘按键定义的完整列表
QList<KeyDefinition> KeyboardProtocol::keyDefinitions()
{
    return buildKeyDefinitions();
}

// Return keys that have both normal and shifted printable characters
// 返回同时具有常规和 Shift 可打印字符的按键
QList<KeyDefinition> KeyboardProtocol::dualCharacterKeys()
{
    QList<KeyDefinition> keys;
    const QList<KeyDefinition> defs = buildKeyDefinitions();
    for (const KeyDefinition &def : defs) {
        if (def.kind == KeyDefinition::Kind::Printable && !def.shifted.isNull())
            keys.append(def);
    }
    return keys;
}

// Build a human-readable description label for a text typing command
// 构建文本输入命令的人类可读描述标签
QString KeyboardProtocol::describeTextCommand(const QString &transport, qsizetype textBytes)
{
    return QString("%1_TYPE (%2 bytes)").arg(transport, QString::number(textBytes));
}

// Build a human-readable description of a raw keycode press/release event
// 构建原始键码按下/释放事件的人类可读描述
QString KeyboardProtocol::describeRawKeycode(quint16 scanCode, bool pressed)
{
    return QString("RAW_KEYCODE scan=0x%1 %2")
        .arg(scanCode, 4, 16, QChar('0'))
        .arg(pressed ? "DOWN" : "UP");
}

// Convert a firmware response byte to a short status label (OK, NACK, BUSY, or hex)
// 将固件响应字节转换为简短状态标签（OK、NACK、BUSY 或十六进制）
QString KeyboardProtocol::describeResponse(quint8 response)
{
    return (response == 0x00) ? "OK"
        : (response == 0xFE) ? "NACK"
        : (response == 0xFF) ? "BUSY"
        : QString("0x%1").arg(response, 2, 16, QChar('0'));
}

// Format a byte array as an uppercase space-separated hex dump string
// 将字节数组格式化为大写空格分隔的十六进制转储字符串
QString KeyboardProtocol::hexDump(const QByteArray &data)
{
    QString s;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0)
            s += ' ';
        s += QString("%1").arg(static_cast<quint8>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    return s;
}
