# 模块测试 CLI：AI 调用指南

`module_test_cli` 用来让 AI 或脚本通过标准输入调用可测试模块，并用标准输出中的 JSON 验证结果。它不会连接真实串口、BLE、WinRT、UDP 设备或 ESP32。

## 构建与回归测试

在仓库根目录执行：

```powershell
.\src\test\build_tests.bat
```

默认可执行文件位置：

```text
src/test/build_tests/Debug/module_test_cli.exe
```

如需查看 CLI 当前支持的模块：

```powershell
$env:PATH = "E:\Qt\6.11.1\msvc2022_64\bin;$env:PATH"
.\src\test\build_tests\Debug\module_test_cli.exe --describe
```

Qt 安装位置可能不同；通过 `build_tests.bat` 运行回归测试时，CTest 会自动加入当前 Qt Core 的运行库目录。

## 标准输入/输出协议

协议名为 `jsonl-v1`：标准输入每行一个 JSON 对象，标准输出按相同顺序每行返回一个 JSON 对象。不要跨行格式化一个请求。

请求结构：

```json
{"id":"optional-correlation-id","module":"keyboard_protocol","action":"encode_text","input":{"text":"abc"}}
```

成功响应：

```json
{"id":"optional-correlation-id","ok":true,"result":{"hex":"74 79 70 65 20 61 62 63 0A","utf8":"type abc\n"}}
```

失败响应：

```json
{"id":"optional-correlation-id","ok":false,"error":"machine_readable_error"}
```

调用一条请求：

```powershell
'{"module":"keyboard_protocol","action":"encode_raw_key","input":{"usage":4,"pressed":true}}' |
    .\src\test\build_tests\Debug\module_test_cli.exe
```

一次调用多条请求：

```powershell
@'
{"id":1,"module":"hudp_protocol","action":"crc32","input":{"data":"123456789","encoding":"utf8"}}
{"id":2,"module":"transport_selector","action":"select","input":{"connected":{"ble":false,"uart":true,"hudp":true}}}
'@ | .\src\test\build_tests\Debug\module_test_cli.exe
```

也可不用管道调用单条请求：

```powershell
.\src\test\build_tests\Debug\module_test_cli.exe --request `
    '{"module":"debug_trace","action":"set","input":{"enabled":false}}'
```

## 如何判定测试结果

调用方应解析每一行 JSON，并依次检查：

1. 输出行数与非空输入行数相同。
2. `id` 与请求一致（请求没有 `id` 时响应也没有）。
3. `ok` 为 `true`。
4. `result` 与预期对象做结构化比较，不要比较 JSON 字段顺序。

标准输入流模式下，业务请求错误会返回 `ok:false`，进程仍可继续处理后续行。因此不要只检查进程退出码。

运行仓库内全部固定契约：

```powershell
.\src\test\build_tests\Debug\module_test_cli.exe --verify `
    .\src\test\cases\unit_cases.jsonl
```

`--verify` 在全部通过时返回退出码 `0`，有不匹配时返回 `1`，用例文件不可读时返回 `2`。

## 模块与动作

### `keyboard_protocol`

| action | input | result |
| --- | --- | --- |
| `encode_text` | `text:string` | `hex`、`utf8` |
| `encode_raw_key` | `usage:0..65535`、`pressed:bool` | `hex` |
| `build_frame` | `command:0..255`、`payload_hex:string` | `hex` |
| `validate_text` | `text:string` | 映射计数与 `unmapped` |
| `key_usage` | `key:string`、`text:string` | `found`，找到时含 `usage` |
| `describe_response` | `response:0..255` | `description` |

十六进制输入允许空格，输出固定为大写、每字节一个空格。

### `mouse_protocol`

| action | input | result |
| --- | --- | --- |
| `encode_report` | `buttons:0..7`、`dx/dy/wheel:-127..127` | `hex`、`description` |
| `describe_report` | 同上 | `description` |

`buttons` 是完整位图：左键 `1`、右键 `2`、中键 `4`，可按位组合。
帧固定为 `AA 06 04 00 <buttons> <dx> <dy> <wheel>`，三个相对量使用
8 位二进制补码。例如：

