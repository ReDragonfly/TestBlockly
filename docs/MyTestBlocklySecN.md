# 第二阶段设计方案（MVP 精简版）：典型积木 + C++ 接口（真实/仿真） + 完整自动测试运行

## 文档修订说明

**本次修订的核心变化**：

| 项目 | 原方案 | 本方案（MVP） |
|------|--------|--------------|
| 积木数量 | 40+ | **10 个典型积木** |
| 积木清单 | 未明确列出 | **逐一定义（输入/输出/生成代码）** |
| 文档自包含性 | 需对照 `Beam-MU-Debug.lua` 阅读 | **不依赖外部文件，每个积木的业务含义在文档内说明** |
| 扩展路径 | 未定义 | **明确从 10 到 40 的增量扩展路线** |
| 实施工时 | 31.5 天 | **约 15 个工作日** |

**MVP 策略**：先用 10 个典型积木 + Blockly 内置流程控制积木，跑通"设置 → 测量 → 判断 → 上报"的完整自动测试闭环；验证通过后再按扩展路径逐批增加到完整积木体系。

**典型验证场景**（贯穿全文）：**"上行功率定标"** —— 设置信号源给设备输入 -30 dBm 信号，循环调节设备增益微调参数，读取频谱仪功率，判断是否达到 6±0.5 dBm，上报 Pass/Fail。

---

## 一、概述

### 1.1 阶段目标

1. **10 个典型积木**：覆盖信号源控制、频谱仪控制、设备参数配置、测试结果上报四大类，配合 Blockly 内置的循环/条件/变量/数学积木，能搭建完整的自动化测试流程。
2. **C++ 接口层**：控制器层 + 后端抽象层，同一套控制器代码同时支持 VISA 真实仪器与纯软件仿真。
3. **仿真硬件**：信号源、频谱仪、OMT 设备的软件仿真，含场景预设（正常/信号丢失/频率偏移等），脱离真实硬件可完成功能验证与使用体验评估。
4. **完整自动测试运行**：点击运行 → 实时日志 → 步骤高亮 → 结果表格实时追加 → 支持中断。

### 1.2 与第一阶段的衔接

第一阶段验证的技术链路（Qt + WebEngine + Blockly + QWebChannel + Lua）保持不变。本阶段在此基础上：

- **纵向**：积木从 3 个验证积木扩展到 10 个典型业务积木。
- **横向**：C++ 接口从模拟返回值改为真实的"控制器 + 后端抽象"架构。
- **闭环**：新增完整的运行反馈机制（结果上报/步骤高亮/中断）。

### 1.3 MVP 验证标准

以下 5 项全部满足，即认为 MVP 阶段完成：

1. 用户在前端用 10 个积木 + 内置流程积木，能搭建出"上行功率定标"完整流程。
2. 生成的 Lua 代码语法正确，能被 ScriptEngine 执行。
3. 仿真模式下，积木链能完整执行，结果表格实时显示 Pass/Fail。
4. 执行过程中积木逐块高亮，日志实时滚动，支持中途停止。
5. 切换到真实硬件后端（`--simulate` 去掉 + VISA 库 + 仪器地址配置正确）后，不改一行积木代码即可运行。

---

## 二、典型积木清单（10 个）

### 2.1 积木总览

| 序号 | 积木类型 | 显示文本 | 类别 | 类型 | 对应业务操作 |
|------|---------|---------|------|------|------------|
| 1 | `sg_set_cw` | 设置信号源 CW | statement | 信号源 | 给被测设备输入指定频率和功率的信号 |
| 2 | `sg_rf_on` | 打开信号源输出 | statement | 信号源 | 射频开 |
| 3 | `sg_rf_off` | 关闭信号源输出 | statement | 信号源 | 射频关 |
| 4 | `sa_set_cw` | 设置频谱仪 CW 测量 | statement | 频谱仪 | 配置频谱仪测量中心频率和参考电平 |
| 5 | `sa_get_power` | 读取频谱仪功率 | **value** | 频谱仪 | 获取当前测量功率值（dBm） |
| 6 | `omt_set_param` | 设置设备参数 | statement | 设备参数 | 向设备写入一个 OID 参数值 |
| 7 | `omt_get_param` | 读取设备参数 | **value** | 设备参数 | 从设备读取一个 OID 参数值 |
| 8 | `test_send_result` | 上报测试结果 | statement | 测试反馈 | 输出一条 Pass/Fail 测试记录 |
| 9 | `test_wait` | 等待毫秒 | statement | 测试反馈 | 延时（等待设备状态稳定） |
| 10 | `test_log` | 输出日志 | statement | 测试反馈 | 在日志面板打印消息 |

**配合使用的 Blockly 内置积木**（已在第一阶段的 `blocks_compressed.js` 中提供）：
- `controls_for`（计次循环）、`controls_whileUntil`（条件循环）
- `controls_if` / `controls_ifelse`（条件分支）
- `logic_compare`（比较）、`logic_operation`（与或）、`logic_negate`（非）
- `math_arithmetic`、`math_single`（abs/round 等）、`math_number`
- `variables_set` / `variables_get`（变量读写）

### 2.2 积木详细定义

#### 积木 1：`sg_set_cw` —— 设置信号源 CW

**业务含义**：配置信号源输出连续波（CW）信号，作为被测设备的输入激励。频率、功率、线损三个参数决定实际到达设备输入端的信号。

