#include "VisaBackend.h"

#ifdef HAVE_VISA
#include <visa.h>
#endif

VisaBackend::VisaBackend(QObject *parent)
    : QObject(parent)
{
}

VisaBackend::~VisaBackend()
{
    disconnect();
}

bool VisaBackend::connect(const QString &resourceName)
{
#ifdef HAVE_VISA
    ViSession rm = VI_NULL;
    ViSession instr = VI_NULL;
    if (viOpenDefaultRM(&rm) < VI_SUCCESS) {
        emit logMessage(QStringLiteral("[VISA] 打开默认资源管理器失败"));
        return false;
    }
    const QByteArray name = resourceName.toUtf8();
    if (viOpen(rm, name.constData(), VI_NULL, VI_NULL, &instr) < VI_SUCCESS) {
        emit logMessage(QStringLiteral("[VISA] 打开仪器失败: %1").arg(resourceName));
        viClose(rm);
        return false;
    }
    viSetAttribute(instr, VI_ATTR_TMO_VALUE, 5000);
    m_rm = reinterpret_cast<void *>(static_cast<quintptr>(rm));
    m_session = reinterpret_cast<void *>(static_cast<quintptr>(instr));
    m_connected = true;
    emit logMessage(QStringLiteral("[VISA] 已连接: %1").arg(resourceName));
    return true;
#else
    Q_UNUSED(resourceName);
    emit logMessage(QStringLiteral("[VISA] 未编译 VISA 支持（HAVE_VISA 未定义），无法连接真实仪器"));
    return false;
#endif
}

void VisaBackend::disconnect()
{
#ifdef HAVE_VISA
    if (m_session) {
        viClose(reinterpret_cast<ViSession>(reinterpret_cast<quintptr>(m_session)));
        m_session = nullptr;
    }
    if (m_rm) {
        viClose(reinterpret_cast<ViSession>(reinterpret_cast<quintptr>(m_rm)));
        m_rm = nullptr;
    }
#endif
    m_connected = false;
}

bool VisaBackend::isConnected() const
{
    return m_connected;
}

void VisaBackend::write(const QString &cmd)
{
#ifdef HAVE_VISA
    if (!m_connected)
        return;
    const QByteArray buf = cmd.toUtf8();
    ViUInt32 written = 0;
    viWrite(reinterpret_cast<ViSession>(reinterpret_cast<quintptr>(m_session)),
            reinterpret_cast<ViBuf>(const_cast<char *>(buf.constData())),
            static_cast<ViUInt32>(buf.size()), &written);
#else
    Q_UNUSED(cmd);
#endif
}

QString VisaBackend::query(const QString &cmd)
{
#ifdef HAVE_VISA
    if (!m_connected)
        return QString();
    write(cmd);
    char buf[512] = {0};
    ViUInt32 read = 0;
    viRead(reinterpret_cast<ViSession>(reinterpret_cast<quintptr>(m_session)),
           reinterpret_cast<ViBuf>(buf), sizeof(buf) - 1, &read);
    return QString::fromUtf8(buf, static_cast<int>(read)).trimmed();
#else
    Q_UNUSED(cmd);
    return QString();
#endif
}

QString VisaBackend::backendType() const
{
    return QStringLiteral("visa");
}
