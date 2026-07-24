#include "HudpProtocol.h"

namespace {
// Append a big-endian 16-bit value to the byte array
// 将大端序16位值追加到字节数组
void put16(QByteArray &out, quint16 value) { out.append(char(value >> 8)); out.append(char(value)); }
// Append a big-endian 32-bit value to the byte array
// 将大端序32位值追加到字节数组
void put32(QByteArray &out, quint32 value) { out.append(char(value >> 24)); out.append(char(value >> 16)); out.append(char(value >> 8)); out.append(char(value)); }
}

// Compute a standard CRC32 checksum over the given bytes
// 对给定字节计算标准 CRC32 校验和
quint32 HudpProtocol::crc32(const QByteArray &bytes)
{
    quint32 crc = 0xffffffffu;
    for (uchar byte : bytes) {
        crc ^= byte;
        for (int bit = 0; bit != 8; ++bit)
            crc = (crc >> 1) ^ ((crc & 1u) ? 0xedb88320u : 0u);
    }
    return crc ^ 0xffffffffu;
}

// Build a complete HUDP packet with header, payload, and trailing CRC32
// 构建完整的 HUDP 数据包，包含头部、负载和尾部 CRC32
QByteArray HudpProtocol::build(quint8 type, quint16 flags, quint32 sessionId, quint32 sequence, quint32 timestampMs, const QByteArray &payload)
{
    if (payload.size() > MaxPayload)
        return {};
    QByteArray packet("HUDP", 4);
    packet.append(char(Version)); packet.append(char(type));
    put16(packet, flags); put32(packet, sessionId); put32(packet, sequence); put32(packet, timestampMs); put16(packet, quint16(payload.size()));
    packet.append(payload);
    put32(packet, crc32(packet));
    return packet;
}
