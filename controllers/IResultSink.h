#pragma once

#include <QString>

// 测试结果分发抽象：MVP 阶段由 FrontendResultSink 推送到前端结果表格，
// 扩展阶段可追加 AtsResultSink 上报到 ATS 系统。
class IResultSink
{
public:
    virtual ~IResultSink() = default;

    virtual void report(const QString &link, const QString &item, const QString &cond,
                        const QString &target, const QString &value, const QString &unit,
                        const QString &status) = 0;
};
