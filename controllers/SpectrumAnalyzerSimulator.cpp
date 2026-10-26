#include "SpectrumAnalyzerSimulator.h"

#include "SignalGeneratorSimulator.h"

#include <QRandomGenerator>
#include <QtMath>

namespace {

bool extractNumber(const QString &cmd, double *out)
{
    const QStringList parts = cmd.split(' ', Qt::SkipEmptyParts);
    for (int i = 1; i < parts.size(); ++i) {
        bool ok = false;
        const double v = parts.at(i).toDouble(&ok);
        if (ok) {
            *out = v;
            return true;
        }
    }
    return false;
}

} // namespace

SpectrumAnalyzerSimulator::SpectrumAnalyzerSimulator(SignalGeneratorSimulator *sg, QObject *parent)
    : SimulatorBackend(parent)
    , m_sg(sg)
{
}

double SpectrumAnalyzerSimulator::noise() const
{
    const double r = QRandomGenerator::global()->generateDouble() * 2.0 - 1.0;
    return r * m_noiseStdDevDb;
}

double SpectrumAnalyzerSimulator::computeMeasuredPower()
{
    // 信号丢失场景：无论信号源状态如何，读数都掉到底噪
    const bool rfOn = (m_scenario != SignalLoss) && m_sg && m_sg->isRfOn();
    if (!rfOn) {
        m_initialized = false;   // 信号恢复后重新起振
        m_stablePower = m_noiseFloorDbm + noise();
        return m_stablePower;
    }

    // 频率偏移场景：信号源频率偏出扫宽
    double freq = m_sg->frequencyMHz();
    if (m_scenario == FreqOffset)
        freq += 50.0;

    const double freqOffset = std::abs(freq - m_centerFreqMHz);
    if (freqOffset > m_spanMHz / 2.0) {
        m_initialized = false;
        m_stablePower = m_noiseFloorDbm + noise();
        return m_stablePower;
    }

    // 实际到达频谱仪的功率 = 信号源实际输出功率 + 设备增益微调 + 频率滚降 + 噪声
    double power = m_sg->powerDbm() + m_gainOffsetDb;
    power += -20.0 * std::log10(1.0 + freqOffset);   // 频率滚降

    if (m_scenario == PowerDrift) {
        m_driftPhase += 0.35;
        power += 1.5 * std::sin(m_driftPhase);       // 读数缓慢变化
    }

    power += noise();

    if (!m_initialized) {
        m_stablePower = power;                        // 首次直接取值，避免惯性起动偏差
        m_initialized = true;
    } else {
        m_stablePower = m_stablePower * 0.3 + power * 0.7;  // 惯性滤波
    }
    return m_stablePower;
}

void SpectrumAnalyzerSimulator::write(const QString &cmd)
{
    const QString c = cmd.trimmed().toUpper();

    if (c.startsWith(":CONF:SAN")) {
        emit logMessage(QStringLiteral("[SA-SIM] 切换到频谱分析模式"));
        return;
    }
    if (c.startsWith(":SENS:FREQ:CENT")) {
        double v = 0;
        if (extractNumber(cmd, &v)) {
            m_centerFreqMHz = v;
            emit logMessage(QStringLiteral("[SA-SIM] 中心频率 %1 MHz").arg(v));
        }
        return;
    }
    if (c.startsWith(":SENS:FREQ:SPAN")) {
        double v = 0;
        if (extractNumber(cmd, &v)) {
            m_spanMHz = v;
            emit logMessage(QStringLiteral("[SA-SIM] 扫宽 %1 MHz").arg(v));
        }
        return;
    }
    if (c.contains("RLEV")) {
        double v = 0;
        if (extractNumber(cmd, &v)) {
            m_refDbm = v;
            emit logMessage(QStringLiteral("[SA-SIM] 参考电平 %1 dBm").arg(v));
        }
        return;
    }
    emit logMessage(QStringLiteral("[SA-SIM] 未识别命令: %1").arg(cmd));
}

QString SpectrumAnalyzerSimulator::query(const QString &cmd)
{
    const QString c = cmd.trimmed().toUpper();
    if (c.startsWith("*OPC"))
        return QStringLiteral("1");

    if (c.contains("MARK1:Y")) {
        // 偶发超时：1% 概率返回空响应
        if (m_scenario == RandomTimeout
            && QRandomGenerator::global()->bounded(10000) == 0) {
            emit logMessage(QStringLiteral("[SA-SIM] 偶发超时（返回空响应）"));
            return QString();
        }
        const double p = computeMeasuredPower();
        emit logMessage(QStringLiteral("[SA-SIM] 功率: %1 dBm").arg(p, 0, 'f', 2));
        return QString::number(p, 'f', 4);
    }
    return QString();
}