**积木外观**：

```
┌─────────────────────────────────────┐
│ 设置信号源 CW                       │
│ 频率 [    1745   ] MHz              │
│ 功率 [    -30    ] dBm              │
│ 线损 [    2.5    ] dB               │
└─────────────────────────────────────┘
```

**JSON 定义**：

```javascript
{
    "type": "sg_set_cw",
    "message0": "设置信号源 CW",
    "message1": "频率 %1 MHz",
    "args1": [{ "type": "input_value", "name": "FREQ", "check": "Number" }],
    "message2": "功率 %1 dBm",
    "args2": [{ "type": "input_value", "name": "POWER", "check": "Number" }],
    "message3": "线损 %1 dB",
    "args3": [{ "type": "input_value", "name": "LOSS", "check": "Number" }],
    "previousStatement": null,
    "nextStatement": null,
    "colour": 230,
    "tooltip": "设置信号源为CW模式，实际输出功率 = 设定功率 - 线损"
}
```

**生成代码**：

```javascript
generator.forBlock['sg_set_cw'] = function(block, gen) {
    const freq  = gen.valueToCode(block, 'FREQ',  gen.ORDER_ATOMIC) || '0';
    const power = gen.valueToCode(block, 'POWER', gen.ORDER_ATOMIC) || '0';
    const loss  = gen.valueToCode(block, 'LOSS',  gen.ORDER_ATOMIC) || '0';
    return `reportStep("${block.id}")\n` +
           `setSgCW(${freq}, ${loss}, ${power})\n`;
};
```

**生成示例**：频率 1745、功率 -30、线损 2.5 →

```lua
reportStep("blk_001")
setSgCW(1745, 2.5, -30)
```

**C++ 侧调用链**：`l_setSgCW` → `SignalGeneratorController::setSgCW(1745, 2.5, -30)` → 后端 `write(":SOUR:FREQ 1745 MHz")` + `write(":SOUR:POW -32.5 dBm")`。

---

#### 积木 2 / 3：`sg_rf_on` / `sg_rf_off` —— 信号源射频开关

**业务含义**：控制信号源射频输出的开/关。通常在设置完 CW 参数后开启输出，测试结束时关闭。

**JSON 定义**：

```javascript
{
    "type": "sg_rf_on",
    "message0": "打开信号源输出",
    "previousStatement": null,
    "nextStatement": null,
    "colour": 230
}
```

```javascript
{
    "type": "sg_rf_off",
    "message0": "关闭信号源输出",
    "previousStatement": null,
    "nextStatement": null,
    "colour": 230
}
```

**生成代码**：

```javascript
generator.forBlock['sg_rf_on'] = function(block, gen) {
    return `reportStep("${block.id}")\nsetSgRfOn()\n`;
};
generator.forBlock['sg_rf_off'] = function(block, gen) {
    return `reportStep("${block.id}")\nsetSgRfOff()\n`;
};
```

**C++ 侧**：`l_setSgRfOn` → `SignalGeneratorController::setSgRfOn()` → 后端 `write(":OUTP ON")`。

---

#### 积木 4：`sa_set_cw` —— 设置频谱仪 CW 测量

**业务含义**：配置频谱仪以 CW 模式测量指定中心频率的功率，参考电平决定显示范围。通常在读取功率之前调用。

**积木外观**：

```
┌─────────────────────────────────────┐
│ 设置频谱仪 CW 测量                  │
│ 中心频率 [  1745  ] MHz             │
│ 参考电平 [   20   ] dBm             │
└─────────────────────────────────────┘
```

**JSON 定义**：

```javascript
{
    "type": "sa_set_cw",
    "message0": "设置频谱仪 CW 测量",
    "message1": "中心频率 %1 MHz",
    "args1": [{ "type": "input_value", "name": "FREQ", "check": "Number" }],
    "message2": "参考电平 %1 dBm",
    "args2": [{ "type": "input_value", "name": "REF", "check": "Number" }],
    "previousStatement": null,
    "nextStatement": null,
    "colour": 230
}
```

**生成代码**：

```javascript
generator.forBlock['sa_set_cw'] = function(block, gen) {
    const freq = gen.valueToCode(block, 'FREQ', gen.ORDER_ATOMIC) || '0';
    const ref  = gen.valueToCode(block, 'REF',  gen.ORDER_ATOMIC) || '20';
    return `reportStep("${block.id}")\nsetSaCW(${freq}, 0, ${ref})\n`;
};
```

**C++ 侧**：`l_setSaCW` → `SpectrumAnalyzerController::setSaCW()` → 后端 `write(":CONF:SAN")` + `write(":DISP:WIND:TRAC:Y:RLEV 20 dBm")` + `write(":SENS:FREQ:CENT 1745 MHz")`。

---

#### 积木 5：`sa_get_power` —— 读取频谱仪功率（value）

**业务含义**：读取频谱仪当前测量的功率值（dBm）。作为值积木，通常赋值给变量或直接参与比较运算。

**积木外观**：

```
┌──────────────────────────┐
│ [读取频谱仪功率]          │
└──────────────────────────┘
```

**JSON 定义**：

```javascript
{
    "type": "sa_get_power",
    "message0": "读取频谱仪功率",
    "output": "Number",
    "colour": 230,
    "tooltip": "返回当前测量的功率值（dBm）"
}
```

