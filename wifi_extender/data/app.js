// app.js — shared utilities for Abdul WiFi Extender dashboard
// Developer: Abdul Korim | github.com/abdul-korim-web

'use strict';

// ── Auth guard ─────────────────────────────────────────────────────────────
function getToken() { return sessionStorage.getItem('token') || ''; }

function requireAuth() {
  if (!getToken()) { location.href = '/'; }
}

// ── Authenticated fetch wrapper ────────────────────────────────────────────
async function apiFetch(url, options = {}) {
  const token = getToken();
  options.headers = options.headers || {};
  options.headers['Authorization'] = 'Bearer ' + token;
  if (options.body && typeof options.body === 'object' && !(options.body instanceof FormData)) {
    options.body = JSON.stringify(options.body);
    options.headers['Content-Type'] = 'application/json';
  }
  const r = await fetch(url, options);
  if (r.status === 401) { sessionStorage.removeItem('token'); location.href = '/'; }
  return r;
}

// ── Toast notification ─────────────────────────────────────────────────────
let toastTimer = null;
function showToast(msg, type = 'info', duration = 3000) {
  const el = document.getElementById('toast');
  if (!el) return;
  el.textContent = msg;
  el.className = 'show ' + type;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { el.className = ''; }, duration);
}

// ── Format uptime seconds → human readable ─────────────────────────────────
function formatUptime(secs) {
  const d = Math.floor(secs / 86400);
  const h = Math.floor((secs % 86400) / 3600);
  const m = Math.floor((secs % 3600) / 60);
  const s = secs % 60;
  let str = '';
  if (d) str += d + 'd ';
  if (h || d) str += h + 'h ';
  str += m + 'm ' + s + 's';
  return str;
}

// ── Format bytes ───────────────────────────────────────────────────────────
function formatBytes(bytes) {
  if (bytes < 1024) return bytes + ' B';
  if (bytes < 1048576) return (bytes / 1024).toFixed(1) + ' KB';
  return (bytes / 1048576).toFixed(2) + ' MB';
}

// ── RSSI → signal label ────────────────────────────────────────────────────
function rssiLabel(rssi) {
  if (rssi >= -50) return 'Excellent';
  if (rssi >= -65) return 'Good';
  if (rssi >= -75) return 'Fair';
  return 'Weak';
}

// ── RSSI → bar count (1–4) ────────────────────────────────────────────────
function rssiBars(rssi) {
  if (rssi >= -50) return 4;
  if (rssi >= -65) return 3;
  if (rssi >= -75) return 2;
  return 1;
}

// ── Render signal bars HTML ────────────────────────────────────────────────
function signalBarsHTML(rssi) {
  const lit = rssiBars(rssi);
  const heights = ['40%','60%','80%','100%'];
  return '<div class="signal-bar">' +
    heights.map((h, i) =>
      `<span style="height:${h}" class="${i < lit ? 'lit' : ''}"></span>`
    ).join('') + '</div>';
}

// ── Sidebar navigation ─────────────────────────────────────────────────────
function initSidebar() {
  const sidebar  = document.getElementById('sidebar');
  const overlay  = document.getElementById('overlay');
  const btnMenu  = document.getElementById('btnMenu');

  if (btnMenu) {
    btnMenu.addEventListener('click', () => {
      sidebar.classList.toggle('open');
      overlay.classList.toggle('open');
    });
  }
  if (overlay) {
    overlay.addEventListener('click', () => {
      sidebar.classList.remove('open');
      overlay.classList.remove('open');
    });
  }

  // Highlight active nav item based on current page
  const path = location.pathname.replace('/','').replace('.html','') || 'dashboard';
  document.querySelectorAll('.nav-item[data-page]').forEach(el => {
    if (el.dataset.page === path) el.classList.add('active');
  });
}

// ── Logout ─────────────────────────────────────────────────────────────────
async function doLogout() {
  try { await apiFetch('/api/logout', { method: 'POST' }); } catch(_) {}
  sessionStorage.removeItem('token');
  location.href = '/';
}

// ── Confirm dialog helper ──────────────────────────────────────────────────
function confirmDialog(title, msg) {
  return new Promise(resolve => {
    const overlay = document.getElementById('confirmOverlay');
    const titleEl = document.getElementById('confirmTitle');
    const msgEl   = document.getElementById('confirmMsg');
    const btnOk   = document.getElementById('confirmOk');
    const btnCancel = document.getElementById('confirmCancel');
    titleEl.textContent = title;
    msgEl.textContent = msg;
    overlay.classList.add('open');
    function cleanup(val) {
      overlay.classList.remove('open');
      btnOk.removeEventListener('click', onOk);
      btnCancel.removeEventListener('click', onCancel);
      resolve(val);
    }
    function onOk() { cleanup(true); }
    function onCancel() { cleanup(false); }
    btnOk.addEventListener('click', onOk);
    btnCancel.addEventListener('click', onCancel);
  });
}

