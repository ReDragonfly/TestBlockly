#include "InstrumentBackendFactory.h"

#include "ConfigurationManager.h"

#include <QCoreApplication>
#include <QProcessEnvironment>

BackendType InstrumentBackendFactory::fromConfig()
{
    if (QCoreApplication::arguments().contains(QStringLiteral("--simulate")))
        return BackendType::Simulator;

    if (QProcessEnvironment::systemEnvironment().value(QStringLiteral("BLOCKLY_SIM")) == QLatin1String("1"))
        return BackendType::Simulator;

    if (ConfigurationManager::instance().backend() == QLatin1String("simulator"))
        return BackendType::Simulator;

    return BackendType::Visa;
}

QString InstrumentBackendFactory::typeName(BackendType type)
{
    return type == BackendType::Simulator ? QStringLiteral("simulator")
                                          : QStringLiteral("visa");
}
