#pragma once

#include <QObject>
#include <QString>

class IDeviceBackend;

// 设备（OMT）控制器：读写设备 OID 参数
class DeviceController : public QObject
{
    Q_OBJECT
public:
    explicit DeviceController(IDeviceBackend *backend, QObject *parent = nullptr);

    void setOmtParam(const QString &oid, int value);
    int  getOmtParam(const QString &oid);

signals:
    void logMessage(const QString &msg);

private:
    IDeviceBackend *m_backend = nullptr;
};
