#pragma once

#include "IDeviceBackend.h"

#include <QHash>
#include <QObject>

class SpectrumAnalyzerSimulator;

// OMT 设备仿真后端：维护 OID 参数表；对增益微调 OID（0A11）做联动，
// 让"调增益 → 读功率"的闭环在仿真下可验证。
class SimulatorDeviceBackend : public QObject, public IDeviceBackend
{
    Q_OBJECT
public:
    explicit SimulatorDeviceBackend(SpectrumAnalyzerSimulator *sa,
                                    QObject *parent = nullptr);

    bool connect(const QString &ip, int port) override;
    void disconnect() override;
    bool isConnected() const override;

    bool setParam(const QString &oid, int value) override;
    int  getParam(const QString &oid) override;

    QString backendType() const override;

signals:
    void logMessage(const QString &msg);

private:
    QHash<QString, int> m_params;
    SpectrumAnalyzerSimulator *m_sa = nullptr;
    bool m_connected = false;
};
