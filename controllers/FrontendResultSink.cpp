#include "FrontendResultSink.h"

FrontendResultSink::FrontendResultSink(QObject *parent)
    : QObject(parent)
{
}

void FrontendResultSink::report(const QString &link, const QString &item, const QString &cond,
                                const QString &target, const QString &value, const QString &unit,
                                const QString &status)
{
    emit resultReported(link, item, cond, target, value, unit, status);
}
