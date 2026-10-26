#pragma once

#include "IInstrumentBackend.h"

#include <QObject>

// 仿真后端基类：实现连接状态管理与日志信号，具体的 SCPI 解析由子类完成。
class SimulatorBackend : public QObject, public IInstrumentBackend
{
    Q_OBJECT
public:
    explicit SimulatorBackend(QObject *parent = nullptr);

    bool connect(const QString &resourceName) override;
    void disconnect() override;
    bool isConnected() const override;
    QString backendType() const override;

signals:
    // 仿真器内部状态变化日志（如 "[SG-SIM] 频率设为 1745 MHz"）
    void logMessage(const QString &msg);

protected:
    bool m_connected = false;
    QString m_resourceName;
};
