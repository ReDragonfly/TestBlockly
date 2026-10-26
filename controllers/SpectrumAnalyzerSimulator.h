#pragma once

#include "SimulatorBackend.h"

class SignalGeneratorSimulator;

// 频谱仪仿真器：根据信号源当前状态 + 设备增益联动计算测量值。
// 支持的命令：
//   :CONF:SAN                        切换到频谱分析模式
//   :SENS:FREQ:CENT <v> MHz          设置中心频率
//   :DISP:WIND:TRAC:Y:RLEV <v> dBm   设置参考电平
//   :SENS:FREQ:SPAN <v> MHz          设置扫宽
// 查询：:CALC:MARK1:Y? / *OPC?
class SpectrumAnalyzerSimulator : public SimulatorBackend
{
    Q_OBJECT
public:
    // 仿真场景预设
    enum Scenario {
        Normal = 0,      // 正常
        SignalLoss,      // 信号丢失：强制 RF OFF，读数掉到底噪
        FreqOffset,      // 频率偏移：信号偏出 span，读数掉底
        PowerDrift,      // 功率漂移：读数缓慢变化
        RandomTimeout    // 偶发超时：1% 概率返回空响应
    };

    explicit SpectrumAnalyzerSimulator(SignalGeneratorSimulator *sg,
                                       QObject *parent = nullptr);

    void write(const QString &cmd) override;
    QString query(const QString &cmd) override;

    void setScenario(Scenario s) { m_scenario = s; }
    Scenario scenario() const { return m_scenario; }

    // 设备增益微调联动：由 SimulatorDeviceBackend 调用
    void adjustGain(double db) { m_gainOffsetDb += db; }

    double centerFreqMHz() const { return m_centerFreqMHz; }
    double refDbm() const { return m_refDbm; }
    double lastReadingDbm() const { return m_stablePower; }

    void setNoiseFloor(double floorDbm) { m_noiseFloorDbm = floorDbm; }
    void setNoiseStdDev(double stdDevDb) { m_noiseStdDevDb = stdDevDb; }

private:
    double computeMeasuredPower();
    double noise() const;

    SignalGeneratorSimulator *m_sg = nullptr;
    Scenario m_scenario = Normal;

    double m_centerFreqMHz = 1000.0;
    double m_spanMHz = 10.0;
    double m_refDbm = 20.0;
    double m_gainOffsetDb = 0.0;     // 设备增益微调造成的功率偏移
    double m_stablePower = -100.0;   // 惯性滤波后的稳定读数
    bool   m_initialized = false;
    double m_driftPhase = 0.0;

    double m_noiseFloorDbm = -100.0;
    double m_noiseStdDevDb = 0.15;
};
