#pragma once

#include <QByteArray>
#include <QtGlobal>

namespace HudpProtocol {
constexpr quint16 DefaultPort = 45820;
constexpr quint8 Version = 1;
constexpr quint16 HeaderSize = 22;
constexpr quint16 MaxPayload = 1024;
// Extension: accepts the existing AA binary stream inside a validated HUDP
// datagram, so firmware can feed it directly to frame_processor.
enum Type : quint8 { KeyboardEvent = 0x01, KeyboardState = 0x02, MouseEvent = 0x03, MouseState = 0x04, KeepAlive = 0x05, StateRequest = 0x06, StateReset = 0x07, Ack = 0x08, LegacyBinaryFrame = 0x20 };
enum Flag : quint16 { AckRequired = 1, Retransmission = 2, FullState = 4, Encrypted = 8, Compressed = 16, ResetState = 32 };
// Build a complete HUDP packet with header, payload, and CRC32
// 构建完整的 HUDP 数据包，包含头部、负载和 CRC32 校验
QByteArray build(quint8 type, quint16 flags, quint32 sessionId, quint32 sequence, quint32 timestampMs, const QByteArray &payload);
// Compute a standard CRC32 checksum over the given bytes
// 对给定字节计算标准 CRC32 校验和
quint32 crc32(const QByteArray &bytes);
}
