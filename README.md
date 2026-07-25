# WirelessKM

ESP32 wireless HID bridge desktop controller. Transmits keyboard, mouse, and text
input from this PC to an ESP32 over BLE, UART, or HUDP. The ESP32 injects them
as USB HID on the target machine.

---

## Architecture

```
Desktop App (Qt/QML)
  |
  | BLE / UART / HUDP
  v
ESP32 HID Bridge
  |
  | USB HID
  v
Target Computer
```

The desktop app captures keyboard, mouse, and text on the local machine, encodes
them into HID Bridge v3 protocol frames, and sends them to the ESP32. The ESP32
decodes the frames and injects them as native USB HID input on the target computer.

Detailed architecture documentation: [docs/structure.md](docs/structure.md)

---

## Features

- Keyboard passthrough — type on a remote machine
- Mouse passthrough — control a remote machine's mouse
- Text and clipboard transfer — paste text remotely
- Auto-clicker — timed key repeat sequences
- Three transports — BLE (wireless), UART (serial), HUDP (network)
- Multi-language UI — Chinese / English
- Dark mode
- Settings import/export

<!-- Screenshots (add your images below) -->
<!-- ![Home](docs/screenshot_home.png) -->
<!-- ![Connection](docs/screenshot_connection.png) -->
<!-- ![Auto Clicker](docs/screenshot_clicker.png) -->

---

## Firmware

ESP32 HID Bridge firmware repository:

https://github.com/MicroDeveci/WirelessKM

---

## Build

### Requirements

- CMake 3.20+
- Qt 6 (with SerialPort, Bluetooth, Network, Widgets, PrintSupport modules)
- Visual Studio 2022+ (Windows) or GCC/Clang (Linux)
- Qt Linguist tools (lupdate, lrelease) for translations

### Windows (PowerShell)

Set `QT_ROOT` to your local Qt installation directory. The path below is an example; select the Qt kit that matches your compiler (for example, `msvc2022_64`).

Release build:

```powershell
$env:QT_ROOT = "C:\path\to\Qt\6.x.x\msvc2022_64"
cmake --workflow --preset build-release
```

Debug build:

```powershell
$env:QT_ROOT = "C:\path\to\Qt\6.x.x\msvc2022_64"
cmake --workflow --preset build-debug
```

Output: `bin/Release/wirelesskm.exe` or `bin/Debug/wirelesskm.exe`

### Linux

```bash
export QT_ROOT=/path/to/qt/6.x.x/gcc_64
cmake --preset debug
cmake --build --preset debug
```

Or using the workflow preset:

```bash
cmake --workflow --preset build-debug
```

### Run Tests

```bash
cmake --build src/test/build_tests --config Debug
ctest --test-dir src/test/build_tests -C Debug --output-on-failure
```

---

## Project Structure

```
wirelesskm/
  CMakeLists.txt          Root build configuration
  CMakePresets.json        Build presets (debug/release)
  FluentUI/               FluentUI QML component library
  src/
    main.cpp              Application entry point
    InterfaceLayer.*      Central coordinator, QML facade
    InterfaceLayer_routes_*.cpp   Command route handlers (11 files)
    BleManager.*          BLE scan/connect/write
    SerialManager.*       UART serial port management
    HudpProtocol.*        UDP protocol encoding
    KeyboardProtocol.*    Keyboard/text HID frame encoding
    MouseProtocol.*       Mouse HID report encoding
    InputQueue.*          Packet buffer and flush
    CaptureManager.*      Global hotkey and app capture
    SettingsManager.*     Persistent JSON configuration
    qml/                  QML UI files
    test/                 Unit tests (33 test cases)
  docs/
    structure.md          Detailed architecture documentation
```

---

## TODO

1. Add firmware screen and button support
2. Add clicker macro functionality
3. Refactor UI

---

## Credits

FluentUI — Qt/QML Fluent Design component library
https://github.com/zhuzichu520/FluentUI

---

## License

This project is licensed under the GNU Affero General Public License v3.0 or later (AGPL-3.0-or-later). See [LICENSE](LICENSE) for the full terms.
