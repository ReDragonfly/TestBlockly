// 工具箱配置
const toolboxDefinition = {
    "contents": [
        {
            "kind": "category",
            "name": "仪器控制",
            "colour": "230",
            "contents": [
                { "kind": "block", "type": "sg_set_power" },
                { "kind": "block", "type": "sa_get_power" }
            ]
        },
        {
            "kind": "category",
            "name": "设备配置",
            "colour": "160",
            "contents": [
                { "kind": "block", "type": "dev_set_omt_param" }
            ]
        },
        {
            "kind": "category",
            "name": "逻辑",
            "colour": "210",
            "contents": [
                { "kind": "block", "type": "controls_if" },
                { "kind": "block", "type": "logic_compare" }
            ]
        },
        {
            "kind": "category",
            "name": "数学",
            "colour": "230",
            "contents": [
                { "kind": "block", "type": "math_number" },
                { "kind": "block", "type": "math_arithmetic" }
            ]
        }
    ]
};
