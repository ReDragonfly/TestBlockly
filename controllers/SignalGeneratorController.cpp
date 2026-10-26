#include "SignalGeneratorController.h"

#include "IInstrumentBackend.h"

SignalGeneratorController::SignalGeneratorController(IInstrumentBackend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
{
}

void SignalGeneratorController::setSgCW(double freqMHz, double lineLoss, double powerDbm)
{
    if (!m_backend)
        return;

    const double actualPower = powerDbm - lineLoss;
    m_backend->write(QStringLiteral("*RST"));
    m_backend->write(QStringLiteral(":OUTP OFF"));
    m_backend->write(QStringLiteral(":SOUR:FREQ %1 MHz").arg(freqMHz));
    m_backend->write(QStringLiteral(":SOUR:POW %1 dBm").arg(actualPower));

    emit logMessage(QStringLiteral("[SG] CW 设置: %1 MHz, %2 dBm")
                        .arg(freqMHz).arg(actualPower));
}

void SignalGeneratorController::setSgRfOn()
{
    if (!m_backend)
        return;
    m_backend->write(QStringLiteral(":OUTP ON"));
    emit logMessage(QStringLiteral("[SG] 打开射频输出"));
}

void SignalGeneratorController::setSgRfOff()
{
    if (!m_backend)
        return;
    m_backend->write(QStringLiteral(":OUTP OFF"));
    emit logMessage(QStringLiteral("[SG] 关闭射频输出"));
}
