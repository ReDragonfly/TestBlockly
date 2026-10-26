#include "ConfigurationManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>

ConfigurationManager &ConfigurationManager::instance()
{
    static ConfigurationManager mgr;
    if (!mgr.m_loaded)
        mgr.load();   // 首次访问时自动加载配置
    return mgr;
}

bool ConfigurationManager::load()
{
    if (m_loaded)
        return true;

    QFile file(QStringLiteral(":/config/instruments.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        m_loaded = true;   // 无配置文件时使用默认值
        return false;
    }

    QJsonParseError err {};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error == QJsonParseError::NoError && doc.isObject())
        m_root = doc.object();

    m_loaded = true;
    return true;
}

QString ConfigurationManager::backend() const
{
    return m_root.value(QStringLiteral("backend")).toString(QStringLiteral("visa"));
}

QString ConfigurationManager::signalGeneratorResource() const
{
    return m_root.value(QStringLiteral("signalGenerator")).toObject()
                 .value(QStringLiteral("resourceName")).toString();
}

QString ConfigurationManager::spectrumAnalyzerResource() const
{
    return m_root.value(QStringLiteral("spectrumAnalyzer")).toObject()
                 .value(QStringLiteral("resourceName")).toString();
}

QString ConfigurationManager::deviceIp() const
{
    return m_root.value(QStringLiteral("device")).toObject()
                 .value(QStringLiteral("ip")).toString();
}

int ConfigurationManager::devicePort() const
{
    return m_root.value(QStringLiteral("device")).toObject()
                 .value(QStringLiteral("port")).toInt(7);
}

double ConfigurationManager::defaultLineLoss() const
{
    return m_root.value(QStringLiteral("signalPath")).toObject()
                 .value(QStringLiteral("defaultLineLoss")).toDouble(2.5);
}

double ConfigurationManager::noiseFloorDbm() const
{
    return m_root.value(QStringLiteral("signalPath")).toObject()
                 .value(QStringLiteral("noiseFloorDbm")).toDouble(-100.0);
}

double ConfigurationManager::noiseStdDevDb() const
{
    return m_root.value(QStringLiteral("signalPath")).toObject()
                 .value(QStringLiteral("noiseStdDevDb")).toDouble(0.15);
}
