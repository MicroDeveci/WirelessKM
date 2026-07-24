#pragma once

#include <QObject>
#include <QSerialPort>
#include <QTimer>

class SerialManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentPort READ currentPort NOTIFY currentPortChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY availablePortsChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString monitorText READ monitorText NOTIFY monitorTextChanged)
    Q_PROPERTY(bool monitorRunning READ monitorRunning NOTIFY monitorRunningChanged)

public:
    // Constructor: initialise and enumerate available serial ports
    // 构造函数: 初始化并枚举可用串口
    explicit SerialManager(QObject *parent = nullptr);
    // Destructor: close and release the serial port
    // 析构函数: 关闭并释放串口
    ~SerialManager() override;

    // Get the currently connected port name
    // 获取当前连接的端口名称
    QString currentPort() const { return m_currentPort; }
    // Check if the serial port is open and connected
    // 检查串口是否已打开并连接
    bool connected() const { return m_serial && m_serial->isOpen(); }
    // Get the list of available serial port names
    // 获取可用串口名称列表
    QStringList availablePorts() const { return m_availablePorts; }
    // Get the current status text
    // 获取当前状态文本
    QString statusText() const { return m_statusText; }
    // Get the serial monitor output text
    // 获取串口监视器输出文本
    QString monitorText() const { return m_monitorText; }
    // Check if the serial monitor is running
    // 检查串口监视器是否正在运行
    bool monitorRunning() const { return m_monitorRunning; }

    // Re-enumerate available serial ports
    // 重新枚举可用串口
    Q_INVOKABLE void refreshPorts();
    // Start the serial monitor output
    // 启动串口监视器输出
    Q_INVOKABLE void startMonitor();
    // Stop the serial monitor output
    // 停止串口监视器输出
    Q_INVOKABLE void stopMonitor();
    // Clear the serial monitor output buffer
    // 清除串口监视器输出缓冲区
    Q_INVOKABLE void clearMonitor();

public slots:
    // Open and connect to a serial port at the given baud rate
    // 打开并以指定波特率连接到串口
    void connectToPort(const QString &port, int baudrate = 115200);
    // Close the current serial port connection
    // 关闭当前串口连接
    void disconnectPort();
    // Write raw bytes to the serial port
    // 向串口写入原始字节数据
    void writeRaw(const QByteArray &data);
    // Switch to a different serial port (disconnect current first)
    // 切换到不同的串口（先断开当前连接）
    void changePort(const QString &newPort, int baudrate = 115200);

signals:
    void currentPortChanged();
    void connectedChanged();
    void availablePortsChanged();
    void statusTextChanged();
    void monitorTextChanged();
    void monitorRunningChanged();
    void errorOccurred(const QString &error);
    void rawDataReceived(const QString &data);
    void rawBytesReceived(const QByteArray &data);
    // Complete HID Bridge v3 frames received from the serial stream.
    void frameReceived(quint8 command, const QByteArray &frame);

private slots:
    // Handle incoming serial data: parse frames and emit signals
    // 处理串口接收数据: 解析帧并发出信号
    void onReadyRead();
    // Handle a serial port error
    // 处理串口错误
    void onSerialError(QSerialPort::SerialPortError error);

private:
    // Set the status text and emit signal if changed
    // 设置状态文本并在变化时发出信号
    void setStatus(const QString &status);
    // Append received data to the serial monitor buffer
    // 将接收数据追加到串口监视器缓冲区
    void appendMonitorData(const QByteArray &data);
    // Parse incoming data for complete HID Bridge v3 protocol frames
    // 解析传入数据以提取完整的 HID Bridge v3 协议帧
    void consumeProtocolFrames(const QByteArray &data);

    QSerialPort *m_serial = nullptr;
    QString m_currentPort;
    QStringList m_availablePorts;
    QString m_statusText;
    QString m_readBuffer;
    QByteArray m_protocolRxBuffer;
    QString m_monitorText;
    bool m_monitorRunning = false;
};
