// 自定义积木的 Lua 代码生成器
function registerLuaGenerators(generator) {
    // 设置信号源功率
    generator.forBlock['sg_set_power'] = function(block, gen) {
        const power = gen.valueToCode(block, 'POWER', gen.ORDER_ATOMIC) || '0';
        return `setSgPower(${power})\n`;
    };

    // 读取频谱仪功率（值积木，返回表达式字符串）
    generator.forBlock['sa_get_power'] = function(block, gen) {
        return ['getSaPower()', gen.ORDER_FUNCTION_CALL];
    };

    // 设置设备 OMT 参数
    generator.forBlock['dev_set_omt_param'] = function(block, gen) {
        const oid = block.getFieldValue('OID');
        const value = gen.valueToCode(block, 'VALUE', gen.ORDER_ATOMIC) || '0';
        return `setOmtParam("${oid}", ${value})\n`;
    };
}
