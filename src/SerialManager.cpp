#include "SerialManager.h"
#include "DebugTrace.h"
#include <QDebug>
#include <QSerialPortInfo>

// Constructor: initialise the serial manager and enumerate available ports
// 构造函数: 初始化串口管理器并枚举可用端口
SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
{
    refreshPorts();
}

// Destructor: close and release the serial port
// 析构函数: 关闭并释放串口
SerialManager::~SerialManager()
{
    if (m_serial) {
        m_serial->close();
        delete m_serial;
    }
}

// Re-enumerate available serial ports and emit signal if the list changed
// 重新枚举可用串口，列表变化时发出信号
void SerialManager::refreshPorts()
{
    QStringList ports;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const auto &info : infos) {
        ports.append(info.portName());
    }
    if (ports != m_availablePorts) {
        m_availablePorts = ports;
        emit availablePortsChanged();
    }
    if (DebugTrace::enabled()) qDebug() << "[SerialManager] Available ports:" << m_availablePorts;
}

// Start the serial monitor output capture
// 启动串口监视器输出捕获
void SerialManager::startMonitor()
{
    if (m_monitorRunning)
        return;

    m_monitorRunning = true;
    emit monitorRunningChanged();
}

// Stop the serial monitor output capture
// 停止串口监视器输出捕获
void SerialManager::stopMonitor()
{
    if (!m_monitorRunning)
        return;

    m_monitorRunning = false;
    emit monitorRunningChanged();
}

// Clear the serial monitor output buffer
// 清除串口监视器输出缓冲区
void SerialManager::clearMonitor()
{
    if (m_monitorText.isEmpty())
        return;

    m_monitorText.clear();
    emit monitorTextChanged();
}

// Open and connect to a serial port with 8N1 configuration and no flow control
// 打开并以 8N1 配置、无流控连接到串口
void SerialManager::connectToPort(const QString &port, int baudrate)
{
    const bool wasConnected = connected();
    if (m_serial && m_serial->isOpen()) {
        m_serial->close();
    }

    if (!m_serial) {
        m_serial = new QSerialPort(this);
    } else {
        // Disconnect old signals before reconnecting to prevent double-signals
        QObject::disconnect(m_serial, nullptr, this, nullptr);
    }

    m_serial->setPortName(port);
    m_serial->setBaudRate(baudrate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadWrite)) {
        m_currentPort = port;
        setStatus(QString("Connected %1").arg(port));
        if (DebugTrace::enabled()) qDebug() << "[SerialManager] Connected" << port << "baudrate:" << baudrate;
        emit currentPortChanged();
        emit connectedChanged();

        connect(m_serial, &QSerialPort::readyRead, this, &SerialManager::onReadyRead);
        connect(m_serial, &QSerialPort::errorOccurred, this, &SerialManager::onSerialError);
    } else {
        QString err = m_serial->errorString();
        qWarning() << "[SerialManager] Open failed:" << port << err;
        const bool hadCurrentPort = !m_currentPort.isEmpty();
        m_currentPort.clear();
        setStatus(QString("%1: %2").arg(port, err));
        if (hadCurrentPort)
            emit currentPortChanged();
        if (wasConnected)
            emit connectedChanged();
        emit errorOccurred(err);
    }
}

// Close the current serial port and update connection state
// 关闭当前串口并更新连接状态
void SerialManager::disconnectPort()
{
    if (m_serial && m_serial->isOpen()) {
        m_serial->close();
    }
    m_currentPort.clear();
    setStatus("Disconnected");
    if (DebugTrace::enabled()) qDebug() << "[SerialManager] Disconnected";
    emit connectedChanged();
    emit currentPortChanged();
}

// Write raw bytes to the serial port and flush
// 向串口写入原始字节并刷新缓冲区
void SerialManager::writeRaw(const QByteArray &data)
{
    if (DebugTrace::enabled()) qDebug() << "[TRACE] SerialManager::writeRaw: len=" << data.size()
             << "serial=" << m_serial
             << "isOpen=" << (m_serial ? m_serial->isOpen() : false);
    if (!m_serial || !m_serial->isOpen()) {
        qWarning() << "[SerialManager] Not connected - cannot write";
        emit errorOccurred("Not connected");
        return;
    }

    qint64 written = m_serial->write(data);
    if (DebugTrace::enabled()) qDebug() << "[TRACE] SerialManager::writeRaw: bytesWritten=" << written;

    if (written == -1) {
        qWarning() << "[SerialManager] Write failed:" << m_serial->errorString();
        emit errorOccurred(m_serial->errorString());
        return;
    }

    m_serial->flush();
}

// Disconnect from the current port and reconnect to a different one
// 断开当前端口并重新连接到另一个端口
void SerialManager::changePort(const QString &newPort, int baudrate)
{
    if (DebugTrace::enabled()) qDebug() << "[SerialManager] Changing port to" << newPort << "baudrate:" << baudrate;
    if (connected()) {
        disconnectPort();
    }
    connectToPort(newPort, baudrate);
}

