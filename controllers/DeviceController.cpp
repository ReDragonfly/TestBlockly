#include "DeviceController.h"

#include "IDeviceBackend.h"

DeviceController::DeviceController(IDeviceBackend *backend, QObject *parent)
    : QObject(parent)
    , m_backend(backend)
{
}

void DeviceController::setOmtParam(const QString &oid, int value)
{
    if (!m_backend)
        return;
    m_backend->setParam(oid, value);
}

int DeviceController::getOmtParam(const QString &oid)
{
    if (!m_backend)
        return 0;
    return m_backend->getParam(oid);
}