**生成代码**：

```javascript
generator.forBlock['sa_get_power'] = function(block, gen) {
    return ['getSaPower()', gen.ORDER_FUNCTION_CALL];
};
```

**C++ 侧**：`l_getSaPower` → `SpectrumAnalyzerController::getSaPower()` → 后端 `query("*OPC?")` + `query(":CALC:MARK1:Y?")` → 返回 `-5.23` 之类的数值。

---

#### 积木 6：`omt_set_param` —— 设置设备参数

**业务含义**：通过私有协议向被测设备写入一个 OID 参数（如增益微调值、中心频点、开关状态等）。OID 编码和参数含义由设备协议定义。

**积木外观**：

```
┌──────────────────────────────────────────┐
│ 设置设备参数 [上行增益微调1 ▼] 值 [ 12 ]  │
└──────────────────────────────────────────┘
```

**JSON 定义**：

```javascript
{
    "type": "omt_set_param",
    "message0": "设置设备参数 %1 值 %2",
    "args0": [
        {
            "type": "field_dropdown",
            "name": "OID",
            "options": [
                ["频段选择1 (0C10)",          "0C10"],
                ["中心频点1 (0C11)",          "0C11"],
                ["带宽1 (0C12)",              "0C12"],
                ["上行AD发送增益微调1 (0A11)", "0A11"],
                ["下行AD发送增益微调1 (0A12)", "0A12"]
            ]
        },
        { "type": "input_value", "name": "VALUE", "check": "Number" }
    ],
    "previousStatement": null,
    "nextStatement": null,
    "colour": 160
}
```

> **说明**：MVP 阶段 OID 下拉只保留 5 个最高频的参数（频段/频点/带宽/上下行增益微调）。其他 OID 在扩展阶段按需追加。

**生成代码**：

```javascript
generator.forBlock['omt_set_param'] = function(block, gen) {
    const oid   = block.getFieldValue('OID');
    const value = gen.valueToCode(block, 'VALUE', gen.ORDER_ATOMIC) || '0';
    return `reportStep("${block.id}")\n` +
           `setOmtParam("${oid}", ${value})\n`;
};
```

**生成示例**：OID=0A11、值=12 →

```lua
reportStep("blk_005")
setOmtParam("0A11", 12)
```

**C++ 侧**：`l_setOmtParam` → `DeviceController::setOmtParam("0A11", 12)` → 构造私有协议帧 → TCP 发送 → 等待响应。

---

#### 积木 7：`omt_get_param` —— 读取设备参数（value）

**业务含义**：从被测设备读取一个 OID 参数值。用于验证设置是否生效，或读取设备当前状态。

**JSON 定义**：

```javascript
{
    "type": "omt_get_param",
    "message0": "读取设备参数 %1",
    "args0": [
        {
            "type": "field_dropdown",
            "name": "OID",
            "options": [
                ["频段选择1 (0C10)",          "0C10"],
                ["中心频点1 (0C11)",          "0C11"],
                ["带宽1 (0C12)",              "0C12"],
                ["上行AD发送增益微调1 (0A11)", "0A11"],
                ["下行AD发送增益微调1 (0A12)", "0A12"]
            ]
        }
    ],
    "output": "Number",
    "colour": 160
}
```

**生成代码**：

```javascript
generator.forBlock['omt_get_param'] = function(block, gen) {
    const oid = block.getFieldValue('OID');
    return [`getOmtParam("${oid}")`, gen.ORDER_FUNCTION_CALL];
};
```

---

#### 积木 8：`test_send_result` —— 上报测试结果

**业务含义**：输出一条测试记录到结果表格。判断是否 Pass 由用户在积木中通过条件逻辑决定，本积木只负责上报。

**积木外观**：

```
┌──────────────────────────────────────────────────┐
│ 上报测试结果                                      │
│ 链路 [功率定标] 项目 [输出功率]                   │
│ 条件 [        ] 目标 [       ]                    │
│ 实测 [        ] 单位 [dBm]                        │
│ 结果 [Pass ▼]                                     │
└──────────────────────────────────────────────────┘
```

**JSON 定义**：

```javascript
{
    "type": "test_send_result",
    "message0": "上报测试结果",
    "message1": "链路 %1 项目 %2",
    "args1": [
        { "type": "field_input", "name": "LINK", "text": "功率定标" },
        { "type": "field_input", "name": "ITEM", "text": "输出功率" }
    ],
    "message2": "条件 %1 目标 %2",
    "args2": [
        { "type": "input_value", "name": "COND",   "check": "String" },
        { "type": "input_value", "name": "TARGET", "check": "String" }
    ],
    "message3": "实测 %1 单位 %2",
    "args3": [
        { "type": "input_value", "name": "VALUE" },
        { "type": "field_input", "name": "UNIT", "text": "dBm" }
    ],
    "message4": "结果 %1",
    "args4": [{
        "type": "field_dropdown",
        "name": "STATUS",
        "options": [["Pass","Pass"], ["Fail","Fail"]]
    }],
    "previousStatement": null,
    "nextStatement": null,
    "colour": 210
}
```

**生成代码**：

