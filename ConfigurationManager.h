#pragma once

#include <QJsonObject>
#include <QString>

// 配置管理：加载 config/instruments.json（打包进 qrc 的 :/config/instruments.json）。
class ConfigurationManager
{
public:
    static ConfigurationManager &instance();

    bool load();

    QString backend() const;
    QString signalGeneratorResource() const;
    QString spectrumAnalyzerResource() const;
    QString deviceIp() const;
    int     devicePort() const;
    double  defaultLineLoss() const;
    double  noiseFloorDbm() const;
    double  noiseStdDevDb() const;

private:
    ConfigurationManager() = default;

    QJsonObject m_root;
    bool m_loaded = false;
};
