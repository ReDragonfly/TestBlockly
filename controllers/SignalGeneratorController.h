#pragma once

#include <QObject>
#include <QString>

class IInstrumentBackend;

// 信号源控制器：封装信号源业务流程，后端差异（真实/仿真）由 IInstrumentBackend 承担。
class SignalGeneratorController : public QObject
{
    Q_OBJECT
public:
    explicit SignalGeneratorController(IInstrumentBackend *backend, QObject *parent = nullptr);

    // 设置 CW：实际输出功率 = powerDbm - lineLoss
    void setSgCW(double freqMHz, double lineLoss, double powerDbm);
    void setSgRfOn();
    void setSgRfOff();

signals:
    void logMessage(const QString &msg);

private:
    IInstrumentBackend *m_backend = nullptr;
};