```javascript
generator.forBlock['test_send_result'] = function(block, gen) {
    const link   = block.getFieldValue('LINK');
    const item   = block.getFieldValue('ITEM');
    const unit   = block.getFieldValue('UNIT');
    const status = block.getFieldValue('STATUS');
    const cond   = gen.valueToCode(block, 'COND',   gen.ORDER_ATOMIC) || "''";
    const target = gen.valueToCode(block, 'TARGET', gen.ORDER_ATOMIC) || "''";
    const value  = gen.valueToCode(block, 'VALUE',  gen.ORDER_ATOMIC) || '0';
    return `reportStep("${block.id}")\n` +
           `sendTestResult("${link}", "${item}", ${cond}, ${target}, ` +
           `tostring(${value}), "${unit}", "${status}")\n`;
};
```

**C++ 侧**：`l_sendTestResult` → 通过 `testResultReceived` 信号推送到前端结果表格。

---

#### 积木 9：`test_wait` —— 等待毫秒

**业务含义**：延时指定毫秒数。设备参数修改后，硬件需要时间稳定，读功率前通常要等 500-1000 ms。

**JSON 定义**：

```javascript
{
    "type": "test_wait",
    "message0": "等待 %1 毫秒",
    "args0": [{ "type": "input_value", "name": "MS", "check": "Number" }],
    "previousStatement": null,
    "nextStatement": null,
    "colour": 210
}
```

**生成代码**：

```javascript
generator.forBlock['test_wait'] = function(block, gen) {
    const ms = gen.valueToCode(block, 'MS', gen.ORDER_ATOMIC) || '1000';
    return `reportStep("${block.id}")\nsleep(${ms})\n`;
};
```

**C++ 侧**：`l_sleep` → `QThread::msleep(ms)`（在 Lua 专用线程中执行，不冻结 UI）。

---

#### 积木 10：`test_log` —— 输出日志

**业务含义**：在日志面板打印自定义消息，用于调试或记录过程信息。

**JSON 定义**：

```javascript
{
    "type": "test_log",
    "message0": "输出日志 %1",
    "args0": [{ "type": "input_value", "name": "MSG", "check": "String" }],
    "previousStatement": null,
    "nextStatement": null,
    "colour": 210
}
```

**生成代码**：

```javascript
generator.forBlock['test_log'] = function(block, gen) {
    const msg = gen.valueToCode(block, 'MSG', gen.ORDER_ATOMIC) || "''";
    return `showMsg(${msg})\n`;
};
```

**C++ 侧**：`l_showMsg` → `emit logMessage(...)` → 前端日志面板。

---

### 2.3 积木在工具箱中的分类

```
├── 信号源控制 (230)
│   ├── 设置信号源 CW
│   ├── 打开信号源输出
│   └── 关闭信号源输出
├── 频谱仪控制 (230)
│   ├── 设置频谱仪 CW 测量
│   └── [读取频谱仪功率]
├── 设备参数 (160)
│   ├── 设置设备参数
│   └── [读取设备参数]
├── 测试反馈 (210)
│   ├── 上报测试结果
│   ├── 等待毫秒
│   └── 输出日志
├── 逻辑 (210)
│   ├── 如果/否则
│   ├── 比较
│   └── 与/或/非
├── 循环 (120)
│   ├── 重复 N 次
│   ├── 计数循环 i 从 A 到 B
│   └── 当条件满足时循环
├── 变量 (330)
│   ├── 设置变量
│   └── [读取变量]
└── 数学 (230)
    ├── 数字
    ├── 算术运算
    └── 绝对值/四舍五入
```

**总计**：10 个自定义积木 + 约 15 个内置积木。

---

## 三、端到端场景演示："上行功率定标"

本节用一条完整的用户搭建流程，展示 10 个积木 + 内置积木如何协作完成一个真实测试。

### 3.1 测试需求

> 被测设备上行链路：信号源给设备输入 1745 MHz、-30 dBm 信号（线损 2.5 dB）；期望设备输出 6±0.5 dBm；如果未达标，调节设备 OID 0A11（上行增益微调），最多重试 20 次；每次调节后等待 500 ms 让硬件稳定；最终上报 Pass/Fail。

### 3.2 用户搭建的积木链

```
[输出日志 "开始功率定标"]
        ↓
[设置信号源 CW 频率 1745 功率 -30 线损 2.5]
        ↓
[打开信号源输出]
        ↓
[设置频谱仪 CW 测量 中心频率 1745 参考电平 20]
        ↓
[设置变量 增益微调 = 0]
        ↓
[设置变量 已通过 = false]
        ↓
[计数循环 i 从 1 到 20]
        ├── [等待 500 毫秒]
        ├── [设置变量 实测功率 = (读取频谱仪功率)]
        ├── [如果 (abs(实测功率 - 6) <= 0.5)]
        │       ├── [上报测试结果 链路"功率定标" 项目"输出功率"
        │       │    条件"1745MHz -30dBm" 目标"6±0.5"
        │       │    实测 (实测功率) 单位"dBm" 结果 Pass]
        │       ├── [设置变量 已通过 = true]
        │       └── [跳出循环]
        └── [否则]
                ├── [设置变量 增益微调 = 增益微调 + (6 - 实测功率) × 2]
                ├── [设置设备参数 0A11 值 (增益微调)]
                └── [输出日志 "调整增益微调到: (增益微调)"]
        ↓
[如果 (已通过 = false)]
        └── [上报测试结果 结果 Fail]
        ↓
[关闭信号源输出]
        ↓
[输出日志 "功率定标完成"]
```

