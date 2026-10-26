#pragma once

#include <QObject>
#include <QString>

class IInstrumentBackend;

// 频谱仪控制器
class SpectrumAnalyzerController : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumAnalyzerController(IInstrumentBackend *backend, QObject *parent = nullptr);

    void setSaCW(double centerFreqMHz, double spanMHz, double refDbm);
    // 读取当前测量功率（dBm），失败返回 NaN
    double getSaPower();

signals:
    void logMessage(const QString &msg);

private:
    IInstrumentBackend *m_backend = nullptr;
};
