// Blockly 10 的 lua_compressed.js 通过 UMD 导出，并以 Blockly.Lua 之名挂载 Lua 生成器
// 实例（源码：root.Blockly.Lua = root.lua.luaGenerator），故此处直接取 Blockly.Lua。
const luaGenerator = Blockly.Lua;

let workspace;
let currentHighlighted = null;

// 步骤高亮节流：50ms 内的多次 reportStep 合并为最后一次
let highlightTimer = null;
let pendingBlockId = null;
let lastHighlightTs = 0;

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
    document.getElementById('btnStop').addEventListener('click', onStopClicked);
    document.getElementById('btnSave').addEventListener('click', onSaveClicked);
    document.getElementById('btnLoad').addEventListener('click', onLoadClicked);
});

// ---------------------------------------------------------------------------
// 工具栏动作
// ---------------------------------------------------------------------------

function onRunClicked() {
    const luaCode = luaGenerator.workspaceToCode(workspace);
    if (!luaCode.trim()) {
        setStatus('工作区为空，请先拖入积木', 'orange');
        return;
    }
    if (!window.scriptBridge) {
        setStatus('通信通道未就绪', 'red');
        return;
    }

    // 清空上一轮的执行反馈
    clearLog();
    clearResults();
    clearHighlight();
    pendingBlockId = null;
    if (highlightTimer) { clearTimeout(highlightTimer); highlightTimer = null; }

    setStatus('正在执行...', 'blue');
    window.scriptBridge.submitScript(luaCode, function(taskId) {
        console.log('[C++] 任务已提交:', taskId);
    });
}

function onStopClicked() {
    if (!window.scriptBridge) return;
    window.scriptBridge.abort();
    setStatus('正在中断...', 'orange');
}

function onSaveClicked() {
    if (!window.scriptBridge) return;
    const json = JSON.stringify(Blockly.serialization.workspaces.save(workspace));
    window.scriptBridge.saveWorkspace(json, function(ok) {
        setStatus(ok ? '工作区已保存' : '保存已取消', ok ? 'green' : 'orange');
    });
}

function onLoadClicked() {
    if (!window.scriptBridge) return;
    window.scriptBridge.loadWorkspace(function(json) {
        if (!json) {
            setStatus('加载已取消', 'orange');
            return;
        }
        try {
            Blockly.serialization.workspaces.load(JSON.parse(json), workspace);
            setStatus('工作区已加载', 'green');
        } catch (e) {
            setStatus('工作区解析失败: ' + e, 'red');
        }
    });
}

// ---------------------------------------------------------------------------
// 执行反馈
// ---------------------------------------------------------------------------

function onExecutionFinished(taskId, result) {
    clearHighlight();
    pendingBlockId = null;
    if (highlightTimer) { clearTimeout(highlightTimer); highlightTimer = null; }

    if (result === 'OK') {
        setStatus('执行成功', 'green');
    } else if (result === 'ABORTED') {
        setStatus('已中断', 'orange');
    } else {
        setStatus('执行失败: ' + result, 'red');
    }
    appendLog('[执行结束] ' + result);
}

// 步骤高亮（50ms 节流，合并高频 reportStep）
function onStepStarted(blockId) {
    const now = Date.now();
    const elapsed = now - lastHighlightTs;

    if (elapsed >= 50) {
        applyHighlight(blockId);
        lastHighlightTs = now;
        pendingBlockId = null;
        return;
    }

    // 距离上次高亮不足 50ms：记录最新积木，等剩余时间到达后再应用
    pendingBlockId = blockId;
    if (!highlightTimer) {
        highlightTimer = setTimeout(function() {
            highlightTimer = null;
            if (pendingBlockId) {
                applyHighlight(pendingBlockId);
                pendingBlockId = null;
                lastHighlightTs = Date.now();
            }
        }, 50 - elapsed);
    }
}

function applyHighlight(blockId) {
    const block = workspace.getBlockById(blockId);
    if (!block) return;

    if (currentHighlighted && currentHighlighted.id !== blockId) {
        try { currentHighlighted.removeSelect(); } catch (e) { /* 忽略 */ }
    }

    try {
        block.addSelect();
        block.bringToFront();
    } catch (e) { /* 忽略 */ }
    currentHighlighted = block;

    scrollIntoViewIfNeeded(block);
}

function clearHighlight() {
    if (currentHighlighted) {
        try { currentHighlighted.removeSelect(); } catch (e) { /* 忽略 */ }
        currentHighlighted = null;
    }
}

// 当前积木在视野外时，自动滚动到可见
function scrollIntoViewIfNeeded(block) {
    try {
        const svgRoot = block.getSvgRoot();
        if (!svgRoot) return;
        const blockRect = svgRoot.getBoundingClientRect();
        const viewRect = document.getElementById('blocklyDiv').getBoundingClientRect();

        const outOfView = blockRect.top < viewRect.top || blockRect.bottom > viewRect.bottom ||
                          blockRect.left < viewRect.left || blockRect.right > viewRect.right;
        if (outOfView) {
            workspace.centerOnBlock(block.id);
        }
    } catch (e) { /* 忽略 */ }
}

// ---------------------------------------------------------------------------
// 日志面板 / 结果表格
// ---------------------------------------------------------------------------

function appendLog(msg) {
    const el = document.getElementById('log');
    const line = document.createElement('div');
    line.textContent = msg;
    el.appendChild(line);
    el.scrollTop = el.scrollHeight;   // 实时滚动到底部
}

function clearLog() {
    document.getElementById('log').innerHTML = '';
}

function appendResult(link, item, cond, target, value, unit, status) {
    const tbody = document.querySelector('#resultTable tbody');
    const row = document.createElement('tr');
    row.className = (status === 'Pass') ? 'pass-row' : 'fail-row';

    const cells = [link, item, cond, target, value + ' ' + unit];
    cells.forEach(function(text) {
        const td = document.createElement('td');
        td.textContent = text;
        row.appendChild(td);
    });

    const tdStatus = document.createElement('td');
    tdStatus.textContent = status;
    tdStatus.className = (status === 'Pass') ? 'pass' : 'fail';
    row.appendChild(tdStatus);

    tbody.appendChild(row);
    row.scrollIntoView({ block: 'nearest' });
}

function clearResults() {
    document.querySelector('#resultTable tbody').innerHTML = '';
}

function setStatus(text, color) {
    const el = document.getElementById('status');
    el.textContent = text;
    el.style.color = color || '#666';
}