### 3.3 生成的 Lua 代码

```lua
showMsg("开始功率定标")

reportStep("blk_sg")
setSgCW(1745, 2.5, -30)

reportStep("blk_rf")
setSgRfOn()

reportStep("blk_sa")
setSaCW(1745, 0, 20)

local gainOffset = 0
local passed = false

for i = 1, 20 do
    reportStep("blk_wait")
    sleep(500)

    reportStep("blk_read")
    local measuredPower = getSaPower()

    if math.abs(measuredPower - 6) <= 0.5 then
        reportStep("blk_pass")
        sendTestResult("功率定标", "输出功率", "1745MHz -30dBm",
                       "6±0.5", tostring(measuredPower), "dBm", "Pass")
        passed = true
        break
    else
        gainOffset = gainOffset + (6 - measuredPower) * 2
        reportStep("blk_set")
        setOmtParam("0A11", gainOffset)
        showMsg("调整增益微调到: " .. gainOffset)
    end
end

if not passed then
    reportStep("blk_fail")
    sendTestResult("功率定标", "输出功率", "1745MHz -30dBm",
                   "6±0.5", "未达标", "dBm", "Fail")
end

reportStep("blk_rfoff")
setSgRfOff()
showMsg("功率定标完成")
```

### 3.4 执行时的用户体验

| 时间轴 | 前端显示 | C++ 日志 |
|--------|---------|---------|
| 0.0s | 积木 [输出日志"开始功率定标"] 高亮 | `[SG-SIM] 频率设为 1745 MHz` |
| 0.1s | 积木 [设置信号源 CW] 高亮 | `[SG-SIM] 功率设为 -32.5 dBm` |
| 0.2s | 积木 [打开信号源输出] 高亮 | `[SG-SIM] RF ON` |
| 0.3s | 积木 [设置频谱仪 CW] 高亮 | `[SA-SIM] 中心频率 1745 MHz` |
| 0.5s | 积木 [等待 500 毫秒] 高亮 | — |
| 1.0s | 积木 [读取频谱仪功率] 高亮 | `[SA-SIM] 功率: -12.34 dBm` |
| 1.1s | 条件判断，进入"否则"分支 | — |
| 1.2s | 积木 [设置设备参数 0A11] 高亮 | `[SIM-OMT] 0A11: 0 → 36` |
| 1.5s | 循环第 2 次… | `[SA-SIM] 功率: -2.15 dBm` |
| … | … | … |
| 2.8s | 条件满足，[上报测试结果 Pass] 高亮 | 结果表格新增一行：功率定标 / 输出功率 / **Pass** |
| 3.0s | 循环跳出，[关闭信号源输出] 高亮 | `[SG-SIM] RF OFF` |
| 3.1s | 执行完成，状态栏变绿 "执行成功" | `执行完成，耗时 3.1s` |

用户在整个过程中：**看着积木逐块变黄 → 日志实时滚动 → 结果表格实时新增行 → 最终看到 Pass/Fail**。

---

## 四、整体架构

### 4.1 分层结构

```
┌──────────────────────────────────────────────────────┐
│ 前端 (WebEngine)                                      │
│ Blockly 工作区 + 工具箱 + 日志面板 + 结果表格         │
└────────────────────┬─────────────────────────────────┘
                     │ QWebChannel
┌────────────────────▼─────────────────────────────────┐
│ ScriptBridge                                          │
│ submitScript / abort / saveWorkspace                  │
│ 信号: executionFinished / logMessage                  │
│       testResultReceived / stepStarted                │
└────────────────────┬─────────────────────────────────┘
                     │
┌────────────────────▼─────────────────────────────────┐
│ ScriptEngine (Lua 5.4.6)                              │
│ 注册 10 个业务函数 + reportStep + sleep + showMsg     │
│ 专用线程 + 中断标志                                    │
└────────────────────┬─────────────────────────────────┘
                     │
┌────────────────────▼─────────────────────────────────┐
│ 控制器层                                              │
│ SignalGeneratorController  SpectrumAnalyzerController │
│ DeviceController                                      │
└────────────────────┬─────────────────────────────────┘
                     │
┌────────────────────▼─────────────────────────────────┐
│ 后端抽象层 IInstrumentBackend                          │
│  ├── VisaBackend       (真实仪器)                     │
│  └── SimulatorBackend  (软件仿真)                     │
└──────────────────────────────────────────────────────┘
```

### 4.2 关键设计：为什么保留后端抽象层

即便 MVP 阶段积木精简到 10 个，**后端抽象层仍需保留**，因为：

1. **仿真验证是本阶段的核心交付**：没有仿真后端，硬件未到位时无法验证。
2. **仿真与真实的切换成本必须为零**：用户搭建的积木链在两种模式下完全兼容，才能证明软件层面功能正确。
3. **控制器代码只写一份**：避免为仿真和真实维护两套业务逻辑。

---

## 五、C++ 接口设计

### 5.1 后端抽象接口

```cpp
// controllers/IInstrumentBackend.h
class IInstrumentBackend
{
public:
    virtual ~IInstrumentBackend() = default;
    virtual bool connect(const QString &resourceName) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual void    write(const QString &cmd) = 0;
    virtual QString query(const QString &cmd) = 0;
    virtual QString backendType() const = 0;
};
```

