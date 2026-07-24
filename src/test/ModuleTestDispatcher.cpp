#include "ModuleTestDispatcher.h"

#include "AutoClickerManager.h"
#include "DebugTrace.h"
#include "InputQueue.h"
#include "KeyboardProtocol.h"
#include "KeySettingManager.h"
#include "HudpProtocol.h"
#include "LogManager.h"
#include "MouseInputManager.h"
#include "MouseProtocol.h"
#include "Packet.h"
#include "SettingsManager.h"
#include "Transport.h"
#include "TransportSelector.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QTimer>
#include <QVariantMap>

#include <utility>

namespace {

class FakeTransport final : public Transport
{
public:
    explicit FakeTransport(QString transportName, bool isConnected = false)
        : m_name(std::move(transportName)), m_connected(isConnected)
    {
    }

    QString name() const override { return m_name; }
    bool connected() const override { return m_connected; }
    void write(const QByteArray &frame) override { writtenFrames.append(frame); }

    void setConnected(bool connected)
    {
        if (m_connected == connected)
            return;
        m_connected = connected;
        emit connectedChanged();
    }

    QList<QByteArray> writtenFrames;

private:
    QString m_name;
    bool m_connected = false;
};

QJsonObject success(const QJsonObject &result)
{
    return {{"ok", true}, {"result", result}};
}

QJsonObject failure(const QString &message)
{
    return {{"ok", false}, {"error", message}};
}

QJsonObject withId(QJsonObject response, const QJsonObject &request)
{
    if (request.contains("id"))
        response.insert("id", request.value("id"));
    return response;
}

bool parseHex(const QString &text, QByteArray *bytes)
{
    QString normalized = text;
    normalized.remove(QRegularExpression("\\s+"));
    if ((normalized.size() % 2) != 0
        || normalized.contains(QRegularExpression("[^0-9A-Fa-f]"))) {
        return false;
    }
    *bytes = QByteArray::fromHex(normalized.toLatin1());
    return true;
}

QJsonArray numbers(const QList<quint16> &values)
{
    QJsonArray array;
    for (quint16 value : values)
        array.append(static_cast<int>(value));
    return array;
}

QJsonArray cleanLogs(const QVariantList &entries)
{
    QJsonArray output;
    for (const QVariant &entry : entries) {
        const QVariantMap map = entry.toMap();
        output.append(QJsonObject{{"level", map.value("level").toInt()},
                                  {"module", map.value("module").toString()},
                                  {"message", map.value("message").toString()},
                                  {"detail", map.value("detail").toString()}});
    }
    return output;
}

QJsonObject keyboard(const QString &action, const QJsonObject &input)
{
    if (action == "encode_text") {
        const QByteArray encoded = KeyboardProtocol::encodeTextCommand(input.value("text").toString());
        return success({{"hex", KeyboardProtocol::hexDump(encoded)},
                        {"utf8", QString::fromUtf8(encoded)}});
    }
    if (action == "encode_raw_key") {
        const int usage = input.value("usage").toInt(-1);
        if (usage < 0 || usage > 0xffff)
            return failure("usage_must_be_between_0_and_65535");
        const QByteArray encoded = KeyboardProtocol::encodeRawKeycode(
            static_cast<quint16>(usage), input.value("pressed").toBool());
        return success({{"hex", KeyboardProtocol::hexDump(encoded)}});
    }
    if (action == "build_frame") {
        const int command = input.value("command").toInt(-1);
        QByteArray payload;
        if (command < 0 || command > 0xff)
            return failure("command_must_be_between_0_and_255");
        if (!parseHex(input.value("payload_hex").toString(), &payload))
            return failure("payload_hex_is_invalid");
        return success({{"hex", KeyboardProtocol::hexDump(
                                   KeyboardProtocol::buildFrame(static_cast<quint8>(command), payload))}});
    }
    if (action == "validate_text") {
        const TextMappingResult mapping = KeyboardProtocol::validateTextMapping(input.value("text").toString());
        QJsonArray unmapped;
        for (const QString &character : mapping.unmappedCharacters)
            unmapped.append(character);
        return success({{"mapping_ok", mapping.ok()},
                        {"total_chars", mapping.totalChars},
                        {"mapped_chars", mapping.mappedChars},
                        {"unmapped", unmapped}});
    }
    if (action == "key_usage") {
        quint16 usage = 0;
        const bool found = KeyboardProtocol::keyUsageForCapturedInput(
            input.value("key").toString(), input.value("text").toString(), &usage);
        QJsonObject result{{"found", found}};
        if (found)
            result.insert("usage", static_cast<int>(usage));
        return success(result);
    }
    if (action == "describe_response") {
        const int response = input.value("response").toInt(-1);
        if (response < 0 || response > 0xff)
            return failure("response_must_be_between_0_and_255");
        return success({{"description", KeyboardProtocol::describeResponse(static_cast<quint8>(response))}});
    }
    return failure("unknown_keyboard_protocol_action");
}

QJsonObject mouseProtocol(const QString &action, const QJsonObject &input)
{
    if (action != "encode_report" && action != "describe_report")
        return failure("unknown_mouse_protocol_action");

    const int buttons = input.value("buttons").toInt(-1);
    const int dx = input.value("dx").toInt(-1000);
    const int dy = input.value("dy").toInt(-1000);
    const int wheel = input.value("wheel").toInt(0);
    if (buttons < 0 || buttons > MouseProtocol::ButtonMask)
        return failure("buttons_must_be_between_0_and_7");
    if (dx < MouseProtocol::MinimumDelta || dx > MouseProtocol::MaximumDelta
        || dy < MouseProtocol::MinimumDelta || dy > MouseProtocol::MaximumDelta
        || wheel < MouseProtocol::MinimumDelta || wheel > MouseProtocol::MaximumDelta) {
        return failure("mouse_deltas_must_be_between_minus_127_and_127");
    }

    const auto buttonMask = static_cast<quint8>(buttons);
    const auto reportDx = static_cast<qint8>(dx);
    const auto reportDy = static_cast<qint8>(dy);
    const auto reportWheel = static_cast<qint8>(wheel);
    if (action == "describe_report") {
        return success({{"description", MouseProtocol::describeReport(
                                            buttonMask, reportDx, reportDy, reportWheel)}});
    }
    return success({{"hex", KeyboardProtocol::hexDump(MouseProtocol::encodeReport(
                                buttonMask, reportDx, reportDy, reportWheel))},
                    {"description", MouseProtocol::describeReport(
                                        buttonMask, reportDx, reportDy, reportWheel)}});
}

QJsonObject mouseInput(const QString &action, const QJsonObject &input)
{
    if (action != "sequence")
        return failure("unknown_mouse_input_action");

    MouseInputManager manager;
    QJsonArray reports;
    QObject::connect(&manager, &MouseInputManager::reportRequested,
                     [&reports](quint8 buttons, qint8 dx, qint8 dy, qint8 wheel) {
        reports.append(QJsonObject{
            {"buttons", static_cast<int>(buttons)},
            {"dx", static_cast<int>(dx)},
            {"dy", static_cast<int>(dy)},
            {"wheel", static_cast<int>(wheel)},
            {"hex", KeyboardProtocol::hexDump(MouseProtocol::encodeReport(buttons, dx, dy, wheel))}
        });
    });

    for (const QJsonValue &value : input.value("operations").toArray()) {
        const QJsonObject operation = value.toObject();
        const QString type = operation.value("type").toString();
        if (type == "move") {
            manager.moveBy(operation.value("dx").toInt(), operation.value("dy").toInt());
        } else if (type == "wheel") {
            manager.wheelBy(operation.value("delta").toInt());
        } else if (type == "set_buttons") {
            const int buttons = operation.value("buttons").toInt(-1);
            if (buttons < 0 || buttons > MouseProtocol::ButtonMask)
                return failure("buttons_must_be_between_0_and_7");
            manager.setButtons(static_cast<quint8>(buttons));
        } else if (type == "button") {
            const int button = operation.value("button").toInt();
            if (button != MouseProtocol::LeftButton
                && button != MouseProtocol::RightButton
                && button != MouseProtocol::MiddleButton) {
                return failure("mouse_button_must_be_1_2_or_4");
            }
            manager.setButton(static_cast<quint8>(button), operation.value("pressed").toBool());
        } else if (type == "flush") {
            manager.flushPending();
        } else if (type == "release_all") {
            manager.releaseAll();
        } else if (type == "reset") {
            manager.resetState();
        } else {
            return failure("unknown_mouse_input_operation");
        }
    }

    return success({{"buttons", static_cast<int>(manager.buttons())},
                    {"flush_interval_ms", manager.flushIntervalMs()},
                    {"reports", reports}});
}

QJsonObject hudp(const QString &action, const QJsonObject &input)
{
    QByteArray data;
    const QString encoding = input.value("encoding").toString("utf8");
    if (encoding == "hex") {
        if (!parseHex(input.value("data").toString(), &data))
            return failure("data_hex_is_invalid");
    } else if (encoding == "utf8") {
        data = input.value("data").toString().toUtf8();
    } else {
        return failure("encoding_must_be_utf8_or_hex");
    }

    if (action == "crc32") {
        return success({{"crc32", QString("%1").arg(HudpProtocol::crc32(data), 8, 16, QChar('0')).toUpper()}});
    }
    if (action == "build") {
        const int type = input.value("type").toInt(-1);
        const int flags = input.value("flags").toInt(0);
        if (type < 0 || type > 0xff || flags < 0 || flags > 0xffff)
            return failure("type_or_flags_out_of_range");
        const QByteArray packet = HudpProtocol::build(
            static_cast<quint8>(type), static_cast<quint16>(flags),
            static_cast<quint32>(input.value("session_id").toDouble()),
            static_cast<quint32>(input.value("sequence").toDouble()),
            static_cast<quint32>(input.value("timestamp_ms").toDouble()), data);
        if (packet.isEmpty() && data.size() > HudpProtocol::MaxPayload)
            return failure("payload_exceeds_1024_bytes");
        return success({{"hex", KeyboardProtocol::hexDump(packet)}, {"size", packet.size()}});
    }
    return failure("unknown_hudp_protocol_action");
}

QJsonObject transportSelector(const QString &action, const QJsonObject &input)
{
    if (action != "select" && action != "sequence")
        return failure("unknown_transport_selector_action");

    FakeTransport ble("BLE");
    FakeTransport uart("UART");
    FakeTransport hudpTransport("HUDP");
    TransportSelector selector;
    int changes = 0;
    QObject::connect(&selector, &TransportSelector::activeTransportChanged,
                     [&changes]() { ++changes; });

    if (action == "select") {
        const QJsonObject connected = input.value("connected").toObject();
        ble.setConnected(connected.value("ble").toBool());
        uart.setConnected(connected.value("uart").toBool());
        hudpTransport.setConnected(connected.value("hudp").toBool());
        selector.setTransports(&ble, &uart, &hudpTransport);
        return success({{"active", selector.activeTransportName()}, {"changes", changes}});
    }

    selector.setTransports(&ble, &uart, &hudpTransport);
    QJsonArray active;
    for (const QJsonValue &value : input.value("states").toArray()) {
        const QJsonObject state = value.toObject();
        ble.setConnected(state.value("ble").toBool());
        uart.setConnected(state.value("uart").toBool());
        hudpTransport.setConnected(state.value("hudp").toBool());
        active.append(selector.activeTransportName());
    }
    return success({{"active", active}, {"changes", changes}});
}

QJsonObject inputQueue(const QString &action, const QJsonObject &input)
{
    if (action != "flush")
        return failure("unknown_input_queue_action");

    FakeTransport transport(input.value("transport_name").toString("FAKE"),
                            input.value("connected").toBool());
    InputQueue queue;
    QStringList errors;
    QJsonArray packetEvents;
    QObject::connect(&queue, &InputQueue::errorOccurred,
                     [&errors](const QString &error) { errors.append(error); });
    QObject::connect(&queue, &InputQueue::packetWritten,
                     [&packetEvents](const QString &name, const QString &hex, const QString &label) {
        packetEvents.append(QJsonObject{{"transport", name}, {"hex", hex}, {"label", label}});
    });
    queue.setActiveTransport(&transport);

    for (const QJsonValue &value : input.value("packets").toArray()) {
        const QJsonObject object = value.toObject();
        QByteArray bytes;
        if (!parseHex(object.value("hex").toString(), &bytes))
            return failure("packet_hex_is_invalid");
        Packet packet;
        packet.bytes = bytes;
        packet.label = object.value("label").toString();
        packet.priority = object.value("priority").toInt();
        queue.enqueue(packet);
    }

    if (input.value("connect_after_enqueue").toBool()) {
        transport.setConnected(true);
        queue.setActiveTransport(&transport);
    }

    QJsonArray written;
    for (const QByteArray &frame : transport.writtenFrames)
        written.append(KeyboardProtocol::hexDump(frame));
    QJsonArray errorArray;
    for (const QString &error : errors)
        errorArray.append(error);
    return success({{"written", written},
                    {"events", packetEvents},
                    {"errors", errorArray},
                    {"remaining", queue.queueSize()}});
}

QJsonObject autoClicker(const QString &action, const QJsonObject &input)
{
    if (action != "configure" && action != "run")
        return failure("unknown_auto_clicker_action");

    QList<quint16> keys;
    for (const QJsonValue &value : input.value("keys").toArray()) {
        const int key = value.toInt(-1);
        if (key < 0 || key > 0xffff)
            return failure("key_usage_out_of_range");
        keys.append(static_cast<quint16>(key));
    }

    AutoClickerManager clicker;
    clicker.setKeys(keys);
    clicker.setIntervalMs(input.value("interval_ms").toInt(100));
    clicker.setRepeatCount(input.value("repeat_count").toInt(0));
    if (action == "configure") {
        return success({{"keys", numbers(clicker.keys())},
                        {"interval_ms", clicker.intervalMs()},
                        {"repeat_count", clicker.repeatCount()},
                        {"running", clicker.running()}});
    }

    if (clicker.repeatCount() < 1 || clicker.repeatCount() > 20)
        return failure("run_requires_repeat_count_between_1_and_20");
    int emittedCycles = 0;
    QObject::connect(&clicker, &AutoClickerManager::clickRequested,
                     [&emittedCycles]() { ++emittedCycles; });
    QEventLoop loop;
    QObject::connect(&clicker, &AutoClickerManager::runningChanged, [&clicker, &loop]() {
        if (!clicker.running())
            loop.quit();
    });
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start(clicker.repeatCount() * clicker.intervalMs() + 250);
    clicker.start();
    if (clicker.running())
        loop.exec();
    const bool timedOut = clicker.running();
    if (timedOut)
        clicker.stop();
    return success({{"keys", numbers(clicker.keys())},
                    {"emitted_cycles", emittedCycles},
                    {"completed_cycles", clicker.completedCycles()},
                    {"running", clicker.running()},
                    {"timed_out", timedOut}});
}

QJsonObject logManager(const QString &action, const QJsonObject &input)
{
    if (action != "evaluate")
        return failure("unknown_log_manager_action");

    LogManager logs;
    logs.setLogFilePath("");
    logs.setDebugTrace(input.value("debug_trace").toBool(true));
    for (const QJsonValue &value : input.value("entries").toArray()) {
        const QJsonObject entry = value.toObject();
        logs.append(entry.value("level").toInt(), entry.value("module").toString(),
                    entry.value("message").toString(), entry.value("detail").toString());
    }
    if (input.contains("clear_level"))
        logs.clearLevel(input.value("clear_level").toInt());
    logs.switchLevel(input.value("view_level").toInt(0));
    return success({{"counts", QJsonObject{{"info", logs.infoCount()},
                                            {"warning", logs.warningCount()},
                                            {"error", logs.errorLogCount()},
                                            {"debug", logs.debugCount()}}},
                    {"all", cleanLogs(logs.allLogs())},
                    {"visible", cleanLogs(logs.logEntries())}});
}

QJsonObject keySettings(const QString &action, const QJsonObject &input)
{
    KeySettingManager settings;
    if (action == "reset") {
        settings.resetToDefaults();
    } else if (action == "import") {
        const QJsonObject values = input.value("values").toObject();
        if (!settings.importFromJson(QString::fromUtf8(QJsonDocument(values).toJson(QJsonDocument::Compact))))
            return failure("key_settings_import_failed");
    } else {
        return failure("unknown_key_settings_action");
    }
    return success({{"capture_enabled", settings.captureEnabled()},
                    {"capture_hotkey", settings.captureHotkey()},
                    {"capture_paste_hotkey", settings.capturePasteHotkey()}});
}

QJsonObject settingsManager(const QString &action, const QJsonObject &input)
{
    if (action != "state")
        return failure("unknown_settings_manager_action");
    SettingsManager settings(nullptr);
    settings.setAutoStart(input.value("auto_start").toBool());
    settings.setRememberLastConnection(input.value("remember_last_connection").toBool());
    settings.setLastConnection(input.value("transport").toString(), input.value("connected").toBool());
    settings.setLastUart(input.value("uart_port").toString(), input.value("uart_baudrate").toInt(115200));
    settings.setLastBleAddress(input.value("ble_address").toString());
    settings.setLastHudp(input.value("hudp_target").toString(), input.value("hudp_port").toInt(45820));
    return success({{"auto_start", settings.autoStart()},
                    {"remember_last_connection", settings.rememberLastConnection()},
                    {"transport", settings.lastConnectionTransport()},
                    {"connected", settings.lastConnectionConnected()},
                    {"uart_port", settings.lastUartPort()},
                    {"uart_baudrate", settings.lastUartBaudrate()},
                    {"ble_address", settings.lastBleAddress()},
                    {"hudp_target", settings.lastHudpTarget()},
                    {"hudp_port", settings.lastHudpPort()}});
}

QJsonObject debugTrace(const QString &action, const QJsonObject &input)
{
    if (action != "set")
        return failure("unknown_debug_trace_action");
    DebugTrace::setEnabled(input.value("enabled").toBool());
    return success({{"enabled", DebugTrace::enabled()}});
}

} // namespace

