#pragma once

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en" data-theme="light">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Growatt Inverter</title>
<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@picocss/pico@2/css/pico.min.css"
  onerror="document.documentElement.classList.add('no-cdn')">
<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/4.2.1/chart.umd.min.js"
  integrity="sha512-GCiwmzA0bNGVsp1otzTJ4LWQT2jjGJENLGyLlerlzckNI30moi2EQT0AfRI7fLYYYDKR+7hnuh35r3y1uJzugw=="
  crossorigin="anonymous" referrerpolicy="no-referrer"></script>
<style>
  /* Tab bar */
  .tabs{display:flex;gap:0;border-bottom:2px solid #ccc;margin-bottom:1rem}
  .tabs button{background:none;border:none;padding:.6rem 1.2rem;cursor:pointer;
    font-size:.95rem;border-bottom:3px solid transparent;color:#555}
  .tabs button.active{border-bottom-color:#1a73e8;color:#1a73e8;font-weight:600}
  .tabs button:hover{color:#1a73e8}
  .tab-content{display:none}
  .tab-content.active{display:block}
  /* Status badges */
  .badge{display:inline-block;padding:.15em .5em;border-radius:4px;font-size:.85rem;font-weight:600;color:#fff}
  .badge-ok{background:#2e7d32}
  .badge-warn{background:#ed6c02}
  .badge-err{background:#d32f2f}
  .badge-off{background:#757575}
  /* Config form */
  .cfg-msg{padding:.5rem;border-radius:4px;margin-bottom:.5rem;display:none}
  .cfg-msg.ok{display:block;background:#e8f5e9;color:#2e7d32}
  .cfg-msg.err{display:block;background:#ffebee;color:#c62828}
  /* Debug iframe */
  .debug-frame{width:100%;height:500px;border:1px solid #ccc;border-radius:4px}
  /* Fallback CSS when CDN is offline */
  .no-cdn{font-family:system-ui,-apple-system,sans-serif;max-width:900px;margin:0 auto;padding:1rem}
  .no-cdn h1{font-size:1.6rem}
  .no-cdn table{width:100%;border-collapse:collapse;margin:.5rem 0}
  .no-cdn th,.no-cdn td{text-align:left;padding:.4rem .6rem;border-bottom:1px solid #ddd}
  .no-cdn input,.no-cdn select{padding:.3rem .5rem;border:1px solid #aaa;border-radius:3px;width:100%;box-sizing:border-box}
  .no-cdn button[type=submit]{background:#1a73e8;color:#fff;border:none;padding:.5rem 1.5rem;border-radius:4px;cursor:pointer}
  .no-cdn details{border:1px solid #ddd;border-radius:4px;padding:.5rem;margin:.5rem 0}
  .no-cdn summary{cursor:pointer;font-weight:600}
</style>
</head>
<body>
<main class="container">
<h1>Growatt Inverter</h1>

<nav class="tabs">
  <button class="active" data-tab="dash">Dashboard</button>
  <button data-tab="cfg">Config</button>
  <button data-tab="cloud">Cloud</button>
  <button data-tab="dbg">Debug Log</button>
  <button data-tab="sys">System</button>
</nav>

<!-- ===== Dashboard Tab ===== -->
<section id="tab-dash" class="tab-content active">
  <div><canvas id="powerChart"></canvas></div>
  <div id="DataContainer"></div>
</section>

<!-- ===== Config Tab ===== -->
<section id="tab-cfg" class="tab-content">
  <div id="cfgMsg" class="cfg-msg"></div>
  <form id="cfgForm">
    <details open>
      <summary>Network</summary>
      <label>Hostname <input name="hostname" id="c_hostname"></label>
      <label>Static IP <input name="static_ip" id="c_static_ip" placeholder="leave blank for DHCP"></label>
      <label>Netmask <input name="static_netmask" id="c_static_netmask"></label>
      <label>Gateway <input name="static_gateway" id="c_static_gateway"></label>
      <label>DNS <input name="static_dns" id="c_static_dns"></label>
    </details>
    <details>
      <summary>MQTT</summary>
      <label>Server <input name="mqtt_server" id="c_mqtt_server" placeholder="leave blank to disable"></label>
      <label>Port <input name="mqtt_port" id="c_mqtt_port"></label>
      <label>Topic <input name="mqtt_topic" id="c_mqtt_topic"></label>
      <label>Username <input name="mqtt_user" id="c_mqtt_user"></label>
      <label>Password <input name="mqtt_pwd" type="password" id="c_mqtt_pwd" placeholder="unchanged if blank"></label>
      <small id="c_mqtt_pwd_hint"></small>
    </details>
    <details>
      <summary>Growatt Cloud</summary>
      <label>Datalogger Serial <input name="cloud_serial" id="c_cloud_serial" placeholder="from stick label"></label>
      <label>Server <input name="cloud_server" id="c_cloud_server" placeholder="server.growatt.com"></label>
    </details>
    <details>
      <summary>Authentication</summary>
      <label>Username <input name="auth_user" id="c_auth_user" placeholder="blank = no auth"></label>
      <label>Password <input name="auth_pass" type="password" id="c_auth_pass" placeholder="unchanged if blank"></label>
      <small id="c_auth_pwd_hint"></small>
    </details>
    <details>
      <summary>Advanced</summary>
      <label>Syslog IP <input name="syslog_ip" id="c_syslog_ip" placeholder="leave blank for none"></label>
    </details>
    <label><input type="checkbox" name="restart" value="1"> Restart after saving</label>
    <button type="submit">Save Configuration</button>
  </form>
</section>

<!-- ===== Cloud Tab ===== -->
<section id="tab-cloud" class="tab-content">
  <table id="cloudTable" role="grid">
    <tbody>
      <tr><th>Status</th><td id="cl_state">--</td></tr>
      <tr><th>Serial</th><td id="cl_serial">--</td></tr>
      <tr><th>Server</th><td id="cl_server">--</td></tr>
      <tr><th>Packets Sent</th><td id="cl_sent">--</td></tr>
      <tr><th>Packets Received</th><td id="cl_recv">--</td></tr>
      <tr><th>Reconnects</th><td id="cl_recon">--</td></tr>
      <tr><th>Last Send</th><td id="cl_last">--</td></tr>
    </tbody>
  </table>
</section>

<!-- ===== Debug Log Tab ===== -->
<section id="tab-dbg" class="tab-content">
  <p>WebSerial debug stream (port 8080):</p>
  <iframe id="debugFrame" class="debug-frame"></iframe>
</section>

<!-- ===== System Tab ===== -->
<section id="tab-sys" class="tab-content">
  <table id="sysTable" role="grid">
    <tbody>
      <tr><th>Hostname</th><td id="s_host">--</td></tr>
      <tr><th>IP Address</th><td id="s_ip">--</td></tr>
      <tr><th>Gateway</th><td id="s_gw">--</td></tr>
      <tr><th>Netmask</th><td id="s_mask">--</td></tr>
      <tr><th>DNS</th><td id="s_dns">--</td></tr>
      <tr><th>WiFi SSID</th><td id="s_ssid">--</td></tr>
      <tr><th>WiFi RSSI</th><td id="s_rssi">--</td></tr>
      <tr><th>MAC Address</th><td id="s_mac">--</td></tr>
      <tr><th>Free Heap</th><td id="s_heap">--</td></tr>
      <tr><th>Uptime</th><td id="s_up">--</td></tr>
      <tr><th>Stick Type</th><td id="s_stick">--</td></tr>
    </tbody>
  </table>
  <h4>Quick Links</h4>
  <p>
    <a href="./status" role="button" class="outline">JSON</a>
    <a href="./uiStatus" role="button" class="outline">UI JSON</a>
    <a href="./metrics" role="button" class="outline">Prometheus</a>
    <a href="./postCommunicationModbus" role="button" class="outline secondary">RW Modbus</a>
  </p>
  <h4>Maintenance</h4>
  <p>
    <a href="./update" role="button" class="outline">Firmware Update</a>
    <a href="#" role="button" class="outline contrast" id="btnAp"
       onclick="if(confirm('Starting config AP will disconnect you. Continue?'))location='./startAp';return false;">Start Config AP</a>
    <a href="#" role="button" class="outline contrast" id="btnReboot"
       onclick="if(confirm('Reboot the WiFi stick?'))location='./reboot';return false;">Reboot</a>
  </p>
</section>

<script>
/* ---- Tab switching ---- */
document.querySelectorAll('.tabs button').forEach(function(btn){
  btn.addEventListener('click', function(){
    document.querySelectorAll('.tabs button').forEach(function(b){b.classList.remove('active')});
    document.querySelectorAll('.tab-content').forEach(function(t){t.classList.remove('active')});
    btn.classList.add('active');
    var target = document.getElementById('tab-' + btn.getAttribute('data-tab'));
    target.classList.add('active');
    /* Lazy-load debug iframe */
    if(btn.getAttribute('data-tab')==='dbg'){
      var fr = document.getElementById('debugFrame');
      if(!fr.src || fr.src===''){
        fr.src = 'http://' + location.hostname + ':8080/';
      }
    }
    /* Fetch config when switching to config tab */
    if(btn.getAttribute('data-tab')==='cfg') loadConfig();
    /* Fetch system status */
    if(btn.getAttribute('data-tab')==='sys') fetchSystem();
  });
});

/* ---- Dashboard: Chart.js + uiStatus polling (preserved from original) ---- */
var initialised = false;
var powerchartelement = document.getElementById('powerChart');
var CHART_COLORS = {
  red:'rgb(255,99,132)', orange:'rgb(255,159,64)', yellow:'rgb(255,205,86)',
  green:'rgb(75,192,192)', blue:'rgb(54,162,235)', purple:'rgb(153,102,255)', grey:'rgb(201,203,207)'
};
var NAMED_COLORS = [CHART_COLORS.red, CHART_COLORS.orange, CHART_COLORS.yellow,
  CHART_COLORS.green, CHART_COLORS.blue, CHART_COLORS.purple, CHART_COLORS.grey];
function namedColor(i){return NAMED_COLORS[i % NAMED_COLORS.length]}
var powerchartData = {labels:[], datasets:[]};
var powerchart = new Chart(powerchartelement, {
  type:'line', data:powerchartData,
  options:{scales:{y:{beginAtZero:true}}}
});
setInterval(function(){
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function(){
    if(this.readyState==4 && this.status==200){
      var obj = JSON.parse(this.responseText);
      var date = new Date();
      powerchartData.labels.push(date.getHours()+":"+date.getMinutes()+":"+date.getSeconds());
      if(initialised==false){
        initialised = true;
        var container = document.getElementById("DataContainer");
        container.innerHTML = "";
        for(var key in obj){
          if(obj[key][2]==true){
            var nd = {label:key, data:[obj[key][0]], fill:false,
              borderColor:namedColor(powerchart.data.datasets.length), tension:0.1};
            powerchartData.datasets.push(nd);
            powerchart.update();
          }
          var el = document.createElement("p");
          el.innerHTML = '<a href="/value/'+key+'">'+key+'</a>: '+obj[key][0]+'\u202F'+obj[key][1];
          el.setAttribute("id", key);
          container.appendChild(el);
        }
      } else {
        for(var key in obj){
          if(obj[key][2]==true){
            for(var d in powerchartData.datasets){
              if(powerchartData.datasets[d].label==key){
                powerchartData.datasets[d].data.push(obj[key][0]);
              }
            }
          }
          var el = document.getElementById(key);
          if(el) el.innerHTML = '<a href="/value/'+key+'">'+key+'</a>: '+obj[key][0]+'\u202F'+obj[key][1];
          powerchart.update();
        }
      }
    }
  };
  xhttp.open("GET","./uiStatus",true);
  xhttp.send();
}, 5000);

/* ---- Config tab ---- */
function loadConfig(){
  fetch('./config').then(function(r){return r.json()}).then(function(d){
    var fields = ['hostname','static_ip','static_netmask','static_gateway','static_dns',
      'mqtt_server','mqtt_port','mqtt_topic','mqtt_user','cloud_serial','cloud_server',
      'auth_user','syslog_ip'];
    for(var i=0;i<fields.length;i++){
      var el = document.getElementById('c_'+fields[i]);
      if(el && d[fields[i]]!==undefined) el.value = d[fields[i]];
    }
    var mh = document.getElementById('c_mqtt_pwd_hint');
    if(mh) mh.textContent = d.mqtt_pwd_set ? 'Password is set. Leave blank to keep.' : 'No password set.';
    var ah = document.getElementById('c_auth_pwd_hint');
    if(ah) ah.textContent = d.auth_pwd_set ? 'Password is set. Leave blank to keep.' : 'No password set.';
  }).catch(function(e){
    showCfgMsg('Failed to load config: '+e, true);
  });
}

document.getElementById('cfgForm').addEventListener('submit', function(ev){
  ev.preventDefault();
  var msg = document.getElementById('cfgMsg');
  msg.className = 'cfg-msg';
  msg.textContent = 'Saving...';
  msg.style.display = 'block';
  fetch('./saveConfig', {
    method:'POST',
    headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body: new URLSearchParams(new FormData(this))
  }).then(function(r){return r.json()}).then(function(d){
    if(d.ok){
      showCfgMsg(d.restart ? 'Saved! Restarting device...' : 'Configuration saved.', false);
    } else {
      showCfgMsg('Save failed.', true);
    }
  }).catch(function(e){
    showCfgMsg('Error: '+e, true);
  });
});

function showCfgMsg(text, isErr){
  var el = document.getElementById('cfgMsg');
  el.className = 'cfg-msg ' + (isErr ? 'err' : 'ok');
  el.textContent = text;
}

/* ---- Cloud tab polling ---- */
var cloudInterval = null;
function fetchCloud(){
  fetch('./cloudStatus').then(function(r){return r.json()}).then(function(d){
    if(d.supported===false){
      document.getElementById('cl_state').innerHTML = '<span class="badge badge-off">Not compiled</span>';
      return;
    }
    if(!d.enabled){
      document.getElementById('cl_state').innerHTML = '<span class="badge badge-off">Disabled</span>';
      document.getElementById('cl_serial').textContent = 'Not configured';
      return;
    }
    var sc = d.stateCode || 0;
    var bc = sc >= 3 ? 'badge-ok' : (sc >= 1 ? 'badge-warn' : 'badge-err');
    if(sc === 5) bc = 'badge-err';
    document.getElementById('cl_state').innerHTML = '<span class="badge '+bc+'">'+d.state+'</span>';
    document.getElementById('cl_serial').textContent = d.serial || '--';
    document.getElementById('cl_server').textContent = d.server || '--';
    document.getElementById('cl_sent').textContent = d.packetsSent;
    document.getElementById('cl_recv').textContent = d.packetsRecv;
    document.getElementById('cl_recon').textContent = d.reconnects;
    document.getElementById('cl_last').textContent = d.lastSendAgo >= 0 ? d.lastSendAgo+'s ago' : 'never';
  }).catch(function(){});
}

/* Start/stop cloud polling based on tab visibility */
document.querySelectorAll('.tabs button').forEach(function(btn){
  btn.addEventListener('click', function(){
    if(btn.getAttribute('data-tab')==='cloud'){
      fetchCloud();
      if(!cloudInterval) cloudInterval = setInterval(fetchCloud, 10000);
    } else {
      if(cloudInterval){clearInterval(cloudInterval); cloudInterval=null;}
    }
  });
});

/* ---- System tab ---- */
function fetchSystem(){
  fetch('./systemStatus').then(function(r){return r.json()}).then(function(d){
    document.getElementById('s_host').textContent = d.hostname || '--';
    document.getElementById('s_ip').textContent = d.ip || '--';
    document.getElementById('s_gw').textContent = d.gateway || '--';
    document.getElementById('s_mask').textContent = d.netmask || '--';
    document.getElementById('s_dns').textContent = d.dns || '--';
    document.getElementById('s_ssid').textContent = d.ssid || '--';
    document.getElementById('s_mac').textContent = d.mac || '--';
    document.getElementById('s_rssi').textContent = (d.rssi || 0) + ' dBm';
    document.getElementById('s_heap').textContent = d.heap ? (d.heap + ' bytes') : '--';
    var up = d.uptime || 0;
    var h = Math.floor(up/3600); var m = Math.floor((up - h*3600)/60); var s = up - h*3600 - m*60;
    document.getElementById('s_up').textContent = h+'h '+m+'m '+s+'s';
    document.getElementById('s_stick').textContent = d.stickType || '--';
  }).catch(function(){});
}
</script>
</main>
</body>
</html>
)rawliteral";

const char SendPostSite_page[] PROGMEM = R"=====(
<!DOCTYPE HTML><html>
<!-- Rui Santos - Complete project details at https://RandomNerdTutorials.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files.
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software. -->
<head>
  <meta charset='utf-8'>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Growatt Inverter</title>
</head>
<body>
  <h2>Growatt Post Communication Modbus</h2>
  <form action="/postCommunicationModbus_p" method="POST">
    <input type="text" name="reg" placeholder="Register ID"></br>
    <input type="text" name="val" placeholder="Input Value (16bit only!)"></br>
    <select name="type">
      <option value="16b" selected>16b</option>
      <option value="32b">32b</option>
    </select></br>
    <select name="operation">
      <option value="R" selected>Read</option>
      <option value="W">Write</option>
    </select></br>
    <select name="registerType">
      <option value="I" selected>Input Register</option>
      <option value="H">Holding Register</option>
    </select></br>
    <input type="submit" value="go">
  </form>
  <a href=".">back</a>
</body>
</html>
)=====";
