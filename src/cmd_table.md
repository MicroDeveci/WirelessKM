# InterfaceLayer command table

| Command | Params | Target |
|---|---|---|
| `home.navigateTo` | `route: string` | UI navigation |
| `key.captureHotkeyChanged` | `newHotkey: string` | Settings |
| `key.capturePasteHotkeyChanged` | `newHotkey: string` | Settings |
| `key.captureToggled` | `enabled: bool` | Settings |
| `hudp.connect` | `target: string, port: int` | Device link |
| `hudp.disconnect` | - | Device link |
| `ble.startScan` | - | Device link |
| `ble.stopScan` | - | Device link |
| `ble.connect` | `address: string` | Device link |
| `ble.disconnect` | - | Device link |
| `uart.refreshPorts` | - | Device link |
| `uart.connect` | `port: string, baudrate: int` | Device link |
| `uart.disconnect` | - | Device link |
| `text.send` | `text: string` | Input |
| `clipboard.send` | - | Input |
| `clipboard.watchToggled` | `enabled: bool` | Input |
| `passthrough.listenToggled` | `enabled: bool` | Passthrough |
| `passthrough.exclusiveToggled` | `enabled: bool` | Passthrough |
| `passthrough.mouseListenToggled` | `enabled: bool` | App-foreground mouse passthrough |
| `mouse.move` | `dx: int, dy: int` | Relative mouse input |
| `mouse.button` | `button: 1/2/4, pressed: bool` | Mouse button input |
| `mouse.wheel` | `delta: int` | Relative mouse wheel input |
| `mouse.releaseAll` | - | Release all mouse buttons |
| `log.switchLevel` | `level: int` | Log |
| `log.select` | `index: int` | Log |
| `log.clear` | `level: int` | Log |
| `log.exportLogs` | - | Log JSON export |
| `log.refresh` | - | Log |
| `log.copyDetail` | - | Log |
| `app.checkUpdate` | - | App |
| `app.openGitHub` | - | App |
| `app.openSponsorLink` | - | App |
| `settings.autoStartToggled` | `enabled: bool` | Settings |
| `settings.rememberLastConnectionToggled` | `enabled: bool` | Settings |
| `settings.importConfig` | `path: string` | Settings |
| `settings.resetToDefaults` | - | Settings |
