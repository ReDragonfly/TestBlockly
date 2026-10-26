// 工具箱配置（第二阶段 MVP：10 个自定义积木 + Blockly 内置积木）
const toolboxDefinition = {
    "contents": [
        {
            "kind": "category",
            "name": "信号源控制",
            "colour": "230",
            "contents": [
                {
                    "kind": "block",
                    "type": "sg_set_cw",
                    "inputs": {
                        "FREQ":  { "shadow": { "type": "math_number", "fields": { "NUM": 1745 } } },
                        "POWER": { "shadow": { "type": "math_number", "fields": { "NUM": -30 } } },
                        "LOSS":  { "shadow": { "type": "math_number", "fields": { "NUM": 2.5 } } }
                    }
                },
                { "kind": "block", "type": "sg_rf_on" },
                { "kind": "block", "type": "sg_rf_off" }
            ]
        },
        {
            "kind": "category",
            "name": "频谱仪控制",
            "colour": "230",
            "contents": [
                {
                    "kind": "block",
                    "type": "sa_set_cw",
                    "inputs": {
                        "FREQ": { "shadow": { "type": "math_number", "fields": { "NUM": 1745 } } },
                        "REF":  { "shadow": { "type": "math_number", "fields": { "NUM": 20 } } }
                    }
                },
                { "kind": "block", "type": "sa_get_power" }
            ]
        },
        {
            "kind": "category",
            "name": "设备参数",
            "colour": "160",
            "contents": [
                {
                    "kind": "block",
                    "type": "omt_set_param",
                    "inputs": { "VALUE": { "shadow": { "type": "math_number", "fields": { "NUM": 0 } } } }
                },
                { "kind": "block", "type": "omt_get_param" }
            ]
        },
        {
            "kind": "category",
            "name": "测试反馈",
            "colour": "210",
            "contents": [
                {
                    "kind": "block",
                    "type": "test_send_result",
                    "inputs": {
                        "COND":   { "shadow": { "type": "text", "fields": { "TEXT": "1745MHz -30dBm" } } },
                        "TARGET": { "shadow": { "type": "text", "fields": { "TEXT": "6±0.5" } } },
                        "VALUE":  { "shadow": { "type": "math_number", "fields": { "NUM": 0 } } }
                    }
                },
                {
                    "kind": "block",
                    "type": "test_wait",
                    "inputs": { "MS": { "shadow": { "type": "math_number", "fields": { "NUM": 500 } } } }
                },
                {
                    "kind": "block",
                    "type": "test_log",
                    "inputs": { "MSG": { "shadow": { "type": "text", "fields": { "TEXT": "开始功率定标" } } } }
                }
            ]
        },
        {
            "kind": "category",
            "name": "逻辑",
            "colour": "210",
            "contents": [
                { "kind": "block", "type": "controls_if" },
                { "kind": "block", "type": "logic_compare" },
                { "kind": "block", "type": "logic_operation" },
                { "kind": "block", "type": "logic_negate" },
                { "kind": "block", "type": "logic_boolean" }
            ]
        },
        {
            "kind": "category",
            "name": "循环",
            "colour": "120",
            "contents": [
                { "kind": "block", "type": "controls_repeat_ext" },
                {
                    "kind": "block",
                    "type": "controls_for",
                    "inputs": { "FROM": { "shadow": { "type": "math_number", "fields": { "NUM": 1 } } },
                                "TO":   { "shadow": { "type": "math_number", "fields": { "NUM": 20 } } },
                                "BY":   { "shadow": { "type": "math_number", "fields": { "NUM": 1 } } } }
                },
                { "kind": "block", "type": "controls_whileUntil" }
            ]
        },
        {
            "kind": "category",
            "name": "变量",
            "colour": "330",
            "custom": "VARIABLE"
        },
        {
            "kind": "category",
            "name": "数学",
            "colour": "230",
            "contents": [
                { "kind": "block", "type": "math_number" },
                { "kind": "block", "type": "math_arithmetic" },
                { "kind": "block", "type": "math_single" },
                { "kind": "block", "type": "math_round" }
            ]
        },
        {
            "kind": "category",
            "name": "文本",
            "colour": "160",
            "contents": [
                { "kind": "block", "type": "text" },
                { "kind": "block", "type": "text_join" }
            ]
        }
    ]
};
