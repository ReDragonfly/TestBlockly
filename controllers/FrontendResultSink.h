#pragma once

#include "IResultSink.h"

#include <QObject>

// 前端结果表格分发器：把测试结果以信号形式转发给 ScriptBridge，
// 再由 ScriptBridge 通过 QWebChannel 推送到前端 JS。
class FrontendResultSink : public QObject, public IResultSink
{
    Q_OBJECT
public:
    explicit FrontendResultSink(QObject *parent = nullptr);

    void report(const QString &link, const QString &item, const QString &cond,
                const QString &target, const QString &value, const QString &unit,
                const QString &status) override;

signals:
    void resultReported(const QString &link, const QString &item, const QString &cond,
                        const QString &target, const QString &value, const QString &unit,
                        const QString &status);
};
