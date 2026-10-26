#pragma once

#include <QString>

enum class BackendType {
    Visa,        // 真实仪器（NI-VISA）
    Simulator    // 软件仿真
};

// 后端类型工厂：按优先级解析运行模式
//   1) 命令行参数 --simulate
//   2) 环境变量 BLOCKLY_SIM=1
//   3) 配置文件 config/instruments.json 的 "backend" 字段
//   4) 默认 Visa
class InstrumentBackendFactory
{
public:
    static BackendType fromConfig();

    static QString typeName(BackendType type);
};
