#include "MainWindow.h"

#include "ScriptBridge.h"
#include "ScriptEngine.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1280, 800);
    setWindowTitle(QStringLiteral("Blockly 自动化测试 — 原型验证"));

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);

    m_webView = new QWebEngineView(this);
    layout->addWidget(m_webView);
    setCentralWidget(central);

    setupWebChannel();
    setupWebView();
}

void MainWindow::setupWebView()
{
    // 启用必要的 WebEngine 设置
    m_webView->settings()->setAttribute(
        QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);

    // 连接加载完成信号，用于调试
    connect(m_webView, &QWebEngineView::loadFinished, this, [](bool ok) {
        qDebug() << "[MainWindow] 页面加载" << (ok ? "成功" : "失败");
    });

    // 加载本地 Blockly 编辑器
    m_webView->load(QUrl(QStringLiteral("qrc:/blockly/index.html")));
}

void MainWindow::setupWebChannel()
{
    m_scriptEngine = new ScriptEngine(this);
    m_scriptBridge = new ScriptBridge(m_scriptEngine, this);

    auto *channel = new QWebChannel(this);
    // 注册对象名为 "scriptBridge"，前端通过 channel.objects.scriptBridge 访问
    channel->registerObject(QStringLiteral("scriptBridge"), m_scriptBridge);

    // 将 QWebChannel 绑定到 WebEngine 页面
    m_webView->page()->setWebChannel(channel);

    // 引擎日志输出到 C++ 控制台
    connect(m_scriptEngine, &ScriptEngine::logMessage,
            [](const QString &msg) { qDebug().noquote() << msg; });
}
