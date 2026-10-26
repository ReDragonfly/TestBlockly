#pragma once

#include <QString>

// 仪器后端抽象接口：真实 VISA 仪器（VisaBackend）与软件仿真器（SimulatorBackend 系列）
// 都实现该接口。控制器层只依赖本接口，从而做到"同一套业务代码，真实/仿真两种后端"。
class IInstrumentBackend
{
public:
    virtual ~IInstrumentBackend() = default;

    virtual bool connect(const QString &resourceName) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    // 发送 SCPI 命令（无返回值）
    virtual void write(const QString &cmd) = 0;
    // 发送 SCPI 查询并返回响应
    virtual QString query(const QString &cmd) = 0;

    // 返回后端类型标识："visa" 或 "simulator"
    virtual QString backendType() const = 0;
};
