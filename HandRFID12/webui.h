// ============================================================
//  HandRFID-Touch – Web UI (embedded HTML)
// ============================================================
#pragma once
#pragma once

const char WEBUI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>HandRFID - Control Panel</title>
<style>
:root {
  --bg: #0d1117; --surface: #161b22; --border: #30363d;
  --accent: #58a6ff; --accent2: #3fb950; --danger: #f85149;
  --text: #e6edf3; --muted: #8b949e; --input-bg: #0d1117;
  --warn: #d29922;
}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:'Segoe UI',Arial,sans-serif;background:var(--bg);color:var(--text);min-height:100vh}

/* Header */
.header{background:linear-gradient(135deg,#161b22 0%,#1a2332 100%);border-bottom:1px solid var(--border);padding:16px 20px;display:flex;align-items:center;gap:14px}
.header svg{width:36px;height:36px;fill:var(--accent)}
.header h1{font-size:1.3em;font-weight:600;color:var(--text)}
.header h1 span{color:var(--accent);font-weight:700}
.header .ver{margin-left:auto;color:var(--muted);font-size:0.75em}

/* Container */
.container{max-width:960px;margin:0 auto;padding:16px}

/* Status */
.status{padding:10px 16px;border-radius:8px;text-align:center;font-weight:500;margin-bottom:12px;display:none;animation:fadeIn .3s}
.status.ok{display:block;background:#0d2818;color:var(--accent2);border:1px solid #238636}
.status.err{display:block;background:#2d1117;color:var(--danger);border:1px solid #f85149}
@keyframes fadeIn{from{opacity:0;transform:translateY(-6px)}to{opacity:1;transform:translateY(0)}}

/* Tabs */
.tabs{display:flex;gap:2px;margin-bottom:0;border-bottom:2px solid var(--border)}
.tab{padding:10px 24px;background:transparent;border:none;color:var(--muted);font-weight:600;font-size:0.9em;cursor:pointer;border-bottom:2px solid transparent;margin-bottom:-2px;transition:all .2s}
.tab:hover{color:var(--text);background:rgba(88,166,255,0.04)}
.tab.active{color:var(--accent);border-bottom-color:var(--accent)}

/* Panels */
.panel{display:none;background:var(--surface);border:1px solid var(--border);border-top:none;border-radius:0 0 10px 10px;padding:16px;animation:fadeIn .2s}
.panel.active{display:block}

/* Table */
.tbl-wrap{overflow-x:auto;border-radius:8px;border:1px solid var(--border)}
table{width:100%;border-collapse:collapse;font-size:0.85em}
th{background:#21262d;color:var(--accent);padding:10px 8px;text-align:left;font-weight:600;font-size:0.8em;text-transform:uppercase;letter-spacing:.5px;position:sticky;top:0;z-index:1}
td{padding:6px 4px;border-top:1px solid var(--border)}
tr:hover td{background:rgba(88,166,255,0.04)}
td input{width:100%;background:var(--input-bg);border:1px solid var(--border);color:var(--text);padding:6px 8px;border-radius:6px;font-size:0.85em;transition:border .2s}
td input:focus{outline:none;border-color:var(--accent);box-shadow:0 0 0 2px rgba(88,166,255,0.15)}
td input[type=number]{width:64px;text-align:center}

/* Buttons */
.btn-row{margin:12px 0;display:flex;gap:8px;flex-wrap:wrap;align-items:center}
button{padding:8px 16px;border:none;border-radius:6px;cursor:pointer;font-weight:600;font-size:0.85em;transition:all .15s}
button:active{transform:scale(0.97)}
.btn-add{background:rgba(88,166,255,0.1);color:var(--accent);border:1px solid var(--accent)}
.btn-add:hover{background:rgba(88,166,255,0.2)}
.btn-del{background:transparent;color:var(--danger);padding:4px 10px;font-size:0.85em;border:1px solid transparent;border-radius:6px}
.btn-del:hover{background:rgba(248,81,73,0.1);border-color:var(--danger)}
.actions{display:flex;justify-content:center;gap:12px;margin:20px 0;padding:16px 0;border-top:1px solid var(--border)}
.btn-save{background:var(--accent2);color:#0d1117;font-size:1em;padding:12px 40px;border-radius:8px;font-weight:700;letter-spacing:.3px}
.btn-save:hover{background:#46d160;box-shadow:0 0 16px rgba(63,185,80,0.3)}
.btn-load{background:var(--surface);color:var(--muted);border:1px solid var(--border);padding:12px 24px;border-radius:8px}
.btn-load:hover{color:var(--text);border-color:var(--muted)}

/* Settings panel */
.set-group{margin-bottom:20px;padding:16px;background:var(--bg);border:1px solid var(--border);border-radius:10px}
.set-group h3{font-size:0.95em;color:var(--accent);margin-bottom:12px;padding-bottom:8px;border-bottom:1px solid var(--border)}
.set-row{display:flex;align-items:center;justify-content:space-between;padding:10px 0;border-bottom:1px solid rgba(48,54,61,0.5)}
.set-row:last-child{border-bottom:none}
.set-label{font-size:0.9em;color:var(--text)}
.set-label small{display:block;color:var(--muted);font-size:0.8em;margin-top:2px}
.set-ctrl{display:flex;align-items:center;gap:8px}
.set-ctrl select,.set-ctrl input[type=number]{background:var(--input-bg);border:1px solid var(--border);color:var(--text);padding:6px 12px;border-radius:6px;font-size:0.9em}
.set-ctrl select{min-width:120px}
.set-ctrl input[type=number]{width:80px;text-align:center}
.set-ctrl input[type=range]{width:140px;accent-color:var(--accent)}
.toggle{position:relative;width:48px;height:26px;cursor:pointer}
.toggle input{opacity:0;width:0;height:0}
.toggle .slider{position:absolute;inset:0;background:var(--border);border-radius:13px;transition:.3s}
.toggle .slider:before{content:'';position:absolute;width:20px;height:20px;left:3px;bottom:3px;background:var(--muted);border-radius:50%;transition:.3s}
.toggle input:checked+.slider{background:var(--accent2)}
.toggle input:checked+.slider:before{transform:translateX(22px);background:white}
.btn-danger{background:rgba(248,81,73,0.1);color:var(--danger);border:1px solid var(--danger);padding:8px 16px;border-radius:6px;font-weight:600}
.btn-danger:hover{background:rgba(248,81,73,0.2)}
.btn-warn{background:rgba(210,153,34,0.1);color:var(--warn);border:1px solid var(--warn);padding:8px 16px;border-radius:6px;font-weight:600}
.btn-warn:hover{background:rgba(210,153,34,0.2)}
.info-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.info-item{padding:8px 12px;background:var(--surface);border:1px solid var(--border);border-radius:6px}
.info-item .lbl{font-size:0.75em;color:var(--muted);text-transform:uppercase;letter-spacing:.5px}
.info-item .val{font-size:1em;color:var(--text);font-weight:600;margin-top:2px}

/* Manufacturer list */
.mfg-list{display:flex;flex-wrap:wrap;gap:8px;margin-top:8px}
.mfg-chip{display:flex;align-items:center;gap:6px;background:var(--surface);border:1px solid var(--border);padding:6px 12px;border-radius:20px;font-size:0.85em}
.mfg-chip .mfg-del{cursor:pointer;color:var(--danger);font-weight:bold;font-size:1.1em;line-height:1}
.mfg-chip .mfg-del:hover{color:#ff6b6b}
.mfg-add{display:flex;gap:6px;margin-top:10px}
.mfg-add input{background:var(--input-bg);border:1px solid var(--border);color:var(--text);padding:6px 12px;border-radius:6px;font-size:0.85em;flex:1;max-width:200px}

/* Footer */
.footer{text-align:center;color:var(--muted);font-size:0.72em;padding:12px 0;margin-top:8px}

/* Responsive */
@media(max-width:600px){
  .header{padding:12px 14px}
  .header h1{font-size:1.1em}
  .tab{padding:8px 14px;font-size:0.82em}
  .container{padding:10px}
  td input[type=number]{width:52px}
  .info-grid{grid-template-columns:1fr}
  .set-row{flex-direction:column;align-items:flex-start;gap:8px}
}
</style>
</head>
<body>

<div class="header">
  <svg viewBox="0 0 24 24"><path d="M20 4H4c-1.1 0-2 .9-2 2v12c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V6c0-1.1-.9-2-2-2zm0 14H4V6h16v12zM6 10h2v2H6zm0 4h8v2H6zm10-4h2v6h-2zm-6-2h2v2h-2zm-4 0h2v2H6z"/></svg>
  <h1><span>Hand</span>RFID</h1>
  <div class="ver" id="fw-ver">Control Panel</div>
</div>

<div class="container">
  <div id="status" class="status"></div>

  <div class="tabs">
    <div class="tab active" onclick="showTab('qidi',this)" id="tab-qidi">QIDI</div>
    <div class="tab" onclick="showTab('anycubic',this)" id="tab-anycubic">Anycubic</div>
    <div class="tab" onclick="showTab('settings',this)" id="tab-settings">Settings</div>
  </div>

  <!-- QIDI panel -->
  <div id="qidi" class="panel active">
    <div class="btn-row">
      <button class="btn-add" onclick="addRow('qidi')" id="btn-add-qidi">+ Add Material</button>
      <span style="color:var(--muted);font-size:0.8em;margin-left:8px" id="cnt-qidi"></span>
    </div>
    <div class="tbl-wrap">
    <table>
      <thead><tr><th id="th-code">Code</th><th id="th-mat-q">Material</th><th>SKU</th><th id="th-emin">Ext Min</th><th id="th-emax">Ext Max</th><th id="th-bmin">Bed Min</th><th id="th-bmax">Bed Max</th><th style="width:36px"></th></tr></thead>
      <tbody id="tbl-qidi"></tbody>
    </table>
    </div>
    <div class="actions">
      <button class="btn-save" onclick="saveAll()" id="btn-save-mat">SAVE MATERIALS</button>
      <button class="btn-load" onclick="loadAll()" id="btn-reload">RELOAD</button>
    </div>
  </div>

  <!-- Anycubic panel -->
  <div id="anycubic" class="panel">
    <div class="btn-row">
      <button class="btn-add" onclick="addRow('anycubic')" id="btn-add-ace">+ Add Material</button>
      <span style="color:var(--muted);font-size:0.8em;margin-left:8px" id="cnt-anycubic"></span>
    </div>
    <div class="tbl-wrap">
    <table>
      <thead><tr><th id="th-mat-a">Material</th><th>SKU</th><th id="th-emin2">Ext Min</th><th id="th-emax2">Ext Max</th><th id="th-bmin2">Bed Min</th><th id="th-bmax2">Bed Max</th><th style="width:36px"></th></tr></thead>
      <tbody id="tbl-anycubic"></tbody>
    </table>
    </div>
    <div class="actions">
      <button class="btn-save" onclick="saveAll()" id="btn-save-mat2">SAVE MATERIALS</button>
      <button class="btn-load" onclick="loadAll()" id="btn-reload2">RELOAD</button>
    </div>
  </div>

  <!-- Settings panel -->
  <div id="settings" class="panel">

    <div class="set-group">
      <h3 id="h-devinfo">Device Info</h3>
      <div class="info-grid">
        <div class="info-item"><div class="lbl" id="lbl-fw">Firmware</div><div class="val" id="si-fw">-</div></div>
        <div class="info-item"><div class="lbl" id="lbl-ip">IP Address</div><div class="val" id="si-ip">-</div></div>
        <div class="info-item"><div class="lbl" id="lbl-nfc">NFC Module</div><div class="val" id="si-nfc">-</div></div>
        <div class="info-item"><div class="lbl" id="lbl-heap">Free Heap</div><div class="val" id="si-heap">-</div></div>
        <div class="info-item"><div class="lbl" id="lbl-up">Uptime</div><div class="val" id="si-up">-</div></div>
        <div class="info-item"><div class="lbl" id="lbl-rssi">WiFi Signal</div><div class="val" id="si-rssi">-</div></div>
      </div>
    </div>

    <div class="set-group">
      <h3 id="h-general">General</h3>
      <div class="set-row">
        <div class="set-label" id="lbl-lang">Language<small id="lbl-lang-s">Interface language</small></div>
        <div class="set-ctrl"><select id="s-lang"><option value="0">English</option><option value="1">Italiano</option></select></div>
      </div>
      <div class="set-row">
        <div class="set-label" id="lbl-auto">Auto Mode<small id="lbl-auto-s">Automatically read tags on contact</small></div>
        <div class="set-ctrl"><label class="toggle"><input type="checkbox" id="s-auto"><span class="slider"></span></label></div>
      </div>
    </div>

    <div class="set-group">
      <h3 id="h-display">Display</h3>
      <div class="set-row">
        <div class="set-label" id="lbl-bl">Backlight Brightness<small id="lbl-bl-s">0 = off, 255 = max</small></div>
        <div class="set-ctrl">
          <input type="range" id="s-bl" min="0" max="255" value="255">
          <span id="s-bl-val" style="color:var(--accent);min-width:32px;text-align:center">255</span>
        </div>
      </div>
      <div class="set-row">
        <div class="set-label" id="lbl-rainbow">Rainbow Title<small id="lbl-rainbow-s">Animated rainbow colors on "HandRFID"</small></div>
        <div class="set-ctrl"><label class="toggle"><input type="checkbox" id="s-rainbow" checked><span class="slider"></span></label></div>
      </div>
      <div class="set-row">
        <div class="set-label" id="lbl-ss">Auto Power Off<small id="lbl-ss-s">Seconds of inactivity before screen + backlight off (0 = never)</small></div>
        <div class="set-ctrl"><input type="number" id="s-ss" min="0" max="3600" value="120"></div>
      </div>
      <div class="set-row">
        <div class="set-label" id="lbl-dbg">Debug Log Panel<small id="lbl-dbg-s">Show real-time debug log on main screen</small></div>
        <div class="set-ctrl"><label class="toggle"><input type="checkbox" id="s-dbgpanel"><span class="slider"></span></label></div>
      </div>
    </div>

    <div class="set-group">
      <h3 id="h-mfg">Manufacturers</h3>
      <div id="mfg-chips" class="mfg-list"></div>
      <div class="mfg-add">
        <input type="text" id="mfg-new" placeholder="New manufacturer..." maxlength="23" id="inp-mfg-new">
        <button class="btn-add" onclick="addMfg()" id="btn-add-mfg">+ Add</button>
      </div>
    </div>

    <div class="set-group">
      <h3 id="h-danger">Danger Zone</h3>
      <div class="set-row">
        <div class="set-label" id="lbl-rwifi">Reset WiFi<small id="lbl-rwifi-s">Clear credentials, device will reboot into setup portal</small></div>
        <div class="set-ctrl"><button class="btn-danger" onclick="resetWifi()" id="btn-rwifi">Reset WiFi</button></div>
      </div>
      <div class="set-row">
        <div class="set-label" id="lbl-rmat">Reset Materials<small id="lbl-rmat-s">Delete all custom materials and restore defaults</small></div>
        <div class="set-ctrl"><button class="btn-danger" onclick="resetMaterials()" id="btn-rmat">Reset Materials</button></div>
      </div>
      <div class="set-row">
        <div class="set-label" id="lbl-reboot">Reboot Device<small id="lbl-reboot-s">Restart the ESP32</small></div>
        <div class="set-ctrl"><button class="btn-warn" onclick="rebootDevice()" id="btn-reboot">Reboot</button></div>
      </div>
    </div>

  </div>

  <div class="footer">HandRFID &mdash; Survival Hacking</div>
</div>

<script>
function showTab(id, el) {
  document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
  document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
  document.getElementById(id).classList.add('active');
  el.classList.add('active');
  if (id === 'settings') loadSettings();
}

function mkInput(val, name, type) {
  type = type || 'text';
  let v = (val !== undefined && val !== null) ? val : '';
  return '<input type="'+type+'" name="'+name+'" value="'+v+'" placeholder="'+name+'">';
}

function renderRow(type, d) {
  let tr = document.createElement('tr');
  if (type === 'qidi') {
    tr.innerHTML = '<td>'+mkInput(d.code,'code','number')+'</td>' +
      '<td>'+mkInput(d.mat,'mat')+'</td>' +
      '<td>'+mkInput(d.sku,'sku')+'</td>' +
      '<td>'+mkInput(d.emin,'emin','number')+'</td>' +
      '<td>'+mkInput(d.emax,'emax','number')+'</td>' +
      '<td>'+mkInput(d.bmin,'bmin','number')+'</td>' +
      '<td>'+mkInput(d.bmax,'bmax','number')+'</td>' +
      '<td><button class="btn-del" onclick="this.closest(\'tr\').remove();updCounts()">X</button></td>';
  } else if (type === 'anycubic') {
    tr.innerHTML = '<td>'+mkInput(d.mat,'mat')+'</td>' +
      '<td>'+mkInput(d.sku,'sku')+'</td>' +
      '<td>'+mkInput(d.emin,'emin','number')+'</td>' +
      '<td>'+mkInput(d.emax,'emax','number')+'</td>' +
      '<td>'+mkInput(d.bmin,'bmin','number')+'</td>' +
      '<td>'+mkInput(d.bmax,'bmax','number')+'</td>' +
      '<td><button class="btn-del" onclick="this.closest(\'tr\').remove();updCounts()">X</button></td>';
  }
  document.getElementById('tbl-'+type).appendChild(tr);
}

function addRow(type) {
  renderRow(type, {});
  updCounts();
}

function updCounts() {
  ['qidi','anycubic'].forEach(t => {
    let n = document.querySelectorAll('#tbl-'+t+' tr').length;
    let el = document.getElementById('cnt-'+t);
    if(el) el.textContent = n + ' items';
  });
}

function getRows(type) {
  let rows = [];
  document.querySelectorAll('#tbl-'+type+' tr').forEach(tr => {
    let d = {};
    tr.querySelectorAll('input').forEach(inp => { d[inp.name] = inp.value; });
    if (d.mat && d.mat.trim()) rows.push(d);
  });
  return rows;
}

function showStatus(msg, ok) {
  let s = document.getElementById('status');
  s.textContent = msg;
  s.className = 'status ' + (ok ? 'ok' : 'err');
  s.style.display = 'block';
  setTimeout(() => { s.style.display = 'none'; }, 4000);
}

function loadAll() {
  fetch('/api/materials').then(r => r.json()).then(data => {
    ['qidi','anycubic'].forEach(t => {
      document.getElementById('tbl-'+t).innerHTML = '';
      (data[t] || []).forEach(d => renderRow(t, d));
    });
    updCounts();
    let total = (data.qidi||[]).length+(data.anycubic||[]).length;
    showStatus('Loaded ' + total + ' materials', true);
  }).catch(e => showStatus('Load error: '+e, false));
}

function checkDuplicateCodes(type) {
  let rows = getRows(type);
  let codes = {};
  for (let r of rows) {
    let c = r.code;
    if (!c || c === '0' || c === '') continue;
    if (codes[c]) return 'Duplicate code ' + c + ' in ' + type.toUpperCase() + ': "' + codes[c] + '" and "' + r.mat + '"';
    codes[c] = r.mat;
  }
  return null;
}

function saveAll() {
  for (let t of ['qidi','anycubic']) {
    let err = checkDuplicateCodes(t);
    if (err) { showStatus('ERROR: ' + err, false); return; }
  }
  let data = { qidi: getRows('qidi'), anycubic: getRows('anycubic') };
  fetch('/api/materials', {
    method: 'POST', headers: {'Content-Type':'application/json'},
    body: JSON.stringify(data)
  }).then(r => r.json()).then(res => {
    showStatus(res.msg || 'Saved!', res.ok);
  }).catch(e => showStatus('Save error: '+e, false));
}

/* ---- i18n ---- */
const I18N = {
  0: { // English
    'tab-qidi':'QIDI','tab-anycubic':'Anycubic','tab-settings':'Settings',
    'btn-add-qidi':'+ Add Material','btn-add-ace':'+ Add Material',
    'btn-save-mat':'SAVE MATERIALS','btn-save-mat2':'SAVE MATERIALS',
    'btn-reload':'RELOAD','btn-reload2':'RELOAD',
    'th-code':'Code','th-mat-q':'Material','th-mat-a':'Material',
    'th-emin':'Ext Min','th-emax':'Ext Max','th-bmin':'Bed Min','th-bmax':'Bed Max',
    'th-emin2':'Ext Min','th-emax2':'Ext Max','th-bmin2':'Bed Min','th-bmax2':'Bed Max',
    'h-devinfo':'Device Info','lbl-fw':'Firmware','lbl-ip':'IP Address',
    'lbl-nfc':'NFC Module','lbl-heap':'Free Heap','lbl-up':'Uptime','lbl-rssi':'WiFi Signal',
    'h-general':'General',
    'lbl-lang':'Language','lbl-lang-s':'Interface language',
    'lbl-auto':'Auto Mode','lbl-auto-s':'Automatically read tags on contact',
    'h-display':'Display',
    'lbl-bl':'Backlight Brightness','lbl-bl-s':'0 = off, 255 = max',
    'lbl-rainbow':'Rainbow Title','lbl-rainbow-s':'Animated rainbow colors on "HandRFID"',
    'lbl-ss':'Auto Power Off','lbl-ss-s':'Seconds of inactivity before screen + backlight off (0 = never)',
    'lbl-dbg':'Debug Log Panel','lbl-dbg-s':'Show real-time debug log on main screen',
    'h-mfg':'Manufacturers','btn-add-mfg':'+ Add',
    'h-danger':'Danger Zone',
    'lbl-rwifi':'Reset WiFi','lbl-rwifi-s':'Clear credentials, device will reboot into setup portal','btn-rwifi':'Reset WiFi',
    'lbl-rmat':'Reset Materials','lbl-rmat-s':'Delete all custom materials and restore defaults','btn-rmat':'Reset Materials',
    'lbl-reboot':'Reboot Device','lbl-reboot-s':'Restart the ESP32','btn-reboot':'Reboot',
    'status-saved':'Settings saved!','status-err':'Error',
    'confirm-wifi':'Reset WiFi credentials? Device will reboot.','confirm-mat':'Delete ALL custom materials and restore defaults?'
  },
  1: { // Italiano
    'tab-qidi':'QIDI','tab-anycubic':'Anycubic','tab-settings':'Impostazioni',
    'btn-add-qidi':'+ Aggiungi Materiale','btn-add-ace':'+ Aggiungi Materiale',
    'btn-save-mat':'SALVA MATERIALI','btn-save-mat2':'SALVA MATERIALI',
    'btn-reload':'RICARICA','btn-reload2':'RICARICA',
    'th-code':'Codice','th-mat-q':'Materiale','th-mat-a':'Materiale',
    'th-emin':'Est Min','th-emax':'Est Max','th-bmin':'Piatto Min','th-bmax':'Piatto Max',
    'th-emin2':'Est Min','th-emax2':'Est Max','th-bmin2':'Piatto Min','th-bmax2':'Piatto Max',
    'h-devinfo':'Info Dispositivo','lbl-fw':'Firmware','lbl-ip':'Indirizzo IP',
    'lbl-nfc':'Modulo NFC','lbl-heap':'Heap Libero','lbl-up':'Uptime','lbl-rssi':'Segnale WiFi',
    'h-general':'Generale',
    'lbl-lang':'Lingua','lbl-lang-s':'Lingua interfaccia',
    'lbl-auto':'Modalità Auto','lbl-auto-s':'Legge automaticamente i tag al contatto',
    'h-display':'Display',
    'lbl-bl':'Luminosità Schermo','lbl-bl-s':'0 = spento, 255 = massimo',
    'lbl-rainbow':'Titolo Arcobaleno','lbl-rainbow-s':'Colori animati su "HandRFID"',
    'lbl-ss':'Auto Spegnimento','lbl-ss-s':'Secondi di inattività prima dello spegnimento (0 = mai)',
    'lbl-dbg':'Pannello Log Debug','lbl-dbg-s':'Mostra il log in tempo reale nella schermata principale',
    'h-mfg':'Produttori','btn-add-mfg':'+ Aggiungi',
    'h-danger':'Zona Pericolosa',
    'lbl-rwifi':'Reset WiFi','lbl-rwifi-s':'Cancella credenziali, il dispositivo si riavvia nel portale di configurazione','btn-rwifi':'Reset WiFi',
    'lbl-rmat':'Reset Materiali','lbl-rmat-s':'Cancella tutti i materiali personalizzati e ripristina i default','btn-rmat':'Reset Materiali',
    'lbl-reboot':'Riavvia Dispositivo','lbl-reboot-s':'Riavvia ESP32','btn-reboot':'Riavvia',
    'status-saved':'Impostazioni salvate!','status-err':'Errore',
    'confirm-wifi':'Resettare le credenziali WiFi? Il dispositivo si riavvierà.','confirm-mat':'Cancellare TUTTI i materiali personalizzati e ripristinare i default?'
  }
};

let _uiLang = 0;
function applyLang(lang) {
  _uiLang = lang;
  const d = I18N[lang] || I18N[0];
  for (const [id, text] of Object.entries(d)) {
    const el = document.getElementById(id);
    if (!el) continue;
    if (el.tagName === 'SMALL' || el.tagName === 'small') el.textContent = text;
    else if (el.tagName === 'BUTTON') el.textContent = text;
    else if (el.tagName === 'TH') el.textContent = text;
    else if (el.tagName === 'H3') el.textContent = text;
    else if (el.tagName === 'DIV' && el.classList.contains('tab')) el.textContent = text;
    else if (el.tagName === 'DIV' && el.classList.contains('lbl')) el.textContent = text;
    else el.textContent = text;
  }
  // Aggiorna placeholder
  const mfgInp = document.getElementById('mfg-new');
  if (mfgInp) mfgInp.placeholder = lang === 1 ? 'Nuovo produttore...' : 'New manufacturer...';
}

/* ---- Settings ---- */
let mfgList = [];

function loadSettings() {
  fetch('/api/settings').then(r => r.json()).then(s => {
    const lang = s.lang || 0;
    document.getElementById('s-lang').value = lang;
    applyLang(lang);
    document.getElementById('s-auto').checked = !!s.autoMode;
    document.getElementById('s-bl').value = s.backlight != null ? s.backlight : 255;
    document.getElementById('s-bl-val').textContent = s.backlight != null ? s.backlight : 255;
    document.getElementById('s-ss').value = s.ssTimeout != null ? s.ssTimeout : 120;
    document.getElementById('s-rainbow').checked = s.rainbowEnabled !== false;
    document.getElementById('s-dbgpanel').checked = !!s.showDebugPanel;
    // Device info
    document.getElementById('si-fw').textContent = s.firmware || '-';
    document.getElementById('si-ip').textContent = s.ip || '-';
    document.getElementById('si-nfc').textContent = s.nfcPresent ? 'PN532 OK' : 'Not found';
    document.getElementById('si-nfc').style.color = s.nfcPresent ? 'var(--accent2)' : 'var(--danger)';
    document.getElementById('si-heap').textContent = s.freeHeap ? (Math.round(s.freeHeap/1024) + ' KB') : '-';
    document.getElementById('si-up').textContent = fmtUptime(s.uptime || 0);
    document.getElementById('si-rssi').textContent = s.rssi ? (s.rssi + ' dBm') : '-';
    document.getElementById('fw-ver').textContent = s.firmware || 'Control Panel';
    // Manufacturers
    mfgList = s.manufacturers || [];
    renderMfg();
  }).catch(e => showStatus('Settings load error: '+e, false));
}

function saveSettings() {
  const lang = parseInt(document.getElementById('s-lang').value);
  applyLang(lang);
  let data = {
    lang: lang,
    autoMode: document.getElementById('s-auto').checked,
    backlight: parseInt(document.getElementById('s-bl').value),
    ssTimeout: parseInt(document.getElementById('s-ss').value),
    rainbowEnabled: document.getElementById('s-rainbow').checked,
    showDebugPanel: document.getElementById('s-dbgpanel').checked,
    manufacturers: mfgList
  };
  fetch('/api/settings', {
    method: 'POST', headers: {'Content-Type':'application/json'},
    body: JSON.stringify(data)
  }).then(r => r.json()).then(res => {
    const d = I18N[_uiLang] || I18N[0];
    showStatus(res.ok ? d['status-saved'] : (res.msg || d['status-err']), res.ok);
  }).catch(e => { const d=I18N[_uiLang]||I18N[0]; showStatus(d['status-err']+': '+e, false); });
}

function renderMfg() {
  let c = document.getElementById('mfg-chips');
  c.innerHTML = '';
  mfgList.forEach((m, i) => {
    c.innerHTML += '<div class="mfg-chip"><span>'+m+'</span><span class="mfg-del" onclick="delMfg('+i+')">&times;</span></div>';
  });
}

function addMfg() {
  let inp = document.getElementById('mfg-new');
  let v = inp.value.trim();
  if (!v) return;
  if (mfgList.length >= 20) { showStatus('Max 20 manufacturers', false); return; }
  if (mfgList.includes(v)) { showStatus('Already exists', false); return; }
  mfgList.push(v);
  inp.value = '';
  renderMfg();
  saveSettings();
}

function delMfg(i) {
  mfgList.splice(i, 1);
  renderMfg();
  saveSettings();
}

function fmtUptime(sec) {
  let d = Math.floor(sec/86400), h = Math.floor((sec%86400)/3600), m = Math.floor((sec%3600)/60);
  if (d > 0) return d+'d '+h+'h '+m+'m';
  if (h > 0) return h+'h '+m+'m';
  return m+'m';
}

/* Auto-save: ogni controllo settings chiama saveSettings() al cambio */
let _ssTimer = null;
function autoSave() {
  // Debounce per slider (evita flood di richieste)
  clearTimeout(_ssTimer);
  _ssTimer = setTimeout(saveSettings, 300);
}

document.getElementById('s-lang').addEventListener('change', function() {
  applyLang(parseInt(this.value));
  autoSave();
});
document.getElementById('s-auto').addEventListener('change', autoSave);
document.getElementById('s-rainbow').addEventListener('change', autoSave);
document.getElementById('s-ss').addEventListener('change', autoSave);
document.getElementById('s-dbgpanel').addEventListener('change', autoSave);
document.getElementById('s-bl').addEventListener('input', function() {
  document.getElementById('s-bl-val').textContent = this.value;
  autoSave();
});

/* Danger zone */
function resetWifi() {
  const d = I18N[_uiLang] || I18N[0];
  if (!confirm(d['confirm-wifi'])) return;
  fetch('/api/reset-wifi', {method:'POST'}).then(() => {
    showStatus(d['status-saved'], true);
  }).catch(e => showStatus(d['status-err']+': '+e, false));
}

function resetMaterials() {
  const d = I18N[_uiLang] || I18N[0];
  if (!confirm(d['confirm-mat'])) return;
  fetch('/api/reset-materials', {method:'POST'}).then(r => r.json()).then(res => {
    showStatus(res.msg || 'Materials reset!', res.ok);
    loadAll();
  }).catch(e => showStatus('Error: '+e, false));
}

function rebootDevice() {
  if (!confirm('Reboot the device?')) return;
  fetch('/api/reboot', {method:'POST'}).then(() => {
    showStatus('Rebooting...', true);
  }).catch(e => showStatus('Error: '+e, false));
}

/* Init */
loadAll();
loadSettings();
</script>
</body>
</html>
)rawliteral";
