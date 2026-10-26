#include "SimulatorDeviceBackend.h"

#include "SpectrumAnalyzerSimulator.h"

SimulatorDeviceBackend::SimulatorDeviceBackend(SpectrumAnalyzerSimulator *sa, QObject *parent)
    : QObject(parent)
    , m_sa(sa)
{
    // MVP 阶段预置 5 个最高频 OID，初值均为 0
    const QStringList oids = {
        QStringLiteral("0C10"), QStringLiteral("0C11"), QStringLiteral("0C12"),
        QStringLiteral("0A11"), QStringLiteral("0A12")
    };
    for (const QString &oid : oids)
        m_params.insert(oid, 0);
}

bool SimulatorDeviceBackend::connect(const QString &ip, int port)
{
    Q_UNUSED(ip);
    Q_UNUSED(port);
    m_connected = true;
    emit logMessage(QStringLiteral("[SIM-OMT] 设备仿真后端已连接"));
    return true;
}

void SimulatorDeviceBackend::disconnect()
{
    m_connected = false;
}

bool SimulatorDeviceBackend::isConnected() const
{
    return m_connected;
}

bool SimulatorDeviceBackend::setParam(const QString &oid, int value)
{
    const int oldValue = m_params.value(oid, 0);
    m_params.insert(oid, value);

    // 增益微调联动：每单位微调影响 0.5 dB
    if (oid == QStringLiteral("0A11") && m_sa)
        m_sa->adjustGain((value - oldValue) * 0.5);

    emit logMessage(QStringLiteral("[SIM-OMT] %1: %2 → %3").arg(oid).arg(oldValue).arg(value));
    return true;
}

int SimulatorDeviceBackend::getParam(const QString &oid)
{
    const int value = m_params.value(oid, 0);
    emit logMessage(QStringLiteral("[SIM-OMT] 读取 %1 = %2").arg(oid).arg(value));
    return value;
}

QString SimulatorDeviceBackend::backendType() const
{
    return QStringLiteral("simulator");
}
