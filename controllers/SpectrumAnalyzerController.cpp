#include "SpectrumAnalyzerController.h"

#include "IInstrumentBackend.h"

#include <QtGlobal>
#include <QtMath>

SpectrumAnalyzerController::SpectrumAnalyzerController(IInstrumentBackend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
{
}

void SpectrumAnalyzerController::setSaCW(double centerFreqMHz, double spanMHz, double refDbm)
{
    if (!m_backend)
        return;

    m_backend->write(QStringLiteral(":CONF:SAN"));
    m_backend->write(QStringLiteral(":DISP:WIND:TRAC:Y:RLEV %1 dBm").arg(refDbm));
    m_backend->write(QStringLiteral(":SENS:FREQ:CENT %1 MHz").arg(centerFreqMHz));
    m_backend->write(QStringLiteral(":SENS:FREQ:SPAN %1 MHz").arg(spanMHz));

    emit logMessage(QStringLiteral("[SA] CW 测量设置: 中心 %1 MHz, 参考 %2 dBm")
                        .arg(centerFreqMHz).arg(refDbm));
}

double SpectrumAnalyzerController::getSaPower()
{
    if (!m_backend)
        return qQNaN();

    m_backend->query(QStringLiteral("*OPC?"));
    const QString resp = m_backend->query(QStringLiteral(":CALC:MARK1:Y?")).trimmed();
    if (resp.isEmpty())
        return qQNaN();

    bool ok = false;
    const double power = resp.toDouble(&ok);
    return ok ? power : qQNaN();
}
