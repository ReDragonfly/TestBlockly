#include "SimulatorBackend.h"

SimulatorBackend::SimulatorBackend(QObject *parent)
    : QObject(parent)
{
}

bool SimulatorBackend::connect(const QString &resourceName)
{
    m_resourceName = resourceName;
    m_connected = true;
    emit logMessage(QStringLiteral("[SIM] 仿真后端已连接（resource: %1）")
                        .arg(resourceName.isEmpty() ? QStringLiteral("<默认>") : resourceName));
    return true;
}

void SimulatorBackend::disconnect()
{
    m_connected = false;
}

bool SimulatorBackend::isConnected() const
{
    return m_connected;
}

QString SimulatorBackend::backendType() const
{
    return QStringLiteral("simulator");
}
