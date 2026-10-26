#pragma once

#include "IInstrumentBackend.h"

#include <QObject>

// 真实仪器后端：NI-VISA 封装。仅当工程找到 VISA 库（HAVE_VISA）时才会真正连接，
// 否则所有操作返回失败/空响应，编译仍可通过（见设计方案 5.4 / 测试用例 T13）。
class VisaBackend : public QObject, public IInstrumentBackend
{
    Q_OBJECT
public:
    explicit VisaBackend(QObject *parent = nullptr);
    ~VisaBackend() override;

    bool connect(const QString &resourceName) override;
    void disconnect() override;
    bool isConnected() const override;

    void write(const QString &cmd) override;
    QString query(const QString &cmd) override;

    QString backendType() const override;

signals:
    void logMessage(const QString &msg);

private:
    void *m_rm = nullptr;        // ViSession（默认资源管理器）
    void *m_session = nullptr;   // ViSession（仪器会话）
    bool m_connected = false;
};
