#pragma once

#include <QMainWindow>

class QWebEngineView;
class ScriptEngine;
class ScriptBridge;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupWebView();
    void setupWebChannel();

    QWebEngineView *m_webView = nullptr;
    ScriptEngine *m_scriptEngine = nullptr;
    ScriptBridge *m_scriptBridge = nullptr;
};
