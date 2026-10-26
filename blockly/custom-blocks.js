// 自定义积木定义（第二阶段 MVP：10 个典型业务积木）
function defineCustomBlocks() {
    Blockly.common.defineBlocksWithJsonArray([
        // ---------------- 信号源 ----------------
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
            "tooltip": "设置信号源为CW模式，实际输出功率 = 设定功率 - 线损",
            "helpUrl": ""
        },
        {
            "type": "sg_rf_on",
            "message0": "打开信号源输出",
            "previousStatement": null,
            "nextStatement": null,
            "colour": 230,
            "tooltip": "打开信号源射频输出（:OUTP ON）",
            "helpUrl": ""
        },
        {
            "type": "sg_rf_off",
            "message0": "关闭信号源输出",
            "previousStatement": null,
            "nextStatement": null,
            "colour": 230,
            "tooltip": "关闭信号源射频输出（:OUTP OFF）",
            "helpUrl": ""
        },

        // ---------------- 频谱仪 ----------------
        {
            "type": "sa_set_cw",
            "message0": "设置频谱仪 CW 测量",
            "message1": "中心频率 %1 MHz",
            "args1": [{ "type": "input_value", "name": "FREQ", "check": "Number" }],
            "message2": "参考电平 %1 dBm",
            "args2": [{ "type": "input_value", "name": "REF", "check": "Number" }],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 230,
            "tooltip": "配置频谱仪以CW模式测量指定中心频率的功率",
            "helpUrl": ""
        },
        {
            "type": "sa_get_power",
            "message0": "读取频谱仪功率",
            "output": "Number",
            "colour": 230,
            "tooltip": "返回当前测量的功率值（dBm）",
            "helpUrl": ""
        },

        // ---------------- 设备参数 ----------------
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
            "colour": 160,
            "tooltip": "向设备写入一个 OID 参数值",
            "helpUrl": ""
        },
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
            "colour": 160,
            "tooltip": "从设备读取一个 OID 参数值",
            "helpUrl": ""
        },

        // ---------------- 测试反馈 ----------------
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
                "options": [["Pass", "Pass"], ["Fail", "Fail"]]
            }],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 210,
            "tooltip": "向结果表格输出一条 Pass/Fail 测试记录",
            "helpUrl": ""
        },
        {
            "type": "test_wait",
            "message0": "等待 %1 毫秒",
            "args0": [{ "type": "input_value", "name": "MS", "check": "Number" }],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 210,
            "tooltip": "延时指定毫秒数（等待设备状态稳定）",
            "helpUrl": ""
        },
        {
            "type": "test_log",
            "message0": "输出日志 %1",
            "args0": [{ "type": "input_value", "name": "MSG", "check": "String" }],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 210,
            "tooltip": "在日志面板打印消息",
            "helpUrl": ""
        }
    ]);
}
