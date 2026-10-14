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

        // 监听 C++ 端信号
        scriptBridge.logMessage.connect(function(msg) {
            console.log('[C++ Log]', msg);
        });

        scriptBridge.executionFinished.connect(function(result) {
            console.log('[C++] 执行结果:', result);
        });

        setStatus('通信通道已就绪', 'green');
    });
}