### 5.2 后端工厂（三种切换方式）

```cpp
BackendType InstrumentBackendFactory::fromConfig()
{
    if (QCoreApplication::arguments().contains("--simulate"))
        return BackendType::Simulator;
    if (qgetenv("BLOCKLY_SIM") == "1")
        return BackendType::Simulator;
    if (loadConfig().value("backend").toString() == "simulator")
        return BackendType::Simulator;
    return BackendType::Visa;
}
```

### 5.3 控制器层（以信号源为例）

```cpp
class SignalGeneratorController : public QObject
{
    Q_OBJECT
public:
    explicit SignalGeneratorController(IInstrumentBackend *backend);

    void setSgCW(double freqMHz, double lineLoss, double powerDbm);
    void setSgRfOn();
    void setSgRfOff();

signals:
    void logMessage(const QString &msg);

private:
    IInstrumentBackend *m_backend;
};
```

```cpp
void SignalGeneratorController::setSgCW(double freq, double loss, double power)
{
    m_backend->write("*RST");
    m_backend->write(":OUTP OFF");
    m_backend->write(QString(":SOUR:FREQ %1 MHz").arg(freq));
    m_backend->write(QString(":SOUR:POW %1 dBm").arg(power - loss));
    emit logMessage(QString("[SG] CW 设置: %1 MHz, %2 dBm")
                        .arg(freq).arg(power - loss));
}
```

**控制器代码不区分真实/仿真**——所有差异由后端承担。

### 5.4 VISA 后端

```cpp
bool VisaBackend::connect(const QString &resourceName)
{
    ViSession rm, instr;
    if (viOpenDefaultRM(&rm) < VI_SUCCESS) return false;
    if (viOpen(rm, resourceName.toUtf8().constData(),
               VI_NULL, VI_NULL, &instr) < VI_SUCCESS) {
        viClose(rm);
        return false;
    }
    viSetAttribute(instr, VI_ATTR_TMO_VALUE, 5000);
    m_session = reinterpret_cast<void*>(instr);
    m_connected = true;
    return true;
}
```

用 `#ifdef HAVE_VISA` 保护；`CMakeLists.txt` 中自动查找 VISA 库，找不到时仅编译仿真后端。

### 5.5 仿真后端

**信号源仿真器**：解析 SCPI 命令并维护虚拟状态（频率/功率/RF 开关）。

**频谱仪仿真器**：根据信号源当前状态计算测量值：

```cpp
double SpectrumAnalyzerSimulator::computeMeasuredPower()
{
    if (!m_signalSource || !m_signalSource->isRfOn())
        return m_cfg.noiseFloorDbm + noise();

    double freqOffset = std::abs(m_signalSource->frequencyMHz() - m_state.centerFreqMHz);
    if (freqOffset > m_state.spanMHz / 2.0)
        return m_cfg.noiseFloorDbm + noise();

    double power = m_signalSource->powerDbm() - m_lineLossDb;
    power += -20.0 * std::log10(1.0 + freqOffset);  // 频率滚降
    power += noise();

    m_stablePower = m_stablePower * 0.3 + power * 0.7;  // 惯性滤波
    return m_stablePower;
}
```

**OMT 仿真器**：维护 `QHash<QString,int>` 参数表；增益微调 OID (0A11) 修改时联动影响频谱仪读数，让"调增益 → 读功率"闭环可验证：

```cpp
if (oid == "0A11") {
    int delta = value - oldValue;
    m_spectrumSimulator->adjustGain(delta * 0.5);  // 每单位微调影响 0.5 dB
}
```

### 5.6 仿真场景预设

| 场景 | 行为 |
|------|------|
| 正常 | 读数正确 |
| 信号丢失 | 强制 RF OFF，读数掉到底噪 |
| 频率偏移 | 信号源偏到 span 外，读数掉底 |
| 功率漂移 | 读数缓慢变化 |
| 偶发超时 | 1% 概率返回空响应 |

---

## 六、运行反馈机制

### 6.1 完整运行时序

```
点击运行
    ↓ 清空结果表格/日志/高亮
    ↓ luaGenerator.workspaceToCode(workspace)
    ↓ scriptBridge.submitScript(luaCode) → 返回 taskId
    ↓ 状态栏"正在执行..."
    ↓ 后台线程执行 Lua
    ├── reportStep(blockId) → stepStarted 信号 → 前端高亮积木
    ├── 硬件操作 → 控制器 → 后端 → logMessage → 前端日志
    ├── sendTestResult(...) → testResultReceived → 前端结果表格
    └── 中断检查：用户点停止 → abort() → 下次操作抛 Lua 错误
    ↓ executionFinished(taskId, "OK"/"ERROR"/"ABORTED")
    ↓ 状态栏显示最终结果
    ↓ 清除高亮
```

### 6.2 测试结果表格

`index.html` 右侧新增结果表格，`testResultReceived` 信号追加行：

```javascript
scriptBridge.testResultReceived.connect(function(link, item, cond,
                                                 target, value, unit, status) {
    const row = document.createElement('tr');
    row.className = (status === 'Pass') ? 'pass-row' : 'fail-row';
    row.innerHTML = `
        <td>${link}</td><td>${item}</td><td>${cond}</td>
        <td>${target}</td><td>${value} ${unit}</td>
        <td class="${status.toLowerCase()}">${status}</td>`;
    document.querySelector('#resultTable tbody').appendChild(row);
});
```

