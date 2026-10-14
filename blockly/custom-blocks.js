// 自定义积木定义（原型阶段 3 个核心积木，用于验证整条链路）
function defineCustomBlocks() {
    Blockly.common.defineBlocksWithJsonArray([
        {
            "type": "sg_set_power",
            "message0": "设置信号源功率 %1 dBm",
            "args0": [
                {
                    "type": "input_value",
                    "name": "POWER",
                    "check": "Number"
                }
            ],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 230,
            "tooltip": "设置信号源的输出功率",
            "helpUrl": ""
        },
        {
            "type": "sa_get_power",
            "message0": "读取频谱仪功率",
            "output": "Number",
            "colour": 230,
            "tooltip": "从频谱仪读取当前功率值",
            "helpUrl": ""
        },
        {
            "type": "dev_set_omt_param",
            "message0": "设置设备参数 OID %1 值 %2",
            "args0": [
                {
                    "type": "field_input",
                    "name": "OID",
                    "text": "06D3"
                },
                {
                    "type": "input_value",
                    "name": "VALUE",
                    "check": "Number"
                }
            ],
            "previousStatement": null,
            "nextStatement": null,
            "colour": 160,
            "tooltip": "设置OMT参数",
            "helpUrl": ""
        }
    ]);
}
