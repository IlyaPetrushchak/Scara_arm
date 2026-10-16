#pragma once
#include <Arduino.h>

// SCARA web panel. Serve with: server.send_P(200, "text/html", INDEX_HTML);
// Без зовнішніх ресурсів (CDN/шрифти) — працює у Wi-Fi SoftAP без інтернету.

const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="uk">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#eceff3">
<link rel="icon" href="data:,">
<title>SCARA · панель керування</title>
<style>
:root{
  --paper:#e9edf2; --surface:#fff; --line:#d3dae4; --grid:#e3e8ef;
  --ink:#1a2431; --mute:#65748a;
  --orange:#f0631a; --teal:#0d8f8a; --red:#d7263d; --green:#1e9c69;
  --ws:rgba(240,99,26,.06); --wsline:rgba(240,99,26,.45);
}
*{box-sizing:border-box;margin:0}
[hidden]{display:none!important}
html{-webkit-text-size-adjust:100%}
body{font:15px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,"Helvetica Neue",Arial,sans-serif;
  background:var(--paper);color:var(--ink);min-height:100vh;font-variant-numeric:tabular-nums;
  padding-bottom:env(safe-area-inset-bottom)}
button{font:inherit;color:inherit;cursor:pointer}
button:focus-visible,input:focus-visible{outline:2px solid var(--teal);outline-offset:2px}
button:disabled{opacity:.45;cursor:not-allowed}

/* header */
.top{position:sticky;top:0;z-index:20;display:flex;align-items:center;gap:14px;
  padding:10px 18px;background:var(--surface);border-bottom:1px solid var(--line)}
.brand{display:flex;flex-direction:column;line-height:1.1;margin-right:auto}
.brand b{font-size:19px;font-weight:700;letter-spacing:.01em}
.brand span{font-size:12px;color:var(--mute)}
.pills{display:flex;gap:8px;flex-wrap:wrap;justify-content:flex-end}
.pill{display:inline-flex;align-items:center;gap:7px;height:28px;padding:0 11px;border-radius:999px;
  background:var(--paper);font-size:13px;font-weight:600;white-space:nowrap}