### 6.3 步骤高亮

```javascript
scriptBridge.stepStarted.connect(function(blockId) {
    const prev = workspace.getHighlightedBlock();
    if (prev) prev.removeSelect();
    const block = workspace.getBlockById(blockId);
    if (block) { block.addSelect(); block.bringToFront(); }
});
```

### 6.4 中断机制

```cpp
// ScriptEngine
void requestAbort() { m_abortFlag.storeRelease(1); }
void resetAbort()   { m_abortFlag.storeRelease(0); }

// 每个 Lua 注册函数开头
if (engine->m_abortFlag.loadAcquire())
    luaL_error(L, "执行已中断");
```

前端"停止"按钮调用 `scriptBridge.abort()`；`execute()` 捕获 Lua 错误时，若 `isAborted()` 返回 `"ABORTED"`。

### 6.5 ATS 集成抽象

通过 `IResultSink` 抽象分发结果：

```cpp
class IResultSink {
public:
    virtual void report(const QString &link, const QString &item,
                        const QString &cond, const QString &target,
                        const QString &value, const QString &unit,
                        const QString &status) = 0;
};
```

MVP 阶段只实现 `FrontendResultSink`（前端表格），ATS 集成在扩展阶段追加 `AtsResultSink`。

---

## 七、其他机制

### 7.1 工作区持久化

```cpp
Q_INVOKABLE bool    saveWorkspace(const QString &json);   // 文件对话框保存
Q_INVOKABLE QString loadWorkspace();                       // 文件对话框加载
```

前端用 `Blockly.serialization.workspaces.save/load` 序列化/反序列化。

### 7.2 配置管理

`config/instruments.json`：

```json
{
    "backend": "simulator",
    "signalGenerator": { "resourceName": "TCPIP0::192.168.1.100::INSTR" },
    "spectrumAnalyzer": { "resourceName": "TCPIP0::192.168.1.101::INSTR" },
    "device": { "ip": "192.168.1.10", "port": 7 },
    "signalPath": {
        "defaultLineLoss": 2.5,
        "noiseFloorDbm": -100.0,
        "noiseStdDevDb": 0.15
    }
}
```

### 7.3 仿真控制面板

右侧 DockWidget，含信号源区（频率/功率/RF）、频谱仪区（中心频率/参考电平/读数）、场景预设、日志区。

仿真模式下主窗口标题显示 `[仿真模式]`。

### 7.4 Lua 线程安全

Lua 状态机非线程安全。ScriptBridge 通过 `BlockingQueuedConnection` 保证 Lua 执行严格在专用线程内：

```cpp
QtConcurrent::run([this, taskId, luaCode]() {
    QString result;
    QMetaObject::invokeMethod(m_engine, "executeInThread",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QString, result),
                              Q_ARG(QString, luaCode));
    emit executionFinished(taskId, result);
});
```

---

## 八、实施路线（约 15 个工作日）

| 步骤 | 内容 | 工时 |
|------|------|------|
| 1 | `IInstrumentBackend` 接口 + 控制器重构 | 1 天 |
| 2 | `VisaBackend`（NI-VISA 封装） | 1.5 天 |
| 3 | `SimulatorBackend` 基类 + 信号源仿真 | 1.5 天 |
| 4 | 频谱仪仿真（测量模型 + 场景预设） | 1.5 天 |
| 5 | OMT 仿真（OID 表 + 增益联动） | 1 天 |
| 6 | `ScriptEngine` 注册 10 个业务函数 + reportStep/sleep/showMsg + 中断 | 1.5 天 |
| 7 | 10 个积木定义 + 生成器 + 工具箱分类 | 2 天 |
| 8 | 运行反馈（结果表格 + 步骤高亮 + 中断） | 2 天 |
| 9 | 工作区持久化 + 配置管理 | 1 天 |
| 10 | 仿真控制面板 | 1.5 天 |
| 11 | 端到端联调 + "功率定标"场景验证 | 1.5 天 |

**总计约 15 个工作日**（单人开发）。

---

## 九、测试用例

### 9.1 积木与生成器

| 用例 | 操作 | 预期 |
|------|------|------|
| T1 | 拖入"设置信号源 CW"填 1745/-30/2.5 | 生成 `reportStep(...)` + `setSgCW(1745, 2.5, -30)` |
| T2 | 拖入"读取频谱仪功率"接变量 | 生成 `local x = getSaPower()` |
| T3 | "功率定标"完整积木链生成 Lua | 语法正确，可被 Lua 5.4 解析 |

### 9.2 仿真执行

| 用例 | 预期 |
|------|------|
| T4 | 执行"功率定标"，最终 Pass（仿真器增益联动生效） |
| T5 | 场景"信号丢失"，结果 Fail |
| T6 | 场景"频率偏移"，结果 Fail |
| T7 | 场景"功率漂移"，读数缓慢变化 |

### 9.3 运行反馈

| 用例 | 预期 |
|------|------|
| T8 | 执行过程中积木逐块高亮 |
| T9 | 结果表格实时追加 Pass/Fail 行 |
| T10 | 点"停止"，状态栏显示"已中断"，后续操作不执行 |
| T11 | 当前积木在视野外时自动滚动到可见 |

### 9.4 真实/仿真切换

