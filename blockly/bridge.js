// QWebChannel 通信封装
let scriptBridge = null;

function initBridge() {
    if (typeof QWebChannel === 'undefined') {
        setStatus('QWebChannel 未加载，请检查 qrc 资源', 'red');
        return;
    }

    new QWebChannel(qt.webChannelTransport, function(channel) {
        scriptBridge = channel.objects.scriptBridge;
        window.scriptBridge = scriptBridge;

        // ---- 监听 C++ 端信号 ----
        // 执行日志
        scriptBridge.logMessage.connect(function(msg) {
            appendLog(msg);
        });

        // 执行结束：taskId + "OK" / "ABORTED" / "ERROR: ..."
        scriptBridge.executionFinished.connect(function(taskId, result) {
            onExecutionFinished(taskId, result);
        });

        // 测试结果行
        scriptBridge.testResultReceived.connect(
            function(link, item, cond, target, value, unit, status) {
                appendResult(link, item, cond, target, value, unit, status);
            });

        // 步骤高亮
        scriptBridge.stepStarted.connect(function(blockId) {
            onStepStarted(blockId);
        });

        setStatus('通信通道已就绪', 'green');
    });
}