// ── Shared sidebar + modal HTML fragment ───────────────────────────────────
function renderShell(activePage, pageTitle, pageSubtitle) {
  const pages = [
    { id:'dashboard',  label:'Dashboard',      icon:'home'    },
    { id:'wifi',       label:'WiFi Config',     icon:'wifi'    },
    { id:'clients',    label:'Devices',         icon:'devices' },
    { id:'monitor',    label:'Monitor',         icon:'chart'   },
    { id:'settings',   label:'Settings',        icon:'settings'},
    { id:'about',      label:'About',           icon:'info'    },
  ];
  const icons = {
    home:    '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M3 9l9-7 9 7v11a2 2 0 01-2 2H5a2 2 0 01-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg>',
    wifi:    '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M5 12.55a11 11 0 0114.08 0"/><path d="M1.42 9a16 16 0 0121.16 0"/><path d="M8.53 16.11a6 6 0 016.95 0"/><circle cx="12" cy="20" r="1" fill="currentColor"/></svg>',
    devices: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8M12 17v4"/></svg>',
    chart:   '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="22 12 18 12 15 21 9 3 6 12 2 12"/></svg>',
    settings:'<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 00.33 1.82l.06.06a2 2 0 010 2.83 2 2 0 01-2.83 0l-.06-.06a1.65 1.65 0 00-1.82-.33 1.65 1.65 0 00-1 1.51V21a2 2 0 01-4 0v-.09A1.65 1.65 0 009 19.4a1.65 1.65 0 00-1.82.33l-.06.06a2 2 0 01-2.83-2.83l.06-.06A1.65 1.65 0 004.68 15a1.65 1.65 0 00-1.51-1H3a2 2 0 010-4h.09A1.65 1.65 0 004.6 9a1.65 1.65 0 00-.33-1.82l-.06-.06a2 2 0 012.83-2.83l.06.06A1.65 1.65 0 009 4.68a1.65 1.65 0 001-1.51V3a2 2 0 014 0v.09a1.65 1.65 0 001 1.51 1.65 1.65 0 001.82-.33l.06-.06a2 2 0 012.83 2.83l-.06.06A1.65 1.65 0 0019.4 9a1.65 1.65 0 001.51 1H21a2 2 0 010 4h-.09a1.65 1.65 0 00-1.51 1z"/></svg>',
    info:    '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><line x1="12" y1="8" x2="12" y2="12"/><line x1="12" y1="16" x2="12.01" y2="16"/></svg>',
    logout:  '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 21H5a2 2 0 01-2-2V5a2 2 0 012-2h4"/><polyline points="16 17 21 12 16 7"/><line x1="21" y1="12" x2="9" y2="12"/></svg>',
  };

  const navHtml = pages.map(p =>
    `<a class="nav-item${p.id===activePage?' active':''}" href="/${p.id==='dashboard'?'dashboard':p.id}.html" data-page="${p.id}">
      ${icons[p.icon]}<span>${p.label}</span>
    </a>`
  ).join('');

  return `
<div class="app-shell">
  <!-- Overlay (mobile) -->
  <div class="overlay" id="overlay"></div>

  <!-- Sidebar -->
  <aside class="sidebar" id="sidebar">
    <div class="sidebar-header">
      <span class="logo-icon">📡</span>
      <div>
        <div class="logo-text">Abdul WiFi</div>
        <div class="logo-sub">Extender Admin</div>
      </div>
    </div>
    <nav class="nav">
      <div class="nav-section">Navigation</div>
      ${navHtml}
      <div class="nav-section" style="margin-top:.75rem;">Account</div>
      <a class="nav-item" href="#" onclick="doLogout();return false;">
        ${icons.logout}<span>Logout</span>
      </a>
    </nav>
    <div class="sidebar-footer">
      <div>Abdul WiFi Extender</div>
      <div>by <a href="https://github.com/abdul-korim-web" target="_blank">Abdul Korim</a></div>
    </div>
  </aside>

  <!-- Main -->
  <div class="main-content">
    <!-- Mobile topbar -->
    <div class="topbar">
      <button class="btn-menu" id="btnMenu">
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <line x1="3" y1="6" x2="21" y2="6"/><line x1="3" y1="12" x2="21" y2="12"/><line x1="3" y1="18" x2="21" y2="18"/>
        </svg>
      </button>
      <span class="topbar-title">📡 Abdul WiFi</span>
      <span></span>
    </div>

    <div class="page-header">
      <div>
        <div class="page-title">${pageTitle}</div>
        <div class="page-subtitle">${pageSubtitle}</div>
      </div>
    </div>
    <div class="page-body" id="pageBody">`;
  // caller must close: </div></div></div>
}

