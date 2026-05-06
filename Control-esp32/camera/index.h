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
            margin: 0; padding: 20px 10px; 
            background-color: #e2e8f0;
            display: flex; justify-content: center; align-items: center;
            min-height: 100vh;
            user-select: none; -webkit-user-select: none;
        }

        .container {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            width: 100%;
            max-width: 760px;
            align-items: stretch; 
        }
        
        .box {
            background: #ffffff;
            border: none;
            padding: 24px;
            border-radius: 16px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.06);
            display: flex;
            flex-direction: column;
        }

        h3 { 
            margin-top: 0; font-size: 16px; font-weight: bold;
            color: #0f172a;
            border-bottom: 2px solid #e2e8f0; 
            padding-bottom: 10px; margin-bottom: 15px; width: 100%; 
            text-transform: uppercase; letter-spacing: 1px;
            text-align: center;
        }

        .status { 
            font-weight: bold; color: #64748b; margin-bottom: 12px; font-size: 13px; text-align: center; 
        }
        
        .streamvideo { flex: 1; display: flex; flex-direction: column; }
        #stream {
            width: 100%; height: 200px; 
            border: none; border-radius: 8px;
            background-color: #0f172a;
            box-shadow: inset 0 0 10px rgba(0,0,0,0.8);
            object-fit: cover;
            margin-bottom: auto;
        }

        #sensor-val { margin-top: 20px; }
        .sensor-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
        }
        .sensor-card {
            background: #f8fafc; border: 1px solid #e2e8f0;
            border-radius: 10px; padding: 12px 10px; text-align: center;
            box-shadow: 0 2px 6px rgba(0,0,0,0.02);
        }
        .sensor-label { font-size: 11px; color: #64748b; text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: 4px; font-weight: bold; }
        .sensor-value { font-size: 18px; font-weight: bold; color: #0f172a; }
        .sensor-value span { font-size: 12px; color: #94a3b8; font-weight: normal; }
        .val-fire { color: #ef4444; }

        .controlbox { flex: 1; display: flex; flex-direction: column; }
        #manual-controls { display: flex; flex-direction: column; flex: 1; }

        .toggle-container {
            display: flex; align-items: center; justify-content: space-between;
            margin: 15px 0 20px 0;
            background: #f8fafc; padding: 10px 15px; border-radius: 8px; border: 1px solid #e2e8f0;
        }
        .brake-label { font-weight: bold; color: #334155; font-size: 14px; margin: 0; }
        .switch { position: relative; display: inline-block; width: 44px; height: 24px; }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #cbd5e1; transition: .3s; border-radius: 24px; }
        .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 3px; bottom: 3px; background-color: white; transition: .3s; border-radius: 50%; box-shadow: 0 2px 4px rgba(0,0,0,0.2); }
        input:checked + .slider { background-color: #10b981; }
        input:checked + .slider:before { transform: translateX(20px); }

        .joy-bg {
            width: 170px; height: 170px;
            background-color: #f8fafc; border: 3px solid #cbd5e1; border-radius: 50%;
            position: relative; margin: 0 auto auto auto;
            display: flex; justify-content: center; align-items: center;
            touch-action: none; box-shadow: inset 0 5px 15px rgba(0,0,0,0.05);
        }
        .joy-stick {
            width: 56px; height: 56px;
            background: linear-gradient(135deg, #334155, #0f172a); border-radius: 50%;
            position: absolute; cursor: grab; border: 2px solid #64748b;
            box-shadow: 0 5px 15px rgba(0,0,0,0.4); transition: box-shadow 0.1s;
        }
        .joy-stick:active { cursor: grabbing; box-shadow: 0 2px 8px rgba(0,0,0,0.5); }

        .btn-control {
            width: 100%; border: none; color: #ffffff;
            padding: 14px; font-size: 14px; font-weight: bold; font-family: 'Times New Roman', serif;
            cursor: pointer; border-radius: 8px; text-transform: uppercase; letter-spacing: 1px;
            transition: all 0.15s; box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }
        .btn-control:active { transform: translateY(2px); box-shadow: 0 2px 5px rgba(0,0,0,0.2); filter: brightness(0.9); }
        
        .btn-auto { background: linear-gradient(135deg, #10b981, #059669); margin-bottom: 10px; }
        .btn-auto.active { background: #064e3b; box-shadow: inset 0 3px 6px rgba(0,0,0,0.4); } 
        
        .btn-pump { background: linear-gradient(135deg, #ef4444, #dc2626); margin-top: 15px; }
        .btn-pump.active { background: #7f1d1d; box-shadow: inset 0 3px 6px rgba(0,0,0,0.4); } 
        
        .btn-voi { background: linear-gradient(135deg, #0ea5e9, #0284c7); width: 48%; }
        .btn-voi.pressing { background: #0369a1; box-shadow: inset 0 3px 6px rgba(0,0,0,0.4); } 
        
        .action-row { display: flex; justify-content: space-between; margin-top: 20px; }

        .disabled-ui { opacity: 0.4; pointer-events: none; filter: grayscale(50%); }

        @media (max-width: 650px) {
            .container { grid-template-columns: 1fr; }
            body { padding: 15px; }
        }

        @media (orientation: landscape) and (max-height: 500px) {
            body { padding: 10px; }
            .container { gap: 15px; }
            .box { padding: 15px; }
            
            #stream { height: 130px; }
            h3 { font-size: 14px; margin-bottom: 8px; padding-bottom: 4px; }
            
            .sensor-grid { grid-template-columns: 1fr 1fr 1fr; gap: 6px; }
            .sensor-card { padding: 6px 4px; border-radius: 6px; }
            .sensor-label { font-size: 9px; margin-bottom: 2px; }
            .sensor-value { font-size: 14px; }
            .sensor-value span { font-size: 10px; }
            
            .btn-control { padding: 10px; font-size: 12px; }
            .toggle-container { margin: 8px 0; padding: 6px 10px; }
            .joy-bg { width: 130px; height: 130px; margin-top: 5px; }
            .joy-stick { width: 46px; height: 46px; }
            .action-row { margin-top: 10px; }
            .btn-pump { margin-top: 10px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="box streamvideo">
            <h3>Camera Quan Sát</h3>
            <div id="status-text" class="status">Trạng thái: Đang kết nối...</div>
            <img id="stream" src="" alt="">
            <div id="sensor-val">Đang đợi dữ liệu...</div>
        </div>
        
        <div class="box controlbox">
            <h3>Điều Khiển Robot</h3>
            <button id="btn-auto" class="btn-control btn-auto">BẬT TỰ ĐỘNG</button>
            <div id="manual-controls">
                
                <div class="toggle-container">
                    <span class="brake-label">Phanh Tự Động (ABS)</span>
                    <label class="switch">
                        <input type="checkbox" id="chk-brake" checked>
                        <span class="slider"></span>
                    </label>
                </div>

                <div class="joy-bg" id="joy-bg">
                    <div class="joy-stick" id="joy-stick"></div>
                </div>
                
                <div class="action-row">
                    <button id="btn-v-left" class="btn-control btn-voi">QUAY TRÁI</button>
                    <button id="btn-v-right" class="btn-control btn-voi">QUAY PHẢI</button>
                </div>
                
                <button id="maybomnuoc" class="btn-control btn-pump">BẬT BƠM NƯỚC</button>
            </div>
        </div>
    </div>

    <script>
        let motorL = 0, motorR = 0, pumpActive = 0, autoMode = 0;
        let vCmd = ""; 
        const gateway = `ws://${window.location.hostname}/ws`;
        let ws;
        const statusText = document.getElementById('status-text');

        window.onload = () => {
            document.getElementById('stream').src = `http://${window.location.hostname}:81/stream`;
            connectWS();
        };

        function connectWS() {
            ws = new WebSocket(gateway);
            ws.onopen = () => { statusText.innerText = "Trạng thái: Đã kết nối"; statusText.style.color="#10b981"; };
            ws.onclose = () => { statusText.innerText = "Trạng thái: Mất kết nối"; statusText.style.color="#ef4444"; setTimeout(connectWS, 2000); };
            ws.onmessage = (e) => {
                try {
                    let obj = JSON.parse(e.data);
                    let fireColorClass = obj.C > 2000 ? "val-fire" : "";
                    
                    document.getElementById('sensor-val').innerHTML = `
                        <div class="sensor-grid">
                            <div class="sensor-card">
                                <div class="sensor-label">Cường Độ Lửa</div>
                                <div class="sensor-value ${fireColorClass}">${obj.C.toFixed(0)}</div>
                            </div>
                            <div class="sensor-card">
                                <div class="sensor-label">Góc Lệch</div>
                                <div class="sensor-value">${obj.G.toFixed(0)}<span>°</span></div>
                            </div>
                            <div class="sensor-card">
                                <div class="sensor-label">Trước</div>
                                <div class="sensor-value">${obj.F}<span>cm</span></div>
                            </div>
                            <div class="sensor-card">
                                <div class="sensor-label">Trái</div>
                                <div class="sensor-value">${obj.L}<span>cm</span></div>
                            </div>
                            <div class="sensor-card">
                                <div class="sensor-label">Phải</div>
                                <div class="sensor-value">${obj.R}<span>cm</span></div>
                            </div>
                        </div>
                    `;
                } catch(err) {
                    document.getElementById('sensor-val').innerText = e.data; 
                }
            };
        }

        function sendCommand() {
            let autoBrake = document.getElementById('chk-brake').checked ? 1 : 0;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ L: motorL, R: motorR, A: autoBrake, P: pumpActive, M: autoMode, V: vCmd }));
            }
        }
        setInterval(sendCommand, 200);

        const btnAuto = document.getElementById('btn-auto');
        const manualControls = document.getElementById('manual-controls');
        btnAuto.onclick = function() {
            autoMode = (autoMode === 0) ? 1 : 0;
            this.classList.toggle('active', autoMode === 1);
            this.innerText = autoMode ? "ĐANG CHẠY TỰ ĐỘNG" : "BẬT TỰ ĐỘNG";
            if(autoMode === 1) {
                manualControls.classList.add('disabled-ui');
                motorL = 0; motorR = 0; pumpActive = 0; vCmd = "";
            } else {
                manualControls.classList.remove('disabled-ui');
            }
            sendCommand();
        };
        const btnPump = document.getElementById('maybomnuoc');
        btnPump.onclick = function() {
            pumpActive = (pumpActive === 0) ? 1 : 0;
            this.classList.toggle('active', pumpActive === 1);
            this.innerText = pumpActive ? "TẮT BƠM NƯỚC" : "BẬT BƠM NƯỚC";
            sendCommand();
        };

        const btnVLeft = document.getElementById('btn-v-left');
        const btnVRight = document.getElementById('btn-v-right');

        function handleVoi(btn, cmd) {
            const start = (e) => { 
                e.preventDefault(); 
                vCmd = cmd; 
                btn.classList.add('pressing'); 
                sendCommand(); 
            };
            const end = (e) => { 
                e.preventDefault(); 
                vCmd = ""; 
                btn.classList.remove('pressing'); 
                sendCommand(); 
            };
            
            btn.onpointerdown = start;
            btn.onpointerup = end;
            btn.onpointerleave = end; 
        }

        handleVoi(btnVLeft, "Q");
        handleVoi(btnVRight, "E");

        document.getElementById('chk-brake').onchange = sendCommand;

        const bg = document.getElementById('joy-bg');
        const stick = document.getElementById('joy-stick');
        let isDragging = false;
        
        function moveJoy(clientX, clientY) {
            if (!isDragging || autoMode === 1) return;
            const maxR = bg.clientWidth / 2 - stick.clientWidth / 2;
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