QJsonObject ModuleTestDispatcher::dispatch(const QJsonObject &request)
{
    const QString module = request.value("module").toString();
    const QString action = request.value("action").toString();
    const QJsonObject input = request.value("input").toObject();
    QJsonObject response;

    if (module.isEmpty() || action.isEmpty())
        response = failure("module_and_action_are_required");
    else if (module == "keyboard_protocol")
        response = keyboard(action, input);
    else if (module == "mouse_protocol")
        response = mouseProtocol(action, input);
    else if (module == "mouse_input")
        response = mouseInput(action, input);
    else if (module == "hudp_protocol")
        response = hudp(action, input);
    else if (module == "transport_selector")
        response = transportSelector(action, input);
    else if (module == "input_queue")
        response = inputQueue(action, input);
    else if (module == "auto_clicker")
        response = autoClicker(action, input);
    else if (module == "log_manager")
        response = logManager(action, input);
    else if (module == "key_settings")
        response = keySettings(action, input);
    else if (module == "settings_manager")
        response = settingsManager(action, input);
    else if (module == "debug_trace")
        response = debugTrace(action, input);
    else if (module == "system" && action == "describe")
        response = describe();
    else
        response = failure("unknown_module");

    return withId(response, request);
}

QJsonObject ModuleTestDispatcher::describe()
{
    return {{"ok", true},
            {"protocol", "jsonl-v1"},
            {"request_shape", QJsonObject{{"id", "optional"},
                                           {"module", "string"},
                                           {"action", "string"},
                                           {"input", "object"}}},
            {"modules", QJsonObject{
                {"keyboard_protocol", QJsonArray{"encode_text", "encode_raw_key", "build_frame", "validate_text", "key_usage", "describe_response"}},
                {"mouse_protocol", QJsonArray{"encode_report", "describe_report"}},
                {"mouse_input", QJsonArray{"sequence"}},
                {"hudp_protocol", QJsonArray{"crc32", "build"}},
                {"transport_selector", QJsonArray{"select", "sequence"}},
                {"input_queue", QJsonArray{"flush"}},
                {"auto_clicker", QJsonArray{"configure", "run"}},
                {"log_manager", QJsonArray{"evaluate"}},
                {"key_settings", QJsonArray{"reset", "import"}},
                {"settings_manager", QJsonArray{"state"}},
                {"debug_trace", QJsonArray{"set"}}
            }}};
}
