#pragma once

#include <QString>

// 被测设备（OMT）后端抽象接口：通过私有协议读写 OID 参数。
// MVP 阶段只实现仿真实现 SimulatorDeviceBackend。
class IDeviceBackend
{
public:
    virtual ~IDeviceBackend() = default;

    virtual bool connect(const QString &ip, int port) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual bool setParam(const QString &oid, int value) = 0;
    virtual int  getParam(const QString &oid) = 0;

    virtual QString backendType() const = 0;
};