// Handle incoming serial data: parse protocol frames and emit raw/text signals
// 处理串口接收数据: 解析协议帧并发出原始/文本信号
void SerialManager::onReadyRead()
{
    QByteArray data = m_serial->readAll();
    if (data.isEmpty()) return;

    appendMonitorData(data);

    // emit raw bytes for binary frame parsing
    emit rawBytesReceived(data);
    consumeProtocolFrames(data);

    // also emit text lines for logging (best-effort)
    m_readBuffer += QString::fromUtf8(data);
    while (m_readBuffer.contains('\n')) {
        int idx = m_readBuffer.indexOf('\n');
        QString line = m_readBuffer.left(idx).trimmed();
        m_readBuffer = m_readBuffer.mid(idx + 1);
        if (!line.isEmpty()) {
            if (DebugTrace::enabled()) qDebug() << "[SerialManager] RX:" << line.left(120);
            emit rawDataReceived(line);
        }
    }
}

// Parse the serial byte stream for AA-prefixed HID Bridge v3 protocol frames
// 解析串口字节流以提取 AA 前缀的 HID Bridge v3 协议帧
void SerialManager::consumeProtocolFrames(const QByteArray &data)
{
    // HID Bridge v3: AA | command | payload length (LE16) | payload (0..16).
    // Serial monitor data can be mixed into this stream, so discard noise and
    // resynchronise after an invalid length instead of retaining it forever.
    constexpr int HeaderSize = 4;
    constexpr quint16 MaximumPayloadLength = 16;

    m_protocolRxBuffer += data;
    while (m_protocolRxBuffer.size() >= HeaderSize) {
        const int frameStart = m_protocolRxBuffer.indexOf(static_cast<char>(0xAA));
        if (frameStart < 0) {
            m_protocolRxBuffer.clear();
            return;
        }
        if (frameStart > 0)
            m_protocolRxBuffer.remove(0, frameStart);
        if (m_protocolRxBuffer.size() < HeaderSize)
            return;

        const quint16 length = static_cast<quint8>(m_protocolRxBuffer.at(2))
            | (static_cast<quint16>(static_cast<quint8>(m_protocolRxBuffer.at(3))) << 8);
        if (length > MaximumPayloadLength) {
            // Keep searching after this header byte; a following AA may start
            // a valid frame.
            m_protocolRxBuffer.remove(0, 1);
            continue;
        }
        const int frameSize = HeaderSize + length;
        if (m_protocolRxBuffer.size() < frameSize)
            return;

        const quint8 command = static_cast<quint8>(m_protocolRxBuffer.at(1));
        const QByteArray frame = m_protocolRxBuffer.left(frameSize);
        m_protocolRxBuffer.remove(0, frameSize);
        emit frameReceived(command, frame);
    }
}

// Append raw serial data to the monitor buffer, truncating if necessary
// 将串口原始数据追加到监视器缓冲区，必要时进行截断
void SerialManager::appendMonitorData(const QByteArray &data)
{
    if (!m_monitorRunning)
        return;

    // Keep device output unparsed so ESP-IDF boot, log and panic text remain
    // in arrival order. The cap only bounds GUI memory.
    m_monitorText += QString::fromUtf8(data);
    constexpr int kMaximumMonitorChars = 1'000'000;
    if (m_monitorText.size() > kMaximumMonitorChars) {
        constexpr int kKeepChars = 900'000;
        m_monitorText = QStringLiteral("[Earlier serial output truncated to keep the monitor responsive]\n")
            + m_monitorText.right(kKeepChars);
    }
    emit monitorTextChanged();
}

// Handle a serial port error: update status and auto-disconnect on device removal
// 处理串口错误: 更新状态，设备移除时自动断开连接
void SerialManager::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;

    QString msg;
    switch (error) {
    case QSerialPort::DeviceNotFoundError:
        msg = "Device not found";
        break;
    case QSerialPort::PermissionError:
        msg = "Permission denied";
        break;
    case QSerialPort::OpenError:
        msg = "Already open";
        break;
    case QSerialPort::TimeoutError:
        msg = "Timeout";
        break;
    case QSerialPort::NotOpenError:
        msg = "Not open";
        break;
    case QSerialPort::ResourceError:
        msg = "Device removed";
        disconnectPort();
        break;
    default:
        msg = m_serial ? m_serial->errorString() : "Unknown error";
        break;
    }

    qWarning() << "[SerialManager] Error:" << msg;
    setStatus(msg);
    emit errorOccurred(msg);
}

// Set the status text and emit signal if changed
// 设置状态文本并在变化时发出信号
void SerialManager::setStatus(const QString &status)
{
    if (m_statusText != status) {
        m_statusText = status;
        emit statusTextChanged();
    }
}
