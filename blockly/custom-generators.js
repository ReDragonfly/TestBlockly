// 自定义积木的 Lua 代码生成器
// 说明：语句积木在生成业务调用前注入 reportStep("<blockId>")，
// 执行时 C++ 端会回推 stepStarted 信号，前端据此高亮对应积木。
function registerLuaGenerators(generator) {
    // 1. 设置信号源 CW：频率 / 功率 / 线损
    generator.forBlock['sg_set_cw'] = function(block, gen) {
        const freq  = gen.valueToCode(block, 'FREQ',  gen.ORDER_ATOMIC) || '0';
        const power = gen.valueToCode(block, 'POWER', gen.ORDER_ATOMIC) || '0';
        const loss  = gen.valueToCode(block, 'LOSS',  gen.ORDER_ATOMIC) || '0';
        return `reportStep("${block.id}")\n` +
               `setSgCW(${freq}, ${loss}, ${power})\n`;
    };

    // 2. 打开信号源输出
    generator.forBlock['sg_rf_on'] = function(block) {
        return `reportStep("${block.id}")\nsetSgRfOn()\n`;
    };

    // 3. 关闭信号源输出
    generator.forBlock['sg_rf_off'] = function(block) {
        return `reportStep("${block.id}")\nsetSgRfOff()\n`;
    };

    // 4. 设置频谱仪 CW 测量
    generator.forBlock['sa_set_cw'] = function(block, gen) {
        const freq = gen.valueToCode(block, 'FREQ', gen.ORDER_ATOMIC) || '0';
        const ref  = gen.valueToCode(block, 'REF',  gen.ORDER_ATOMIC) || '20';
        return `reportStep("${block.id}")\nsetSaCW(${freq}, 0, ${ref})\n`;
    };

    // 5. 读取频谱仪功率（值积木）
    generator.forBlock['sa_get_power'] = function(block, gen) {
        return ['getSaPower()', gen.ORDER_HIGH];
    };

    // 6. 设置设备参数
    generator.forBlock['omt_set_param'] = function(block, gen) {
        const oid   = block.getFieldValue('OID');
        const value = gen.valueToCode(block, 'VALUE', gen.ORDER_ATOMIC) || '0';
        return `reportStep("${block.id}")\n` +
               `setOmtParam("${oid}", ${value})\n`;
    };

    // 7. 读取设备参数（值积木）
    generator.forBlock['omt_get_param'] = function(block, gen) {
        const oid = block.getFieldValue('OID');
        return [`getOmtParam("${oid}")`, gen.ORDER_HIGH];
    };

    // 8. 上报测试结果
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

    // 9. 等待毫秒
    generator.forBlock['test_wait'] = function(block, gen) {
        const ms = gen.valueToCode(block, 'MS', gen.ORDER_ATOMIC) || '1000';
        return `reportStep("${block.id}")\nsleep(${ms})\n`;
    };

    // 10. 输出日志
    generator.forBlock['test_log'] = function(block, gen) {
        const msg = gen.valueToCode(block, 'MSG', gen.ORDER_ATOMIC) || "''";
        return `showMsg(${msg})\n`;
    };
}
