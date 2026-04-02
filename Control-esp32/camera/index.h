#include <pgmspace.h>

const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Robot Control Panel</title>
    <style>
        body { 
            font-family: 'Times New Roman', serif;
            margin: 0; padding: 20px; 
            background-color: #f1f5f9;
            display: flex; flex-wrap: wrap; 
            gap: 20px; justify-content: center;
            user-select: none; -webkit-user-select: none;
        }
        .box {
            background: #ffffff;
            border: 1px solid #cbd5e1;
            padding: 15px;
            width: 320px;
            border-radius: 4px;
        }
        .streamvideo { text-align: center; }
        #stream {
            width: 100%; height: 240px; 
            border: 1px solid #94a3b8;
            background-color: #334155;
            image-rendering: pixelated;
        }
        .sensor-box {
            margin-top: 10px;
            padding: 10px;
            background-color: #f8fafc;
            border: 1px dashed #2563eb;
            text-align: left;
            font-family: Times New Roman;
            font-size: 14px;
            color: #1e293b;
        }
        h3 { 
            margin-top: 0; font-size: 16px;
            color: #2563eb;
            border-bottom: 2px solid #2563eb; 
            padding-bottom: 5px; width: 100%; 
            text-transform: uppercase;
        }
        .status { font-weight: bold; color: #64748b; margin-bottom: 10px; font-size: 13px; }
        .joy-bg {
            width: 180px; height: 180px;
            background-color: #e2e8f0;
            border: 2px solid #94a3b8;
            border-radius: 50%;
            position: relative;
            margin: 15px auto;
            display: flex; justify-content: center; align-items: center;
            touch-action: none;
        }
        .joy-stick {
            width: 60px; height: 60px;
            background-color: #1e293b;
            border-radius: 50%;
            position: absolute;
            cursor: pointer;
            border: 2px solid #f1f5f9;
        }
        .maybomnuoc {
            width: 100%; background-color: #ef4444;
            border: none; color: #ffffff;
            padding: 15px; font-size: 16px; font-weight: bold;
            cursor: pointer; margin-top: 10px;
            border-radius: 4px;
            text-transform: uppercase;
        }
        .maybomnuoc.active { 
            background-color: #b91c1c; 
            color: #fca5a5;
        }
        .brake-label { font-weight: bold; color: #475569; cursor: pointer; display: block; margin-bottom: 10px;}
    </style>
</head>
<body>
    <div class="box streamvideo">
        <h3>Camera</h3>
        <div id="status-text" class="status">Trạng thái: Đang kết nối...</div>
        <img id="stream" src="" alt="">
        <div class="sensor-box">
            Dữ liệu cảm biến: <span id="sensor-val">---</span>
        </div>
    </div>
    <div class="box controlbox">
        <h3>Điều khiển Robot</h3>
        <label class="brake-label">
            <input type="checkbox" id="chk-brake" checked> Phanh tự động
        </label>
        <div class="joy-bg" id="joy-bg">
            <div class="joy-stick" id="joy-stick"></div>
        </div>
        <button id="maybomnuoc" class="maybomnuoc">BẬT BƠM NƯỚC</button>
    </div>
    <script>
        let motorL = 0, motorR = 0, pumpActive = 0;
        const gateway = `ws://${window.location.hostname}/ws`;
        let ws;
        const statusText = document.getElementById('status-text');
        window.onload = () => {
            document.getElementById('stream').src = `http://${window.location.hostname}:81/stream`;
            connectWS();
        };
        function connectWS() {
            ws = new WebSocket(gateway);
            ws.onopen = () => { statusText.innerText = "Trạng thái: Đã kết nối"; statusText.style.color="#16a34a"; };
            ws.onclose = () => { statusText.innerText = "Trạng thái: Mất kết nối"; statusText.style.color="#dc2626"; setTimeout(connectWS, 2000); };
            ws.onmessage = (e) => {
                document.getElementById('sensor-val').innerText = e.data;
            };
        }
        function sendCommand() {
            let autoBrake = document.getElementById('chk-brake').checked ? 1 : 0;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ L: motorL, R: motorR, A: autoBrake, P: pumpActive }));
            }
        }
        const btnPump = document.getElementById('maybomnuoc');
        btnPump.onclick = function() {
            pumpActive = (pumpActive === 0) ? 1 : 0;
            this.classList.toggle('active', pumpActive === 1);
            this.innerText = pumpActive ? "ĐANG BƠM" : "BẬT BƠM NƯỚC";
            sendCommand();
        };
        document.getElementById('chk-brake').onchange = sendCommand;
        const bg = document.getElementById('joy-bg');
        const stick = document.getElementById('joy-stick');
        let isDragging = false;
        const maxR = bg.clientWidth / 2 - stick.clientWidth / 2; 
        function moveJoy(clientX, clientY) {
            if (!isDragging) return;
            const rect = bg.getBoundingClientRect();
            const cx = rect.left + rect.width / 2;
            const cy = rect.top + rect.height / 2;
            let dx = clientX - cx;
            let dy = clientY - cy;
            let dist = Math.sqrt(dx * dx + dy * dy);
            if (dist > maxR) { dx = (dx / dist) * maxR; dy = (dy / dist) * maxR; }
            stick.style.transform = `translate(${dx}px, ${dy}px)`;
            let fwd = (-dy / maxR) * 255; 
            let turn = (dx / maxR) * 255;
            motorL = Math.round(Math.max(-255, Math.min(255, fwd + turn)));
            motorR = Math.round(Math.max(-255, Math.min(255, fwd - turn)));
            sendCommand();
        }
        function stopJoy() {
            isDragging = false;
            stick.style.transform = `translate(0px, 0px)`;
            motorL = 0; motorR = 0;
            sendCommand();
        }
        stick.onmousedown = () => isDragging = true;
        document.onmousemove = (e) => moveJoy(e.clientX, e.clientY);
        document.onmouseup = stopJoy;
        stick.ontouchstart = (e) => { isDragging = true; e.preventDefault(); };
        document.ontouchmove = (e) => { if(isDragging) moveJoy(e.touches[0].clientX, e.touches[0].clientY); };
        document.ontouchend = stopJoy;
    </script>
</body>
</html>
)=====";