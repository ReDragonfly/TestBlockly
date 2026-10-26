#pragma once

#include <QDockWidget>

class QLabel;
class QComboBox;
class QPlainTextEdit;
class QTimer;
class SignalGeneratorSimulator;
class SpectrumAnalyzerSimulator;

// 仿真控制面板：显示仿真仪器实时状态、切换场景预设、查看仿真日志。
class SimulationPanel : public QDockWidget
{
    Q_OBJECT
public:
    SimulationPanel(SignalGeneratorSimulator *sg,
                    SpectrumAnalyzerSimulator *sa,
                    QWidget *parent = nullptr);

public slots:
    void appendLog(const QString &msg);

private:
    void refreshState();

    SignalGeneratorSimulator *m_sg = nullptr;
    SpectrumAnalyzerSimulator *m_sa = nullptr;

    QLabel *m_sgFreq = nullptr;
    QLabel *m_sgPower = nullptr;
    QLabel *m_sgRf = nullptr;
    QLabel *m_saCenter = nullptr;
    QLabel *m_saRef = nullptr;
    QLabel *m_saReading = nullptr;

    QComboBox *m_scenario = nullptr;
    QPlainTextEdit *m_log = nullptr;
    QTimer *m_timer = nullptr;
};
