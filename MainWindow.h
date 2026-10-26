#pragma once

#include <QMainWindow>

class QWebEngineView;
class QThread;
class ScriptEngine;
class ScriptBridge;
class FrontendResultSink;
class SignalGeneratorSimulator;
class SpectrumAnalyzerSimulator;
class SimulationPanel;
class IInstrumentBackend;
class IDeviceBackend;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupBackends();
    void setupWebView();
    void setupWebChannel();
    void setupSimulationPanel();

    QWebEngineView *m_webView = nullptr;

    QThread *m_engineThread = nullptr;
    ScriptEngine *m_scriptEngine = nullptr;     // 无父对象，移入引擎线程后手动释放
    ScriptBridge *m_scriptBridge = nullptr;
    FrontendResultSink *m_resultSink = nullptr;

    IInstrumentBackend *m_sgBackend = nullptr;
    IInstrumentBackend *m_saBackend = nullptr;
    IDeviceBackend *m_devBackend = nullptr;

    SignalGeneratorSimulator *m_sgSim = nullptr;   // 仿真模式下非空
    SpectrumAnalyzerSimulator *m_saSim = nullptr;
    SimulationPanel *m_simPanel = nullptr;
};
