// Blockly 10 的 lua_compressed.js 通过 UMD 导出，并以 Blockly.Lua 之名挂载 Lua 生成器
// 实例（源码：root.Blockly.Lua = root.lua.luaGenerator），故此处直接取 Blockly.Lua。
const luaGenerator = Blockly.Lua;

let workspace;
let savedWorkspaceJson = null;

document.addEventListener('DOMContentLoaded', function() {
    // 定义自定义积木
    defineCustomBlocks();

    // 注入 Blockly 工作区
    workspace = Blockly.inject('blocklyDiv', {
        toolbox: toolboxDefinition,       // 来自 toolbox.js
        trashcan: true,
        // 不指定 media 时 Blockly 会回落到官方远程默认地址
        // （https://blockly-demo.appspot.com/static/media/），导致缩放控件与垃圾桶的
        // 图标精灵图加载失败/异常。这里指向随程序打包的本地副本。
        media: 'media/',
        // media 目录下只打包了 sprites.png 图标；音频（click/disconnect/delete）未随包，
        // 保留默认的 sounds:true 会产生 3 条 QResource 缺失告警，故关闭音效。
        sounds: false,
        zoom: { controls: true, wheel: true, startScale: 0.9 },
        grid: { spacing: 20, length: 3, colour: '#ccc', snap: true },
        renderer: 'zelos'                 // 使用块状渲染器，更美观
    });

    // 注册 Lua 代码生成器
    registerLuaGenerators(luaGenerator);

    // 初始化 QWebChannel 通信
    initBridge();

    // 按钮事件绑定
    document.getElementById('btnRun').addEventListener('click', onRunClicked);
    document.getElementById('btnSave').addEventListener('click', onSaveClicked);
    document.getElementById('btnLoad').addEventListener('click', onLoadClicked);
});

function onRunClicked() {
    const luaCode = luaGenerator.workspaceToCode(workspace);
    if (!luaCode.trim()) {
        setStatus('工作区为空，请先拖入积木', 'orange');
        return;
    }
    setStatus('正在执行...', 'blue');
    // 通过 QWebChannel 发送给 C++
    if (window.scriptBridge) {
        window.scriptBridge.executeScript(luaCode, function(result) {
            if (result === 'OK') {
                setStatus('执行成功', 'green');
            } else {
                setStatus('执行失败: ' + result, 'red');
            }
        });
    } else {
        setStatus('通信通道未就绪', 'red');
    }
}

function onSaveClicked() {
    savedWorkspaceJson = Blockly.serialization.workspaces.save(workspace);
    setStatus('工作区已保存到内存', 'green');
}

function onLoadClicked() {
    if (!savedWorkspaceJson) {
        setStatus('没有可加载的工作区', 'orange');
        return;
    }
    Blockly.serialization.workspaces.load(savedWorkspaceJson, workspace);
    setStatus('工作区已加载', 'green');
}

function setStatus(text, color) {
    const el = document.getElementById('status');
    el.textContent = text;
    el.style.color = color || '#666';
}
