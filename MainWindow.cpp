#include "MainWindow.h"

#include "ConfigurationManager.h"
#include "ScriptBridge.h"
#include "ScriptEngine.h"
#include "controllers/DeviceController.h"
#include "controllers/FrontendResultSink.h"
#include "controllers/InstrumentBackendFactory.h"
#include "controllers/SignalGeneratorController.h"
#include "controllers/SignalGeneratorSimulator.h"
#include "controllers/SimulatorDeviceBackend.h"
#include "controllers/SpectrumAnalyzerController.h"
#include "controllers/SpectrumAnalyzerSimulator.h"
#include "controllers/VisaBackend.h"
#include "ui/SimulationPanel.h"

#include <QDebug>
#include <QThread>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1400, 860);

    const bool sim = (InstrumentBackendFactory::fromConfig() == BackendType::Simulator);
    setWindowTitle(sim ? QStringLiteral("Blockly 自动化测试 — 原型验证 [仿真模式]")
                       : QStringLiteral("Blockly 自动化测试 — 原型验证"));

    setupBackends();
    setupWebView();
    setupWebChannel();
    setupSimulationPanel();
}

MainWindow::~MainWindow()
{
    if (m_engineThread) {
        m_engineThread->quit();
        m_engineThread->wait();
    }
    delete m_scriptEngine;   // 无父对象，需手动释放
}

// ---------------------------------------------------------------------------
// 后端装配：按运行模式创建真实（VISA）或仿真后端
// ---------------------------------------------------------------------------
void MainWindow::setupBackends()
{
    ConfigurationManager &cfg = ConfigurationManager::instance();
    const BackendType type = InstrumentBackendFactory::fromConfig();

    if (type == BackendType::Simulator) {
        m_sgSim = new SignalGeneratorSimulator(this);
        m_saSim = new SpectrumAnalyzerSimulator(m_sgSim, this);
        m_saSim->setNoiseFloor(cfg.noiseFloorDbm());
        m_saSim->setNoiseStdDev(cfg.noiseStdDevDb());
        m_sgSim->connect(cfg.signalGeneratorResource());
        m_saSim->connect(cfg.spectrumAnalyzerResource());
        m_sgBackend = m_sgSim;
        m_saBackend = m_saSim;
    } else {
        auto *sgVisa = new VisaBackend(this);
        auto *saVisa = new VisaBackend(this);
        sgVisa->connect(cfg.signalGeneratorResource());
        saVisa->connect(cfg.spectrumAnalyzerResource());
        m_sgBackend = sgVisa;
        m_saBackend = saVisa;
    }

    // MVP 阶段设备后端只实现了仿真实现；真实模式下会给出提示。
    m_devBackend = new SimulatorDeviceBackend(m_saSim, this);
    m_devBackend->connect(cfg.deviceIp(), cfg.devicePort());
    if (type != BackendType::Simulator)
        qDebug() << "[设备] 真实设备后端尚未实现（MVP），当前使用仿真 OID 参数表";

    auto *sgCtrl = new SignalGeneratorController(m_sgBackend, this);
    auto *saCtrl = new SpectrumAnalyzerController(m_saBackend, this);
    auto *devCtrl = new DeviceController(m_devBackend, this);

    m_resultSink = new FrontendResultSink(this);

    // 引擎放入专用线程，保证 Lua 状态机只在单一线程内被访问
    m_scriptEngine = new ScriptEngine(sgCtrl, saCtrl, devCtrl, m_resultSink);
    m_engineThread = new QThread(this);
    m_scriptEngine->moveToThread(m_engineThread);
    m_engineThread->start();

    m_scriptBridge = new ScriptBridge(m_scriptEngine, m_resultSink, this);

    // 控制器与仿真器的日志统一汇聚到桥接对象的 logMessage 信号
    connect(sgCtrl, &SignalGeneratorController::logMessage,
            m_scriptBridge, &ScriptBridge::logMessage);
    connect(saCtrl, &SpectrumAnalyzerController::logMessage,
            m_scriptBridge, &ScriptBridge::logMessage);
    connect(devCtrl, &DeviceController::logMessage,
            m_scriptBridge, &ScriptBridge::logMessage);
    if (m_sgSim)
        connect(m_sgSim, &SimulatorBackend::logMessage,
                m_scriptBridge, &ScriptBridge::logMessage);
    if (m_saSim)
        connect(m_saSim, &SimulatorBackend::logMessage,
                m_scriptBridge, &ScriptBridge::logMessage);
    connect(static_cast<SimulatorDeviceBackend *>(m_devBackend), &SimulatorDeviceBackend::logMessage,
            m_scriptBridge, &ScriptBridge::logMessage);
}

void MainWindow::setupWebView()
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);

    m_webView = new QWebEngineView(this);
    layout->addWidget(m_webView);
    setCentralWidget(central);

    m_webView->settings()->setAttribute(
        QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);

    connect(m_webView, &QWebEngineView::loadFinished, this, [](bool ok) {
        qDebug() << "[MainWindow] 页面加载" << (ok ? "成功" : "失败");
    });

    m_webView->load(QUrl(QStringLiteral("qrc:/blockly/index.html")));
}

void MainWindow::setupWebChannel()
{
    auto *channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("scriptBridge"), m_scriptBridge);

    m_webView->page()->setWebChannel(channel);

    connect(m_scriptEngine, &ScriptEngine::logMessage,
            [](const QString &msg) { qDebug().noquote() << msg; });
}

void MainWindow::setupSimulationPanel()
{
    if (!m_sgSim || !m_saSim)
        return;

    m_simPanel = new SimulationPanel(m_sgSim, m_saSim, this);
    addDockWidget(Qt::RightDockWidgetArea, m_simPanel);

    connect(m_scriptBridge, &ScriptBridge::logMessage,
            m_simPanel, &SimulationPanel::appendLog);
}