.pill .dot{width:8px;height:8px;border-radius:50%;background:var(--mute)}
.pill.ok .dot{background:var(--green)}
.pill.bad .dot{background:var(--red)}
.pill.run{background:#fde6d9;color:#a8400a}
.stop{min-height:44px;padding:0 26px;border:0;border-radius:999px;background:var(--red);color:#fff;
  font-weight:700;font-size:16px;box-shadow:0 2px 0 #9d1a2b}
.stop:active{transform:translateY(1px);box-shadow:0 1px 0 #9d1a2b}

.banner{padding:11px 18px;font-weight:600;background:var(--red);color:#fff}
.banner.off{background:#3a4658}

/* layout */
main{display:grid;gap:18px;padding:18px;max-width:1240px;margin:0 auto}
@media(min-width:960px){
  main{grid-template-columns:minmax(0,1.1fr) minmax(360px,.9fr);align-items:start}
  .left{position:sticky;top:78px}
}
.panel{background:var(--surface);border:1px solid var(--line);border-radius:14px;overflow:hidden}
.sec{padding:18px 20px}
.sec+.sec{border-top:1px solid var(--line)}
h2{font-size:14px;font-weight:700;margin-bottom:12px}
.sub{font-size:12.5px;color:var(--mute)}

/* canvas */
.cvhead{display:flex;justify-content:space-between;align-items:baseline;gap:10px;padding:14px 20px 0;flex-wrap:wrap}
.legend{display:flex;gap:14px;font-size:12.5px;color:var(--mute)}
.legend i{display:inline-block;width:18px;height:0;border-top:3px solid var(--orange);vertical-align:middle;margin-right:6px;border-radius:2px}
.legend i.t{border-top:2px dashed var(--teal)}
.cvwrap{padding:6px 12px 0}
#cv{display:block;width:100%;max-width:620px;aspect-ratio:1/1;margin:0 auto;touch-action:none;user-select:none;-webkit-user-select:none}
.hint{padding:0 20px 12px;text-align:center}
.ro{display:grid;grid-template-columns:repeat(5,1fr);border-top:1px solid var(--line)}
.ro div{padding:11px 12px;border-left:1px solid var(--line);min-width:0}
.ro div:first-child{border-left:0}
.ro small{display:block;font-size:12px;color:var(--mute)}
.ro b{font-size:clamp(16px,2.4vw,22px);font-weight:700}
.ro i{font-style:normal;font-size:12px;color:var(--mute);margin-left:2px}

/* controls */
.row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}
.btn{min-height:44px;padding:0 18px;border:1px solid var(--line);border-radius:10px;background:#fff;font-weight:600}
.btn:hover{border-color:#b7c1cf}
.btn:active{background:var(--paper)}
.btn.pri{background:var(--ink);border-color:var(--ink);color:#fff}
.btn.acc{background:var(--orange);border-color:var(--orange);color:#fff}
.btn.grow{flex:1}
.seg{display:inline-flex;background:var(--paper);border-radius:11px;padding:3px}
.seg button{border:0;background:transparent;min-height:38px;padding:0 20px;border-radius:9px;font-weight:600;color:var(--mute)}
.seg button.on{background:#fff;color:var(--ink);box-shadow:0 1px 3px rgba(20,30,50,.18)}
.count{margin-left:auto;font-weight:700}
.count small{font-weight:400;color:var(--mute)}

.chips{display:flex;gap:6px;align-items:center;flex-wrap:wrap;margin-bottom:8px}
.chips button{min-height:34px;padding:0 13px;border:1px solid var(--line);border-radius:999px;background:#fff;font-size:13.5px;font-weight:600}
.chips button.on{background:var(--teal);border-color:var(--teal);color:#fff}

.ax{padding:9px 0}
.ax-h{display:flex;justify-content:space-between;align-items:baseline;margin-bottom:5px;font-weight:600}
.ax-h small{font-weight:400;color:var(--mute);margin-left:6px}
.ax-h .v{font-size:17px}
.ax-h i{font-style:normal;color:var(--mute);margin-left:3px;font-size:13px}
.ax-c{display:grid;grid-template-columns:46px 1fr 46px;gap:10px;align-items:center}
.ax-c.g{grid-template-columns:auto 1fr auto}
.jb{height:46px;border:1px solid var(--line);border-radius:12px;background:#fff;font-size:24px;line-height:1;font-weight:500;
  touch-action:none;user-select:none;-webkit-user-select:none;-webkit-touch-callout:none}
.jb.on{background:var(--orange);border-color:var(--orange);color:#fff}
.sm{height:46px;padding:0 14px;border:1px solid var(--line);border-radius:12px;background:#fff;font-weight:600;font-size:14px}

input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:30px;background:transparent}
input[type=range]::-webkit-slider-runnable-track{height:6px;border-radius:3px;background:#cfd6e0}
input[type=range]::-moz-range-track{height:6px;border-radius:3px;background:#cfd6e0}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:26px;height:26px;margin-top:-10px;border-radius:50%;
  background:var(--orange);border:3px solid #fff;box-shadow:0 1px 4px rgba(20,30,50,.4)}
input[type=range]::-moz-range-thumb{width:20px;height:20px;border-radius:50%;background:var(--orange);border:3px solid #fff;box-shadow:0 1px 4px rgba(20,30,50,.4)}

.cols{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}
.cols .btn{padding:0}
.colnote{margin-top:10px;display:flex;justify-content:space-between;align-items:center;gap:10px}

/* locked states */
body.auto .man,body.estop .man,body.offline .man,
body.estop .ctl,body.offline .ctl{opacity:.42;pointer-events:none}
body.estop #seg,body.offline #seg{opacity:.42;pointer-events:none}

#toast{position:fixed;left:50%;bottom:calc(20px + env(safe-area-inset-bottom));transform:translate(-50%,20px);
  background:var(--ink);color:#fff;padding:10px 18px;border-radius:10px;font-weight:600;font-size:14px;
  opacity:0;pointer-events:none;transition:opacity .18s,transform .18s;max-width:90vw;text-align:center;z-index:50}
#toast.show{opacity:1;transform:translate(-50%,0)}

@media(max-width:560px){
  .top{padding:8px 12px;gap:8px;flex-wrap:wrap}
  .stop{padding:0 20px}
  main{padding:12px;gap:12px}
  .sec{padding:16px}
  .pill{padding:0 9px}
}
@media(prefers-reduced-motion:reduce){#toast{transition:none}}
</style>
</head>
<body class="offline">

<header class="top">
  <div class="brand"><b>SCARA</b><span>RoboArm-WiFi</span></div>
  <div class="pills">
    <span class="pill" id="pConn"><i class="dot"></i><span>Підключення…</span></span>
    <span class="pill" id="pMode">Ручний</span>
    <span class="pill run" id="pRun" hidden>Рух</span>
  </div>
  <button class="stop" id="bStop" title="Esc">Стоп</button>
</header>

<div class="banner" id="banEstop" hidden>Аварійна зупинка. Відпустіть кнопку E-STOP і перезапустіть контролер — доти керування вимкнене.</div>
<div class="banner off" id="banOff" hidden>Немає зв’язку з роботом. Перевірте, що пристрій підключений до Wi-Fi «RoboArm-WiFi».</div>

<main>
  <section class="left">
    <div class="panel">
      <div class="cvhead">
        <h2>Вид зверху</h2>
        <div class="legend"><span><i></i>Фактично</span><span><i class="t"></i>Ціль</span></div>
      </div>
      <div class="cvwrap"><canvas id="cv"></canvas></div>
      <p class="sub hint" id="padHint">Потягніть кінчик інструмента, щоб їхати в точку на полі.</p>
      <div class="ro">
        <div><small>Плече 1</small><b id="rA1">—</b><i>°</i></div>
        <div><small>Плече 2</small><b id="rA2">—</b><i>°</i></div>
        <div><small>Z</small><b id="rZ">—</b><i>мм</i></div>
        <div><small>X</small><b id="rX">—</b><i>мм</i></div>
        <div><small>Y</small><b id="rY">—</b><i>мм</i></div>
      </div>
    </div>
  </section>

  <section class="right panel">
    <div class="sec ctl">
      <h2>Режим і програма</h2>
      <div class="row">
        <div class="seg" id="seg">
          <button data-m="MANUAL" class="on">Ручний</button>
          <button data-m="AUTO">Авто</button>
        </div>
        <div class="count"><span id="pts">0</span><small> / <span id="ptsMax">20</span> точок</small></div>
      </div>
      <div class="row" style="margin-top:12px" id="grpManual">
        <button class="btn pri grow" id="bTeach">Зберегти точку</button>
        <button class="btn" id="bClear">Очистити</button>
      </div>
      <div id="grpAuto" hidden style="margin-top:12px">
        <div class="row">
          <button class="btn acc grow" id="bAuto">Старт</button>
        </div>
        <p class="sub" style="margin-top:8px" id="autoTxt">Програма виконується по колу.</p>
      </div>
    </div>

    <div class="sec man">
      <h2>Осі</h2>
      <div class="chips" id="steps">
        <span class="sub">Кнопки ± :</span>
        <button data-s="0" class="on">тримати</button>
        <button data-s="1">1</button>
        <button data-s="5">5</button>
        <button data-s="10">10</button>
      </div>

      <div class="ax" data-ax="a1">
        <div class="ax-h"><span>Плече 1</span><span><b class="v">0.0</b><i>°</i></span></div>
        <div class="ax-c"><button class="jb" data-dir="-1" aria-label="Плече 1 мінус">−</button><input type="range" step="0.1" value="0" aria-label="Плече 1"><button class="jb" data-dir="1" aria-label="Плече 1 плюс">+</button></div>
      </div>
      <div class="ax" data-ax="a2">
        <div class="ax-h"><span>Плече 2</span><span><b class="v">0.0</b><i>°</i></span></div>
        <div class="ax-c"><button class="jb" data-dir="-1" aria-label="Плече 2 мінус">−</button><input type="range" step="0.1" value="0" aria-label="Плече 2"><button class="jb" data-dir="1" aria-label="Плече 2 плюс">+</button></div>
      </div>
      <div class="ax" data-ax="z">
        <div class="ax-h"><span>Підйом Z<small>0 — угорі</small></span><span><b class="v">0.0</b><i>мм</i></span></div>
        <div class="ax-c"><button class="jb" data-dir="-1" aria-label="Z вгору">−</button><input type="range" step="0.1" value="0" aria-label="Z"><button class="jb" data-dir="1" aria-label="Z вниз">+</button></div>
      </div>

      <div class="row" style="margin-top:8px">
        <button class="btn grow" id="bHome">Їхати в HOME</button>
      </div>
    </div>

    <div class="sec man">
      <h2>Інструмент</h2>
      <div class="ax" data-ax="rot">
        <div class="ax-h"><span>Поворот кисті</span><span><b class="v">0</b><i>°</i></span></div>
        <div class="ax-c"><button class="jb" data-dir="-1" aria-label="Поворот мінус">−</button><input type="range" step="1" value="90" aria-label="Поворот кисті"><button class="jb" data-dir="1" aria-label="Поворот плюс">+</button></div>
      </div>
      <div class="ax" data-ax="g">
        <div class="ax-h"><span>Схват</span><span><b class="v">0</b><i>°</i></span></div>
        <div class="ax-c g"><button class="sm" data-g="90">Відкрити</button><input type="range" step="1" value="90" aria-label="Схват"><button class="sm" data-g="180">Закрити</button></div>
      </div>
    </div>

    <div class="sec man">
      <h2>Колони</h2>
      <div class="cols">
        <button class="btn" data-col="1">1</button>
        <button class="btn" data-col="2">2</button>
        <button class="btn" data-col="3">3</button>
        <button class="btn pri" data-col="all">Усі</button>
      </div>
      <div class="colnote">
        <span class="sub" id="colTxt">Над колонкою, вниз, пауза, вгору.</span>
        <button class="btn" id="bColStop" hidden>Зупинити цикл</button>
      </div>
    </div>
  </section>
</main>

<div id="toast" role="status" aria-live="polite"></div>

<script>
(function(){
'use strict';
var $=function(s,r){return (r||document).querySelector(s)};
var $$=function(s,r){return Array.prototype.slice.call((r||document).querySelectorAll(s))};

/* Геометрія й ліміти — мають збігатися з прошивкою */
var L1=91.61, L2=97.528, RMIN=45, RMAX=185;
var LIM={a1:[-115,115],a2:[-138,138],z:[0,300],rot:[0,180],g:[90,180]};
var KEY={a1:'l1',a2:'l2',z:'z',rot:'rot',g:'g'};
var COLS=[[-75,110],[0,135],[75,110]];
var AUTO_TXT=['Очікує старту','Їде до точки','Пауза на точці','Призупинено','Програму завершено','Помилка'];

var S=null, online=false, everOnline=false, fails=0;
var step=0, jogging=false, localAuto=0;
var dragging={}, lastUser={};
var epoch=0, chain=Promise.resolve();

function f(v,d){ return (v==null||isNaN(v))?'—':Number(v).toFixed(d==null?1:d); }
function locked(){ return !online||!S||S.estop||S.mode==='AUTO'; }

/* ---------- toast ---------- */
var toastT=0;
function toast(m){
  var t=$('#toast'); t.textContent=m; t.classList.add('show');
  clearTimeout(toastT); toastT=setTimeout(function(){t.classList.remove('show')},2600);
}

/* ---------- мережа: одна черга, щоб команди не перегонялися ---------- */
function raw(path,ms){
  var c=new AbortController(), t=setTimeout(function(){c.abort()},ms);
  return fetch(path,{signal:c.signal,cache:'no-store'}).then(function(r){
    clearTimeout(t);
    return r.json().catch(function(){return null}).then(function(j){
      if(!r.ok){ var e=new Error((j&&j.error)||('HTTP '+r.status)); e.status=r.status; throw e; }
      return j;
    });
  },function(e){ clearTimeout(t); throw e; });
}
function api(path,o){
  o=o||{};
  if(o.urgent) return raw(path,2000);
  var e=epoch;
  var p=chain.then(function(){
    if(!o.poll && e!==epoch) return null;
    if(o.guard && !o.guard()) return null;
    return raw(path,4000);
  });
  chain=p.catch(function(){});
  return p;
}
function cmd(path,o){
  return api(path,o).then(function(j){ setOnline(true); return j; },function(e){
    if(e.status){ toast(e.status===403?'Аварійна зупинка активна':e.message); }
    else { setOnline(false); toast('Робот не відповідає'); }
    return null;
  });
}

var polling=false;
function poll(){
  if(polling||document.hidden) return;
  polling=true;
  api('/state',{poll:true}).then(function(j){
    S=j; fails=0; setOnline(true); loadCfg(); render();
  },function(){
    if(++fails>=5) setOnline(false);
  }).then(function(){polling=false});
}
setInterval(poll,400);
document.addEventListener('visibilitychange',function(){ if(document.hidden) endAllJogs(); else poll(); });

function setOnline(v){
  if(v) everOnline=true;
  if(v!==online){ online=v; if(!v) endAllJogs(); }
  document.body.classList.toggle('offline',!online);
  var p=$('#pConn');
  p.className='pill '+(online?'ok':(everOnline?'bad':''));
  p.lastChild.textContent=online?'Онлайн':(everOnline?'Немає зв’язку':'Підключення…');
  $('#banOff').hidden=online||!everOnline;
}

/* ---------- тротлінг для слайдерів ---------- */
var thr={};
function throttled(key,fn,ms){
  ms=ms||120;
  var t=thr[key]||(thr[key]={last:0,timer:null,fn:null});
  var wait=ms-(performance.now()-t.last);
  if(wait<=0){ t.last=performance.now(); t.fn=null; fn(); }
  else{
    t.fn=fn;
    if(!t.timer) t.timer=setTimeout(function(){
      t.timer=null; t.last=performance.now();
      if(t.fn){ var g=t.fn; t.fn=null; g(); }
    },wait);
  }
}
function clearPending(k){ var t=thr[k]; if(t){ clearTimeout(t.timer); t.timer=null; t.fn=null; } }
function clearAllPending(){ for(var k in thr) clearPending(k); }

/* ---------- утримуваний джог ---------- */
var jogEnders=[];
function endAllJogs(){ jogEnders.slice().forEach(function(fn){fn()}); }

function bindJog(btn,id,dir){
  var axis=KEY[id], active=false, iv=0, busy=false;
  function send(){
    if(busy||!active) return;
    busy=true;
    cmd('/jog?axis='+axis+'&dir='+dir,{guard:function(){return active}}).then(function(){busy=false});
  }
  function end(){
    if(!active) return;
    active=false; clearInterval(iv); btn.classList.remove('on'); jogging=false;
    if(axis!=='rot') cmd('/jog_stop',{urgent:true,quiet:true});
  }
  jogEnders.push(end);
  btn.addEventListener('pointerdown',function(e){
    if(locked()) return;
    e.preventDefault();
    try{btn.setPointerCapture(e.pointerId)}catch(_){}
    if(step>0){ cmd('/jog?axis='+axis+'&step='+(dir*step)); return; }
    active=true; jogging=true; btn.classList.add('on');
    send();
    /* сторожовий таймер прошивки — 1.8 с, тому повторюємо команду */
    iv=setInterval(send,axis==='rot'?70:600);
  });
  ['pointerup','pointercancel','lostpointercapture'].forEach(function(t){btn.addEventListener(t,end)});
  btn.addEventListener('contextmenu',function(e){e.preventDefault()});
}

/* ---------- осі та слайдери ---------- */
var rows={};
$$('.ax').forEach(function(row){
  var id=row.dataset.ax, rg=$('input',row), v=$('.v',row);
  rows[id]={rg:rg,v:v};
  rg.min=LIM[id][0]; rg.max=LIM[id][1];
  function send(){ lastUser[id]=performance.now(); cmd('/set?'+KEY[id]+'='+(+rg.value).toFixed(2)); }
  rg.addEventListener('pointerdown',function(){dragging[id]=true});
  rg.addEventListener('input',function(){
    lastUser[id]=performance.now(); v.textContent=(+rg.value).toFixed(1);
    throttled('s'+id,send);
  });
  rg.addEventListener('change',function(){ clearPending('s'+id); send(); });
  $$('.jb',row).forEach(function(b){ bindJog(b,id,+b.dataset.dir); });
  $$('[data-g]',row).forEach(function(b){
    b.addEventListener('click',function(){ rg.value=b.dataset.g; v.textContent=rg.value; send(); });
  });
});
['pointerup','pointercancel'].forEach(function(t){
  window.addEventListener(t,function(){ dragging={}; });
});

$$('#steps button').forEach(function(b){
  b.addEventListener('click',function(){
    step=+b.dataset.s;
    $$('#steps button').forEach(function(x){x.classList.toggle('on',x===b)});
  });
});

/* ---------- конфіг із прошивки: ліміти й геометрія завжди збігаються ---------- */
var cfgOk=false, cfgBusy=false;
function applyCfg(c){
  if(c.l1) L1=c.l1; if(c.l2) L2=c.l2;
  if(c.rmin) RMIN=c.rmin; if(c.rmax) RMAX=c.rmax;
  ['a1','a2','z','rot','g'].forEach(function(k){ if(c[k]&&c[k].length===2) LIM[k]=c[k]; });
  if(c.cols&&c.cols.length===3) COLS=c.cols;
  for(var id in rows){ rows[id].rg.min=LIM[id][0]; rows[id].rg.max=LIM[id][1]; }
  var gb=$$('[data-g]');
  if(gb.length===2){ gb[0].dataset.g=LIM.g[0]; gb[1].dataset.g=LIM.g[1]; }
  if(c.pts) $('#ptsMax').textContent=c.pts;
  sc=(W/2-34)/RMAX;
  cfgOk=true;
}
function loadCfg(){
  if(cfgOk||cfgBusy) return;
  cfgBusy=true;
  api('/config',{poll:true}).then(function(c){ if(c) applyCfg(c); },function(){}).then(function(){cfgBusy=false});
}

/* ---------- кнопки ---------- */
function doStop(){
  epoch++; clearAllPending(); endAllJogs(); pad.on=false;
  cmd('/stop',{urgent:true}).then(function(j){ if(j) toast('Зупинено'); poll(); });
}
$('#bStop').addEventListener('click',doStop);
document.addEventListener('keydown',function(e){ if(e.key==='Escape') doStop(); });
window.addEventListener('blur',endAllJogs);

$('#bHome').addEventListener('click',function(){
  cmd('/home').then(function(j){ if(j) toast('Їду в HOME'); });
});

$$('#seg button').forEach(function(b){
  b.addEventListener('click',function(){
    if(!S||S.estop||S.mode===b.dataset.m) return;
    cmd('/mode').then(function(j){
      if(!j) return;
      if(j.mode!==b.dataset.m) toast('Зупиніть осі, потім змініть режим');
      localAuto=0; poll();
    });
  });
});

$('#bTeach').addEventListener('click',function(){
  var prev=S?S.points:0;
  cmd('/teach').then(function(j){
    if(!j) return;
    toast(j.points>prev?('Точку P'+j.points+' збережено'):'Не збережено: зупиніть осі або програма заповнена');
    poll();
  });
});
$('#bClear').addEventListener('click',function(){
  if(!confirm('Очистити всю програму?')) return;
  cmd('/clear').then(function(j){ if(j) toast('Програму очищено'); poll(); });
});
$('#bAuto').addEventListener('click',function(){
  cmd('/auto').then(function(j){
    if(j&&j.auto_state!==undefined) localAuto=j.auto_state;
    if(S&&S.points===0) toast('Немає збережених точок');
    poll();
  });
});
$$('[data-col]').forEach(function(b){
  b.addEventListener('click',function(){
    cmd('/column?id='+b.dataset.col).then(function(j){ if(j) toast('Цикл колон запущено'); poll(); });
  });
});
$('#bColStop').addEventListener('click',doStop);

/* ---------- кінематика (дзеркало прошивки) ---------- */
var D2R=Math.PI/180;
function fk(a1,a2){
  var t1=a1*D2R, t2=(a1+a2)*D2R, ex=L1*Math.cos(t1), ey=L1*Math.sin(t1);
  return {ex:ex,ey:ey,x:ex+L2*Math.cos(t2),y:ey+L2*Math.sin(t2)};
}
function ik(x,y,up){
  var r2=x*x+y*y, r=Math.sqrt(r2);
  if(r<RMIN||r>RMAX) return null;
  var c=(r2-L1*L1-L2*L2)/(2*L1*L2);
  if(c<-1||c>1) return null;
  var s=Math.sqrt(1-c*c); if(!up) s=-s;
  var t2=Math.atan2(s,c);
  var t1=Math.atan2(y,x)-Math.atan2(L2*s,L1+L2*c);
  var a1=t1/D2R, a2=t2/D2R;
  if(a1<LIM.a1[0]||a1>LIM.a1[1]||a2<LIM.a2[0]||a2>LIM.a2[1]) return null;
  return {a1:a1,a2:a2};
}

/* ---------- полотно ---------- */
var cv=$('#cv'), cg=cv.getContext('2d');
var W=0,H=0,DPR=1,sc=1,cx=0,cy=0;
var CS=getComputedStyle(document.documentElement);
function col(n){ return CS.getPropertyValue(n).trim(); }
function fit(){
  DPR=window.devicePixelRatio||1; W=cv.clientWidth; H=cv.clientHeight;
  cv.width=Math.round(W*DPR); cv.height=Math.round(H*DPR);
  sc=(W/2-34)/RMAX; cx=W/2; cy=H/2;
}
if(window.ResizeObserver) new ResizeObserver(fit).observe(cv); else window.addEventListener('resize',fit);
fit();

var Dsp={a1:0,a2:0,t1:0,t2:0,z:0,tz:0,init:false};
var pad={on:false,x:0,y:0,ok:true};

function px(x){return cx+x*sc} function py(y){return cy-y*sc}

function arm(a1,a2,color,w,dash,alpha){
  var p=fk(a1,a2), g=cg;
  g.save(); g.globalAlpha=alpha||1; g.lineCap='round'; g.lineJoin='round';
  g.strokeStyle=color; g.lineWidth=w; g.setLineDash(dash||[]);
  g.beginPath(); g.moveTo(px(0),py(0)); g.lineTo(px(p.ex),py(p.ey)); g.lineTo(px(p.x),py(p.y)); g.stroke();
  g.restore(); return p;
}

function draw(){
  var g=cg; g.setTransform(DPR,0,0,DPR,0,0); g.clearRect(0,0,W,H);
  var TAU=Math.PI*2;

  /* сітка */
  g.lineWidth=1; g.strokeStyle=col('--grid');
  for(var r=50;r<=150;r+=50){ g.beginPath(); g.arc(cx,cy,r*sc,0,TAU); g.stroke(); }
  g.beginPath(); g.moveTo(cx,8); g.lineTo(cx,H-8); g.moveTo(8,cy); g.lineTo(W-36,cy); g.stroke();
  g.fillStyle=col('--mute'); g.font='11px system-ui,sans-serif';
  g.textAlign='left'; g.fillText('X',W-46,cy-6);
  g.textAlign='center'; g.fillText('Y',cx+9,16);

  /* робоча зона */
  g.beginPath(); g.arc(cx,cy,RMAX*sc,0,TAU); g.arc(cx,cy,RMIN*sc,0,TAU,true);
  g.fillStyle=col('--ws'); g.fill('evenodd');
  g.setLineDash([5,5]); g.strokeStyle=col('--wsline');
  g.beginPath(); g.arc(cx,cy,RMAX*sc,0,TAU); g.stroke();
  g.beginPath(); g.arc(cx,cy,RMIN*sc,0,TAU); g.stroke();
  g.setLineDash([]);

  /* колони */
  var actCol=(S&&S.col_act)?S.col_id:0;
  g.font='600 11px system-ui,sans-serif'; g.textAlign='center'; g.textBaseline='middle';
  COLS.forEach(function(c,i){
    var x=px(c[0]), y=py(c[1]), on=(actCol===i+1);
    g.beginPath(); g.rect(x-10,y-10,20,20);
    g.fillStyle=on?col('--teal'):'#fff'; g.fill();
    g.lineWidth=1.5; g.strokeStyle=col('--teal'); g.stroke();
    g.fillStyle=on?'#fff':col('--teal'); g.fillText(String(i+1),x,y+.5);
  });
  g.textBaseline='alphabetic';

  /* ціль (команда) */
  if(S && (Math.abs(Dsp.t1-Dsp.a1)>0.4||Math.abs(Dsp.t2-Dsp.a2)>0.4))
    arm(Dsp.t1,Dsp.t2,col('--teal'),3,[6,5],.9);

  /* фактична рука */
  var p=arm(Dsp.a1,Dsp.a2,col('--orange'),11,null,1);
  g.lineWidth=2; g.strokeStyle='#fff';
  g.beginPath(); g.arc(px(0),py(0),10,0,TAU); g.fillStyle=col('--ink'); g.fill(); g.stroke();
  g.beginPath(); g.arc(px(p.ex),py(p.ey),6.5,0,TAU); g.fillStyle='#fff'; g.fill();
  g.lineWidth=2.5; g.strokeStyle=col('--orange'); g.stroke();
  g.beginPath(); g.arc(px(p.x),py(p.y),6,0,TAU); g.fillStyle=col('--ink'); g.fill();
  g.lineWidth=2; g.strokeStyle='#fff'; g.stroke();

  /* ручка перетягування */
  if(!locked()){
    g.setLineDash([3,3]); g.lineWidth=1.5; g.strokeStyle=col('--orange');
    g.beginPath(); g.arc(px(p.x),py(p.y),16,0,TAU); g.stroke(); g.setLineDash([]);
  }
  if(pad.on){
    g.beginPath(); g.arc(px(pad.x),py(pad.y),9,0,TAU);
    g.fillStyle=pad.ok?'rgba(30,156,105,.25)':'rgba(215,38,61,.25)'; g.fill();
    g.lineWidth=2; g.strokeStyle=pad.ok?col('--green'):col('--red'); g.stroke();
  }

  /* датчик Z: 0 угорі, 300 унизу */
  var gx=W-16, gy=22, gh=H-44, gw=8;
  g.beginPath(); if(g.roundRect) g.roundRect(gx-gw/2,gy,gw,gh,4); else g.rect(gx-gw/2,gy,gw,gh);
  g.fillStyle='#dde3ec'; g.fill();
  var zf=Math.max(0,Math.min(1,Dsp.z/LIM.z[1]));
  if(zf>0){ g.beginPath(); g.rect(gx-gw/2,gy,gw,gh*zf); g.fillStyle=col('--orange'); g.fill(); }
  var tzf=Math.max(0,Math.min(1,Dsp.tz/LIM.z[1]));
  g.beginPath(); g.moveTo(gx-9,gy+gh*tzf); g.lineTo(gx+9,gy+gh*tzf); g.lineWidth=2; g.strokeStyle=col('--teal'); g.stroke();
  g.fillStyle=col('--mute'); g.font='10px system-ui,sans-serif'; g.textAlign='center';
  g.fillText('Z',gx,12); g.fillText('300',gx,H-6);
}

function frame(){
  requestAnimationFrame(frame);
  if(!W) return;
  if(S){
    if(!Dsp.init){ Dsp.a1=S.act_a1; Dsp.a2=S.act_a2; Dsp.z=S.act_z; Dsp.init=true; }
    var k=.3;
    Dsp.a1+=(S.act_a1-Dsp.a1)*k; Dsp.a2+=(S.act_a2-Dsp.a2)*k; Dsp.z+=(S.act_z-Dsp.z)*k;
    Dsp.t1=S.a1; Dsp.t2=S.a2; Dsp.tz=S.z;
  }
  draw();
}
requestAnimationFrame(frame);

/* перетягування кінчика інструмента */
function toWorld(e){
  var r=cv.getBoundingClientRect();
  var X=(e.clientX-r.left)*(W/r.width), Y=(e.clientY-r.top)*(H/r.height);
  return {x:(X-cx)/sc, y:(cy-Y)/sc, px:X, py:Y};
}
function nearTcp(w){
  var p=fk(Dsp.a1,Dsp.a2);
  return Math.hypot(w.px-px(p.x),w.py-py(p.y))<28;
}
function padSend(final){
  if(!pad.ok) return;
  var up=(S?S.a2:0)>=0;               /* зберігаємо поточну конфігурацію ліктя */
  var s=ik(pad.x,pad.y,up); if(!s) return;
  var q='/set?l1='+s.a1.toFixed(2)+'&l2='+s.a2.toFixed(2);
  ['a1','a2'].forEach(function(id){lastUser[id]=performance.now()});
  cmd(q);
}
cv.addEventListener('pointerdown',function(e){
  if(locked()) return;
  var w=toWorld(e); if(!nearTcp(w)) return;
  pad.on=true; pad.x=w.x; pad.y=w.y; pad.ok=true;
  try{cv.setPointerCapture(e.pointerId)}catch(_){}
  e.preventDefault();
});
cv.addEventListener('pointermove',function(e){
  var w=toWorld(e);
  if(!pad.on){ cv.style.cursor=(!locked()&&nearTcp(w))?'grab':'default'; return; }
  pad.x=w.x; pad.y=w.y;
  var up=(S?S.a2:0)>=0;
  pad.ok=!!ik(w.x,w.y,up);
  throttled('pad',function(){padSend(false)},110);
});
function padEnd(){
  if(!pad.on) return;
  pad.on=false; clearPending('pad'); padSend(true);
}
cv.addEventListener('pointerup',padEnd);
cv.addEventListener('pointercancel',function(){pad.on=false;clearPending('pad')});

/* ---------- відображення стану ---------- */
function render(){
  var s=S; if(!s) return;
  var isAuto=s.mode==='AUTO', now=performance.now();
  document.body.classList.toggle('auto',isAuto);
  document.body.classList.toggle('estop',!!s.estop);
  $('#banEstop').hidden=!s.estop;
  $('#pMode').textContent=isAuto?'Авто':'Ручний';
  $('#pRun').hidden=!(s.running||s.col_act);

  $$('#seg button').forEach(function(b){ b.classList.toggle('on',b.dataset.m===s.mode); });
  $('#grpManual').hidden=isAuto;
  $('#grpAuto').hidden=!isAuto;
  $('#pts').textContent=s.points;

  var a=(s.auto!==undefined)?s.auto:localAuto;
  $('#bAuto').textContent=(a===1||a===2)?'Пауза':(a===3?'Продовжити':'Старт');
  $('#bAuto').disabled=s.points===0||a===5;
  $('#autoTxt').textContent=s.points===0?'Спочатку збережіть точки в ручному режимі.':AUTO_TXT[a]||'';

  $('#rA1').textContent=f(s.act_a1); $('#rA2').textContent=f(s.act_a2); $('#rZ').textContent=f(s.act_z);
  $('#rX').textContent=f(s.act_x); $('#rY').textContent=f(s.act_y);

  var cur={a1:s.act_a1,a2:s.act_a2,z:s.act_z,rot:s.rot,g:s.grip};
  var tgt=(s.running&&jogging)?cur:{a1:s.a1,a2:s.a2,z:s.z,rot:s.rot,g:s.grip};
  for(var id in rows){
    if(dragging[id]) continue;
    rows[id].v.textContent=f(cur[id],id==='rot'||id==='g'?0:1);
    if(now-(lastUser[id]||0)>900) rows[id].rg.value=tgt[id];
  }

  $('#bColStop').hidden=!s.col_act;
  $('#colTxt').textContent=s.col_act?('Виконується колона '+s.col_id):'Над колонкою, вниз, пауза, вгору.';
  $('#padHint').hidden=isAuto||s.estop;
}

poll();
})();
</script>
</body>
</html>
)rawliteral";
