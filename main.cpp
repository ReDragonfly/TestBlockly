#include <QApplication>
#include <QDebug>

#include "ConfigurationManager.h"
#include "MainWindow.h"
#include "controllers/InstrumentBackendFactory.h"

int main(int argc, char *argv[])
{
    // 说明：设计方案中曾建议为 qrc 方案追加 FetchApiAllowed 标志，但该标志自 Qt 6.6
    // 才引入，Qt 6.5.3 的 QWebEngineUrlScheme::Flag 中并不存在。当前页面完全通过
    // <script src="..."> 加载 qrc 资源，未使用 Fetch API，qrc 方案默认即可满足需求，
    // 因此这里不做额外注册。
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("BlocklyPrototype"));

    // 后端模式解析优先级：--simulate > BLOCKLY_SIM=1 > 配置文件 backend 字段 > 默认 visa
    ConfigurationManager::instance().load();
    qDebug().noquote() << "[main] 后端模式:"
                       << InstrumentBackendFactory::typeName(InstrumentBackendFactory::fromConfig());

    MainWindow window;
    window.show();

    return app.exec();
}
