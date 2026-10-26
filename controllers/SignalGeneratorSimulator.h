#pragma once

#include "SimulatorBackend.h"

// 信号源仿真器：解析 SCPI 命令并维护虚拟状态（频率 / 功率 / RF 开关）。
// 支持的命令：
//   *RST                     复位（RF OFF）
//   :OUTP ON | :OUTP OFF     射频开关
//   :SOUR:FREQ <v> MHz       设置频率
//   :SOUR:POW <v> dBm        设置输出功率
// 查询：:SOUR:FREQ? / :SOUR:POW? / :OUTP? / *OPC?
class SignalGeneratorSimulator : public SimulatorBackend
{
    Q_OBJECT
public:
    explicit SignalGeneratorSimulator(QObject *parent = nullptr);

    void write(const QString &cmd) override;
    QString query(const QString &cmd) override;

    double frequencyMHz() const { return m_freqMHz; }
    double powerDbm() const { return m_powerDbm; }
    bool   isRfOn() const { return m_rfOn; }

private:
    double m_freqMHz = 1000.0;
    double m_powerDbm = -30.0;
    bool   m_rfOn = false;
};