```powershell
'{"module":"mouse_protocol","action":"encode_report","input":{"buttons":1,"dx":12,"dy":-8,"wheel":0}}' |
    .\src\test\build_tests\Debug\module_test_cli.exe
```

### `mouse_input`

`sequence` 按顺序执行 `input.operations`，返回 manager 发出的完整鼠标报告：

```json
{
  "module": "mouse_input",
  "action": "sequence",
  "input": {
    "operations": [
      {"type":"move", "dx":300, "dy":-20},
      {"type":"button", "button":1, "pressed":true},
      {"type":"move", "dx":4, "dy":2},
      {"type":"release_all"}
    ]
  }
}
```

支持的 operation：

- `move`：累加 `dx`、`dy`，约 4 ms（250 Hz）后自动发送；测试可用 `flush` 确定性触发。
- `wheel`：累加 `delta`。
- `button`：`button` 为 `1`、`2` 或 `4`，切换前先 flush 旧按钮状态下的移动。
- `set_buttons`：直接设置 `buttons:0..7` 完整位图，模拟捕获模块输入。
- `flush`：立即分片发出待处理相对量。
- `release_all`：先 flush，再发送全按钮释放报告。
- `reset`：丢弃待处理输入并清除本地按钮状态，不发送报告。

单帧相对量限制为 `-127..127`；大位移会按顺序拆成多帧，不会截断。

### `hudp_protocol`

| action | input | result |
| --- | --- | --- |
| `crc32` | `data:string`、`encoding` 为 `utf8` 或 `hex` | 8 位大写 `crc32` |
| `build` | `type`、`flags`、`session_id`、`sequence`、`timestamp_ms`、`data`、`encoding` | HUDP 包的 `hex` 与 `size` |

`build` 的载荷上限为 1024 字节。

### `transport_selector`

`select` 输入三个连接状态，返回按 `BLE > UART > HUDP` 选择的 `active`：

```json
{"module":"transport_selector","action":"select","input":{"connected":{"ble":false,"uart":true,"hudp":true}}}
```

`sequence` 输入 `states` 数组，返回每一步的活动传输数组以及 `changes` 信号次数。这里使用内存中的假传输，不访问硬件。

### `input_queue`

`flush` 输入：

```json
{
  "connected": false,
  "connect_after_enqueue": true,
  "transport_name": "FAKE",
  "packets": [
    {"hex": "AA 01", "label": "first", "priority": 0}
  ]
}
```

结果包含 `written`、`events`、`errors` 和 `remaining`。假传输能验证阻塞、重连和写出顺序，不连接真实设备。

### `auto_clicker`

- `configure`：输入 `keys`、`interval_ms`、`repeat_count`，验证去重与边界归一化。
- `run`：执行有限次数，`repeat_count` 必须为 1 到 20；结果包含发射次数、完成次数和超时标记。

### `log_manager`

`evaluate` 接收 `entries`，以及可选的 `debug_trace`、`view_level`、`clear_level`。结果返回计数、全部日志与当前可见日志。为了保证输出可比较，响应会去掉运行时生成的时间戳。

### `key_settings`

- `reset`：返回默认快捷键状态。
- `import`：从 `input.values` 导入 `captureEnabled`、`captureHotkey`、`capturePasteHotkey`。

CLI 把 `QSettings` 重定向到每次进程专属的临时目录，不污染用户配置。

### `settings_manager`

`state` 验证设置对象的内存状态与连接信息 setter/getter。它不会调用 `save`、`load` 或 `applyAutoStart`，因此不会改文件或系统开机启动项。

### `debug_trace`

`set` 接收 `enabled:bool`，返回模块当前状态。

## AI 调用建议

- 先调用 `--describe` 获取当前能力，不要凭旧文档猜 action。
- 为并行生成的请求设置唯一 `id`。
- 测试二进制协议时使用 `encoding:"hex"`，避免终端文本编码影响载荷。
- 测试鼠标顺序时显式加入 `flush`，避免依赖真实计时。
- 对预期错误也应明确比较 `ok:false` 与 `error`，例如越界 usage。
- 需要新增覆盖时，优先把稳定输入/输出加入 `cases/unit_cases.jsonl`，再运行 `build_tests.bat`。