| 用例 | 预期 |
|------|------|
| T12 | `--simulate` 启动，标题显示"仿真模式" |
| T13 | 无 VISA 库编译，仅仿真后端可用，编译通过 |
| T14 | 配置 `"backend":"visa"` + 有 VISA 库，连接真实仪器（如有） |

### 9.5 持久化与配置

| 用例 | 预期 |
|------|------|
| T15 | 保存工作区 → 重启 → 加载，积木布局恢复 |
| T16 | 修改配置文件线损，重新执行结果相应变化 |

---

## 十、目录结构

```
BlocklyPrototype/
├── CMakeLists.txt
├── main.cpp                              # 解析 --simulate
├── MainWindow.h / .cpp                   # 挂载仿真面板
├── ScriptEngine.h / .cpp                 # 注册 10 个业务函数 + reportStep + 中断
├── ScriptBridge.h / .cpp                 # submitScript / abort / 信号
├── ConfigurationManager.h / .cpp
├── controllers/
│   ├── IInstrumentBackend.h
│   ├── IDeviceBackend.h
│   ├── IResultSink.h
│   ├── FrontendResultSink.h / .cpp
│   ├── InstrumentBackendFactory.h / .cpp
│   ├── VisaBackend.h / .cpp
│   ├── SimulatorBackend.h / .cpp
│   ├── SignalGeneratorSimulator.h / .cpp
│   ├── SpectrumAnalyzerSimulator.h / .cpp
│   ├── SimulatorDeviceBackend.h / .cpp
│   ├── SignalGeneratorController.h / .cpp
│   ├── SpectrumAnalyzerController.h / .cpp
│   └── DeviceController.h / .cpp
├── ui/
│   └── SimulationPanel.h / .cpp
├── config/
│   └── instruments.json
├── lua/
├── blockly/
│   ├── index.html                        # 新增日志面板 + 结果表格 + 停止按钮
│   ├── custom-blocks.js                  # 10 个积木
│   ├── custom-generators.js              # 含 reportStep 注入
│   ├── toolbox.js
│   ├── bridge.js
│   ├── blockly-config.js
│   ├── blockly_compressed.js
│   ├── blocks_compressed.js
│   └── lua_compressed.js
└── resources.qrc
```

---

## 十一、后续扩展路径（从 10 到 40）

MVP 阶段验证通过后，按以下批次扩展积木，每批次都**不影响已有积木和控制器接口**：

| 批次 | 内容 | 新增积木 | 说明 |
|------|------|---------|------|
| **批次 1** | 信号源模式/读取 | 4 个 | `sg_set_mode`（调制模式）、`sg_preset`、`sg_get_power`、`sg_set_freq` |
| **批次 2** | 频谱仪完整模板 | 8 个 | LTE 功率模板、ACP 模板、EVM 模板、OBW 等 |
| **批次 3** | 完整 OMT/模调参数 | 10 个 | 扩展 OID 下拉到 30+ 个，增加模调参数积木 |
| **批次 4** | 复合测试流程 | 5 个 | 幅相校准、输出功率定标、输入增益定标等一键封装 |
| **批次 5** | 数据与线损 | 5 个 | 线损表查询、round、formatP5 等 |

**扩展不改动的部分**：
- 后端抽象接口 `IInstrumentBackend` 不变
- 控制器层方法只增不改
- ScriptBridge / ScriptEngine 的注册机制不变
- 前端页面骨架不变（只追加积木定义和生成器）

**每次扩展的增量成本**：
- 每新增 1 个积木：JSON 定义（~20 行）+ 生成器（~5 行）+ C++ Lua 注册函数（~10 行）+ 控制器方法（~5 行）= **约 0.2 天/积木**。
- 从 10 到 40 大约需要 **6-8 个工作日** 的增量开发。

---

## 十二、风险与缓解

| 风险 | 影响 | 缓解 |
|------|------|------|
| 积木数量不足以覆盖所有测试 | 部分场景需临时扩展 | 每批次扩展成本低（0.2 天/积木），按需增量 |
| 仿真器与真实仪器行为偏差大 | 仿真通过真实失败 | 仿真器模拟真实响应延迟/底噪/滚降；真实硬件到位后立刻对照验证 |
| VISA 库版本不兼容 | 编译失败 | `#ifdef HAVE_VISA` 保护，无 VISA 时只编译仿真 |
| Lua 线程与 Qt 主线程冲突 | 崩溃 | 专用线程 + `BlockingQueuedConnection` |
| 高频 reportStep 导致前端卡顿 | UI 卡顿 | 前端节流：50ms 内的多次高亮合并 |
| 中断响应延迟 | 中断慢 | 每个 Lua 注册函数开头检查中断标志，最坏延迟 ≈ 单次仪器操作耗时 |

---

## 附：文档版本

| 版本 | 变更 |
|------|------|
| v1.0 | 第二阶段初版（40+ 积木，依赖 lua 文件对照阅读） |
| v2.0 | 并入完整运行反馈机制（结果上报/步骤高亮/中断/ATS 集成） |
| **v3.0（本版）** | **MVP 精简：积木从 40+ 缩减到 10 个典型积木；逐一列出积木定义（不依赖外部 lua 文件）；新增"功率定标"端到端场景演示；明确从 10 到 40 的增量扩展路径；工时从 31.5 天缩减到 15 天** |