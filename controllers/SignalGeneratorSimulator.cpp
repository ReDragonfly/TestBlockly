#include "SignalGeneratorSimulator.h"

namespace {

// 取命令的首个数值参数："… <v> MHz" / "… <v> dBm"
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

SignalGeneratorSimulator::SignalGeneratorSimulator(QObject *parent)
    : SimulatorBackend(parent)
{
}

void SignalGeneratorSimulator::write(const QString &cmd)
{
    const QString c = cmd.trimmed().toUpper();

    if (c.startsWith("*RST")) {
        m_freqMHz = 1000.0;
        m_powerDbm = -30.0;
        m_rfOn = false;
        emit logMessage(QStringLiteral("[SG-SIM] *RST 复位"));
        return;
    }
    if (c.startsWith(":OUTP")) {
        m_rfOn = c.contains("ON") && !c.contains("OFF");
        emit logMessage(QStringLiteral("[SG-SIM] RF %1").arg(m_rfOn ? "ON" : "OFF"));
        return;
    }
    if (c.startsWith(":SOUR:FREQ")) {
        double v = 0;
        if (extractNumber(cmd, &v)) {
            m_freqMHz = v;
            emit logMessage(QStringLiteral("[SG-SIM] 频率设为 %1 MHz").arg(v));
        }
        return;
    }
    if (c.startsWith(":SOUR:POW")) {
        double v = 0;
        if (extractNumber(cmd, &v)) {
            m_powerDbm = v;
            emit logMessage(QStringLiteral("[SG-SIM] 功率设为 %1 dBm").arg(v));
        }
        return;
    }
    emit logMessage(QStringLiteral("[SG-SIM] 未识别命令: %1").arg(cmd));
}

QString SignalGeneratorSimulator::query(const QString &cmd)
{
    const QString c = cmd.trimmed().toUpper();
    if (c.startsWith("*OPC"))
        return QStringLiteral("1");
    if (c.startsWith(":SOUR:FREQ"))
        return QString::number(m_freqMHz);
    if (c.startsWith(":SOUR:POW"))
        return QString::number(m_powerDbm);
    if (c.startsWith(":OUTP"))
        return m_rfOn ? QStringLiteral("1") : QStringLiteral("0");
    return QString();
}
