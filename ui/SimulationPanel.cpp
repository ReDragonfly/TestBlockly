#include "SimulationPanel.h"

#include "controllers/SignalGeneratorSimulator.h"
#include "controllers/SpectrumAnalyzerSimulator.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

SimulationPanel::SimulationPanel(SignalGeneratorSimulator *sg,
                                 SpectrumAnalyzerSimulator *sa,
                                 QWidget *parent)
    : QDockWidget(QStringLiteral("仿真控制面板"), parent)
    , m_sg(sg)
    , m_sa(sa)
{
    setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    auto *root = new QWidget(this);
    auto *layout = new QVBoxLayout(root);

    // ---------------- 信号源 ----------------
    auto *sgTitle = new QLabel(QStringLiteral("<b>信号源</b>"), root);
    layout->addWidget(sgTitle);

    auto *sgForm = new QFormLayout;
    m_sgFreq = new QLabel(root);
    m_sgPower = new QLabel(root);
    m_sgRf = new QLabel(root);
    sgForm->addRow(QStringLiteral("频率"), m_sgFreq);
    sgForm->addRow(QStringLiteral("功率"), m_sgPower);
    sgForm->addRow(QStringLiteral("射频"), m_sgRf);
    layout->addLayout(sgForm);

    // ---------------- 频谱仪 ----------------
    layout->addWidget(new QLabel(QStringLiteral("<b>频谱仪</b>"), root));
    auto *saForm = new QFormLayout;
    m_saCenter = new QLabel(root);
    m_saRef = new QLabel(root);
    m_saReading = new QLabel(root);
    saForm->addRow(QStringLiteral("中心频率"), m_saCenter);
    saForm->addRow(QStringLiteral("参考电平"), m_saRef);
    saForm->addRow(QStringLiteral("当前读数"), m_saReading);
    layout->addLayout(saForm);

    // ---------------- 场景预设 ----------------
    layout->addWidget(new QLabel(QStringLiteral("<b>场景预设</b>"), root));
    m_scenario = new QComboBox(root);
    m_scenario->addItem(QStringLiteral("正常"));
    m_scenario->addItem(QStringLiteral("信号丢失"));
    m_scenario->addItem(QStringLiteral("频率偏移"));
    m_scenario->addItem(QStringLiteral("功率漂移"));
    m_scenario->addItem(QStringLiteral("偶发超时"));
    layout->addWidget(m_scenario);

    // ---------------- 日志 ----------------
    layout->addWidget(new QLabel(QStringLiteral("<b>仿真日志</b>"), root));
    m_log = new QPlainTextEdit(root);
    m_log->setReadOnly(true);
    m_log->setMaximumBlockCount(500);
    layout->addWidget(m_log);

    setWidget(root);

    if (m_sa) {
        connect(m_scenario, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            m_sa->setScenario(static_cast<SpectrumAnalyzerSimulator::Scenario>(idx));
            appendLog(QStringLiteral("[仿真] 场景切换为: %1").arg(m_scenario->currentText()));
        });
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(300);
    connect(m_timer, &QTimer::timeout, this, &SimulationPanel::refreshState);
    m_timer->start();

    refreshState();
}

void SimulationPanel::appendLog(const QString &msg)
{
    m_log->appendPlainText(msg);
}

void SimulationPanel::refreshState()
{
    if (m_sg) {
        m_sgFreq->setText(QStringLiteral("%1 MHz").arg(m_sg->frequencyMHz(), 0, 'f', 3));
        m_sgPower->setText(QStringLiteral("%1 dBm").arg(m_sg->powerDbm(), 0, 'f', 2));
        m_sgRf->setText(m_sg->isRfOn() ? QStringLiteral("ON") : QStringLiteral("OFF"));
    }
    if (m_sa) {
        m_saCenter->setText(QStringLiteral("%1 MHz").arg(m_sa->centerFreqMHz(), 0, 'f', 3));
        m_saRef->setText(QStringLiteral("%1 dBm").arg(m_sa->refDbm(), 0, 'f', 1));
        m_saReading->setText(QStringLiteral("%1 dBm").arg(m_sa->lastReadingDbm(), 0, 'f', 2));
    }
}
