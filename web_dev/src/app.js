/**
 * dtuGateway — Alpine.js application store
 *
 * All dashboard state lives in a single Alpine store ('app').
 * Polling intervals mirror the original ESP32 dashboard timings:
 *   data.json    → 1 000 ms
 *   info.json    → 5 000 ms
 *   dtuData.json → 7 500 ms
 *   version      → 300 000 ms
 *
 * All settings POSTs use application/x-www-form-urlencoded to match
 * the ESP32 AsyncWebServer form handler expectations.
 */

document.addEventListener('alpine:init', () => {

  Alpine.store('app', {

    // ── Reactive state ──────────────────────────────────────────────
    data:     {},   // /api/data.json
    info:     {},   // /api/info.json
    dtuData:  { warningCount: 0, warningsActive: 0, warningsLastUpdate: 0, warnings: [] },  // /api/dtuData.json
    events:   { events: [], statistics: { eventCount: 0 } },  // /api/dtuEvents.json

    // UI state
    drawerOpen:     false,
    drawerTab:      'wifi',
    showWarnings:   false,
    showPowerLimit: false,
    showEvents:     false,
    rebootTarget:   null,
    fwFile:         null,
    updateProgress: -1,
    toasts:         [],
    _toastSeq:      0,

    passVis: { wifiPass: false, mqttPass: false },

    // Reload progress bar (counts down between last DTU response updates)
    reloadBarPct:  100,
    _waitMs:       31000,
    _barMs:        31000,

    // Value-change flash
    ui: { gridFlash: false, pv0Flash: false, pv1Flash: false },

    // Editable form values (populated when drawer opens)
    form: {
      wifiSSID: '', wifiPass: '',
      dtuIp: '', dtuCycle: 31,
      dtuCloudPause: true, remoteDisplay: false, remoteSummary: false,
      batteryMonitor: false,
      ohActive: false, ohIp: '', ohPrefix: '',
      mqttActive: false, mqttTLS: false,
      mqttIpPort: '', mqttUser: '', mqttPass: '', mqttTopic: '', mqttHA: false,
      powerLimit: 100,
    },

    // ── Init ────────────────────────────────────────────────────────
    init() {
      this._fetchData();
      this._fetchInfo();
      this._fetchDtuData();
      this._fetchEvents();

      setInterval(() => this._fetchData(),    1000);
      setInterval(() => this._fetchInfo(),    5000);
      setInterval(() => this._fetchDtuData(), 7500);
      setInterval(() => this._fetchEvents(),  15000);
      setInterval(() => this._checkVersion(), 300000);

      // Reload bar ticker (100 ms)
      setInterval(() => {
        if (this._barMs > 0) {
          this._barMs -= 100;
          this.reloadBarPct = Math.max(0, (this._barMs / this._waitMs) * 100);
        }
      }, 100);
    },

    // ── Fetch helpers ───────────────────────────────────────────────
    async _get(url) {
      const res = await fetch(url, { cache: 'no-store' });
      if (!res.ok) throw new Error(`${url} → ${res.status}`);
      return res.json();
    },

    async _post(url, params) {
      const body = new URLSearchParams(params);
      const res = await fetch(url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: body.toString(),
      });
      if (!res.ok) throw new Error(`${url} → ${res.status}`);
      return res.json();
    },

    // ── Polling fetchers ────────────────────────────────────────────
    async _fetchData() {
      try {
        const d = await this._get('/api/data.json');

        // Detect value changes for flash animation
        if (this.data.grid && d.grid.p !== this.data.grid.p) this._flash('grid');
        if (this.data.pv0  && d.pv0.p  !== this.data.pv0.p)  this._flash('pv0');
        if (this.data.pv1  && d.pv1.p  !== this.data.pv1.p)  this._flash('pv1');

        // Reset reload bar when we get a fresh lastResponse
        if (this.data.lastResponse !== d.lastResponse) {
          this._waitMs = (this.info.dtuConnection?.dtuDataCycle ?? 31) * 1000;
          this._barMs  = this._waitMs;
        }

        this.data = d;

        // Update page title with current grid power
        const gridP = isNaN(d.grid?.p) ? '--' : d.grid.p.toFixed(0);
        document.title = `${gridP} W — dtuGateway`;
      } catch (_) {
        // Silent fail; stale data shown until next poll
      }
    },

    async _fetchInfo() {
      try {
        this.info = await this._get('/api/info.json');
        this._waitMs = (this.info.dtuConnection?.dtuDataCycle ?? 31) * 1000;
      } catch (_) { /* silent */ }
    },

    async _fetchDtuData() {
      try {
        const d = await this._get('/api/dtuData.json');
        // sort warnings newest first (timestampStart descending)
        if (d.warnings?.length) {
          d.warnings.sort((a, b) => b.timestampStart - a.timestampStart);
          d.warningsActive = d.warnings.filter(w => w.timestampStop === 0).length;
        } else {
          d.warningsActive = 0;
        }
        this.dtuData = d;
      } catch (_) { /* silent */ }
    },

    async _fetchEvents() {
      try {
        this.events = await this._get('/api/dtuEvents.json');
      } catch (_) { /* silent */ }
    },

    async _checkVersion() {
      try {
        const d = await this._get('/updateGetInfo');
        if (this.info.firmware) {
          this.info.firmware.versionServer   = d.versionServer;
          this.info.firmware.updateAvailable = d.updateAvailable;
          this.info.firmware.selectedUpdateChannel = d.selectedUpdateChannel;
        }
      } catch (_) { /* silent */ }
    },

    // ── Value-flash helper ──────────────────────────────────────────
    _flash(channel) {
      const key = `${channel}Flash`;
      this.ui[key] = false;
      // Micro-delay so Alpine re-renders the removal before re-adding
      requestAnimationFrame(() => {
        this.ui[key] = true;
        setTimeout(() => { this.ui[key] = false; }, 700);
      });
    },

    // ── Formatters ──────────────────────────────────────────────────
    /**
     * Format a numeric value.
     * @param {number|undefined} val
     * @param {number} decimals
     * @param {string} unit
     * @returns {string}
     */
    fmt(val, decimals = 1, unit = '') {
      if (val === undefined || val === null || isNaN(val)) {
        return `--${unit ? ' ' + unit : ''}`;
      }
      return `${Number(val).toFixed(decimals)}${unit ? ' ' + unit : ''}`;
    },

    /**
     * Format a Unix timestamp.
     * @param {number} ts  - Unix seconds
     * @param {'time'|'date'|'short'} mode
     * @returns {string}
     */
    // Format ESP32 millis() uptime as d HH:MM:SS or HH:MM:SS
    formatMillis(ms) {
      if (!ms && ms !== 0) return '--';
      const totalSec = Math.floor(ms / 1000);
      const s = totalSec % 60;
      const m = Math.floor(totalSec / 60) % 60;
      const h = Math.floor(totalSec / 3600) % 24;
      const d = Math.floor(totalSec / 86400);
      const pad = v => String(v).padStart(2, '0');
      return d > 0
        ? `${d}d ${pad(h)}:${pad(m)}:${pad(s)}`
        : `${pad(h)}:${pad(m)}:${pad(s)}`;
    },

    formatTs(ts, mode = 'time') {
      if (!ts) return '--:--:--';
      const d = new Date(ts * 1000);
      if (mode === 'time') {
        return d.toLocaleTimeString('de-DE');
      }
      if (mode === 'date') {
        return d.toLocaleDateString('de-DE');
      }
      if (mode === 'short') {
        const dd = String(d.getDate()).padStart(2, '0');
        const mm = String(d.getMonth() + 1).padStart(2, '0');
        const hh = String(d.getHours()).padStart(2, '0');
        const min = String(d.getMinutes()).padStart(2, '0');
        return `${dd}.${mm}. ${hh}:${min}`;
      }
      if (mode === 'datetime') {
        return d.toLocaleDateString('de-DE', { day: '2-digit', month: '2-digit', year: 'numeric' })
          + ' ' + d.toLocaleTimeString('de-DE');
      }
      return String(ts);
    },

    /**
     * Returns contextual label/value pairs for warning data0/data1 fields.
     * Mirrors the original showDtuWarnings() logic.
     * @param {object} w - warning object
     * @returns {object|null} - {l0, v0, l1, v1} or null if data0 is zero
     */
    warnDetail(w) {
      if (!w.data0) return null;
      const msg = w.message.toLowerCase().replace(/-/g, '');
      let l0 = 'data0', v0 = String(w.data0);
      let l1 = 'data1', v1 = String(w.data1);
      if (msg.includes('undervoltage') || msg.includes('overvoltage')) {
        l0 = 'measured voltage'; v0 = (w.data0 / 10).toFixed(2) + ' V';
        l1 = msg.includes('undervoltage') ? 'minimal voltage' : 'maximum voltage';
        v1 = (w.data1 / 10).toFixed(2) + ' V';
      } else if (msg.includes('overfrequency') || msg.includes('underfrequency')) {
        l0 = 'measured frequency'; v0 = (w.data0 / 100).toFixed(2) + ' Hz';
        l1 = msg.includes('underfrequency') ? 'minimum frequency' : 'maximum frequency';
        v1 = (w.data1 / 100).toFixed(2) + ' Hz';
      } else if (msg.includes('over temperature')) {
        l0 = 'measured temperature'; v0 = (w.data0 / 100).toFixed(2) + ' °C';
        l1 = 'maximum temperature'; v1 = (w.data1 / 100).toFixed(2) + ' °C';
      }
      return { l0, v0, l1, v1 };
    },

    // ── Connection state labels & badge class ───────────────────────
    connStateLabel() {
      const labels = [
        'offline', 'connected', 'cloud pause', 'reconnecting',
        'rebooting', 'connect error', 'stopped', 'reboot inverter',
      ];
      return labels[this.data.dtuConnState] ?? 'unknown';
    },

    connBadgeClass() {
      const s = this.data.dtuConnState;
      if (s === 1) return 'ok';
      if (s === 2) return 'paused';
      if (s === 3 || s === 4 || s === 7) return 'warn';
      return 'error';
    },

    dtuErrorLabel() {
      const labels = ['ok', 'no time', 'time error', 'data error', 'last TX done'];
      return labels[this.data.dtuErrorState] ?? 'no info';
    },

    // ── Remote summary display mode helpers ─────────────────────────
    // Backward-compat: old firmware uses dtuRemoteSummaryDisplay (single flag)
    // Feature branch: dtuRemoteDisplay_SolarMonitor + dtuRemoteDisplay_BatteryMonitor
    isSolarMonitor() {
      const c = this.info.dtuConnection;
      return !!(c?.dtuRemoteDisplay_SolarMonitor || c?.dtuRemoteSummaryDisplay);
    },
    isBatteryMonitor() {
      return !!(this.info.dtuConnection?.dtuRemoteDisplay_BatteryMonitor);
    },
    isMonitorMode() {
      return this.isSolarMonitor() || this.isBatteryMonitor();
    },
    monitorLabel() {
      const s = this.isSolarMonitor(), b = this.isBatteryMonitor();
      if (s && b) return 'solar & battery monitor';
      if (s)      return 'solar monitor';
      return              'battery monitor';
    },
    monitorGlow() {
      const s = this.isSolarMonitor(), b = this.isBatteryMonitor();
      if (s && b) return 'monitor-glow--both';
      if (s)      return 'monitor-glow--solar';
      return              'monitor-glow--battery';
    },

    pageTitle() {
      const dev = this.info.dtuConnection?.deviceData;
      let model = dev?.inverter_model ?? 'HMS-xxxW-xT';
      if (model === '--') model = 'HMS-xxxW-xT';
      if (this.info.dtuConnection?.dtuRemoteDisplay) return `Hoymiles ${model} — Remote Display`;
      const s = this.isSolarMonitor(), b = this.isBatteryMonitor();
      if (s && b) return `Hoymiles ${model} — Solar & Battery Monitor`;
      if (s)      return `Hoymiles ${model} — Solar Monitor`;
      if (b)      return `Hoymiles ${model} — Battery Monitor`;
      return `Hoymiles ${model} — Gateway`;
    },

    // ── Drawer ──────────────────────────────────────────────────────
    openDrawer(tab = 'wifi') {
      this.drawerTab = tab;
      this.drawerOpen = true;
      this._populateForm();
      if (tab === 'wifi') this.requestWifiScan();
    },

    closeDrawer() {
      this.drawerOpen = false;
      this.rebootTarget = null;
    },

    _populateForm() {
      const i = this.info;
      if (!i || !i.wifiConnection) return;

      const wc   = i.wifiConnection;
      const dtu  = i.dtuConnection ?? {};
      const oh   = i.openHabConnection ?? {};
      const mqtt = i.mqttConnection ?? {};

      this.form.wifiSSID       = wc.wifiSsid ?? '';
      this.form.wifiPass       = wc.wifiPassword ?? '';

      this.form.dtuIp          = dtu.dtuHostIpDomain ?? '';
      this.form.dtuCycle       = dtu.dtuDataCycle ?? 31;
      this.form.dtuCloudPause  = !!dtu.dtuCloudPause;
      this.form.remoteDisplay  = !!dtu.dtuRemoteDisplay;
      // old fw: dtuRemoteSummaryDisplay; feature-branch fw: dtuRemoteDisplay_SolarMonitor
      this.form.remoteSummary  = !!(dtu.dtuRemoteSummaryDisplay || dtu.dtuRemoteDisplay_SolarMonitor);
      this.form.batteryMonitor = !!dtu.dtuRemoteDisplay_BatteryMonitor;

      this.form.ohActive       = !!oh.ohActive;
      this.form.ohIp           = oh.ohHostIp ?? '';
      this.form.ohPrefix       = oh.ohItemPrefix ?? '';

      this.form.mqttActive     = !!mqtt.mqttActive;
      this.form.mqttTLS        = !!mqtt.mqttUseTLS;
      this.form.mqttIpPort     = mqtt.mqttIp ? `${mqtt.mqttIp}:${mqtt.mqttPort}` : '';
      this.form.mqttUser       = mqtt.mqttUser ?? '';
      this.form.mqttPass       = mqtt.mqttPassword ?? '';
      this.form.mqttTopic      = mqtt.mqttMainTopic ?? '';
      this.form.mqttHA         = !!mqtt.mqttHAautoDiscoveryON;

      this.form.powerLimit     = this.data.inverter?.pLimSet ?? 100;
    },

    // ── Settings save actions ───────────────────────────────────────
    async saveWifi() {
      if (this.info.protectSettings) return;
      try {
        await this._post('/updateWifiSettings', {
          wifiSSIDsend: this.form.wifiSSID,
          wifiPASSsend: this.form.wifiPass,
        });
        this._toast('WiFi settings saved', 'success');
      } catch (e) {
        this._toast('Save failed: ' + e.message, 'error');
      }
    },

    async saveDtu() {
      if (this.info.protectSettings) return;
      try {
        await this._post('/updateDtuSettings', {
          dtuHostIpDomainSend:         this.form.dtuIp,
          dtuDataCycleSend:            this.form.dtuCycle,
          dtuCloudPauseSend:           this.form.dtuCloudPause ? '1' : '0',
          // mutually exclusive: remoteDisplay disables summary/monitor flags and vice versa
          remoteDisplayActiveSend:            (this.form.remoteDisplay && !this.form.remoteSummary && !this.form.batteryMonitor) ? '1' : '0',
          remoteSummaryDisplayActiveSend:     (!this.form.remoteDisplay && this.form.remoteSummary)  ? '1' : '0',
          ...('dtuRemoteDisplay_SolarMonitor' in (this.info.dtuConnection ?? {}) ? {
            solarMonitorActiveSend:   (!this.form.remoteDisplay && this.form.remoteSummary)  ? '1' : '0',
            batteryMonitorActiveSend: (!this.form.remoteDisplay && this.form.batteryMonitor) ? '1' : '0',
          } : {}),
        });
        this._toast('DTU settings saved', 'success');
        this._fetchInfo();
      } catch (e) {
        this._toast('Save failed: ' + e.message, 'error');
      }
    },

    async saveBindings() {
      if (this.info.protectSettings) return;
      try {
        await this._post('/updateBindingsSettings', {
          openhabActiveSend:          this.form.ohActive ? '1' : '0',
          openhabIPSend:              this.form.ohIp,
          ohItemPrefixSend:           this.form.ohPrefix,
          mqttActiveSend:             this.form.mqttActive ? '1' : '0',
          mqttUseTLSSend:             this.form.mqttTLS ? '1' : '0',
          mqttIPSend:                 this.form.mqttIpPort,
          mqttUserSend:               this.form.mqttUser,
          mqttPasswordSend:           this.form.mqttPass,
          mqttMainTopicSend:          this.form.mqttTopic,
          mqttHAautoDiscoveryONSend:  this.form.mqttHA ? '1' : '0',
        });
        this._toast('Bindings saved', 'success');
      } catch (e) {
        this._toast('Save failed: ' + e.message, 'error');
      }
    },

    async savePowerLimit() {
      try {
        await this._post('/updatePowerLimit', {
          powerLimitSet: this.form.powerLimit,
        });
        this._toast(`Power limit set to ${this.form.powerLimit} %`, 'success');
        this.showPowerLimit = false;
      } catch (e) {
        this._toast('Save failed: ' + e.message, 'error');
      }
    },

    // ── OTA update ────────────────────────────────────────���─────────
    async setChannel(ch) {
      try {
        await this._post('/updateOTASettings', { updateChannel: ch });
        if (this.info.firmware) this.info.firmware.selectedUpdateChannel = ch;
      } catch (e) {
        this._toast('Channel change failed', 'error');
      }
    },

    async startOnlineUpdate() {
      this.updateProgress = 0;
      try {
        await this._post('/doupdate', {});
        this._pollUpdateProgress();
      } catch (e) {
        this._toast('Update failed: ' + e.message, 'error');
        this.updateProgress = -1;
      }
    },

    manualFwSelected(event) {
      this.fwFile = event.target.files[0] ?? null;
    },

    async uploadManualFirmware() {
      if (!this.fwFile) return;
      const fd = new FormData();
      fd.append('update', this.fwFile);
      this.updateProgress = 0;
      try {
        const res = await fetch('/doupdate', { method: 'POST', body: fd });
        if (!res.ok) throw new Error(res.statusText);
        this._pollUpdateProgress();
      } catch (e) {
        this._toast('Upload failed: ' + e.message, 'error');
        this.updateProgress = -1;
      }
    },

    _pollUpdateProgress() {
      const interval = setInterval(async () => {
        try {
          const s = await this._get('/updateState');
          this.updateProgress = s.progress ?? 0;
          if (!s.updateRunning) {
            clearInterval(interval);
            this._toast('Update complete — rebooting…', 'success');
            setTimeout(() => { this.updateProgress = -1; }, 4000);
          }
        } catch (_) {
          clearInterval(interval);
          this.updateProgress = -1;
        }
      }, 500);
    },

    // ── Reboot ──────────────────────────────────────────────────────
    confirmReboot(target) {
      this.rebootTarget = target;
    },

    rebootLabel() {
      const labels = { dtuGw: 'dtuGateway', dtu: 'DTU', mi: 'Micro Inverter' };
      return labels[this.rebootTarget] ?? this.rebootTarget;
    },

    async executeReboot() {
      const endpoints = { dtuGw: '/rebootDtuGw', dtu: '/rebootDtu', mi: '/rebootMi' };
      const url = endpoints[this.rebootTarget];
      if (!url) return;
      try {
        await this._post(url, {});
        this._toast(`${this.rebootLabel()} reboot initiated`, 'warn');
        this.rebootTarget = null;
      } catch (e) {
        this._toast('Reboot request failed', 'error');
      }
    },

    // ── WiFi scan ───────────────────────────────────────────────────
    async requestWifiScan() {
      try {
        await this._get('/getWifiNetworks');
        if (this.info.wifiConnection) this.info.wifiConnection.wifiScanIsRunning = 1;

        // Poll info until scan complete (max 15 s)
        let elapsed = 0;
        const poller = setInterval(async () => {
          elapsed += 250;
          await this._fetchInfo();
          if (!this.info.wifiConnection?.wifiScanIsRunning || elapsed >= 15000) {
            clearInterval(poller);
          }
        }, 250);
      } catch (_) { /* silent */ }
    },

    // ── DTU events clear ────────────────────────────────────────────
    async clearEvents() {
      try {
        await this._get('/api/dtuEventsClear');
        this.events = { eventCount: 0, events: [] };
        this._toast('Events cleared', 'success');
      } catch (e) {
        this._toast('Clear failed: ' + e.message, 'error');
      }
    },

    // ── Misc helpers ────────────────────────────────────────────────
    togglePassVis(field) {
      this.passVis[field] = !this.passVis[field];
    },

    mqttSectionHint() {
      if (this.form.remoteDisplay) {
        return 'Remote display active — subscribes to source dtuGateway MQTT topics.';
      }
      if (this.form.remoteSummary) {
        return 'Solar Monitor active — subscribes to PV power / daily yield topics.';
      }
      return 'Publish all data to a MQTT broker and subscribe to power-limit commands.';
    },

    // ── Toast system ────────────────────────────────────────────────
    /**
     * Show a temporary notification.
     * @param {string} msg
     * @param {'success'|'error'|'warn'|''} type
     */
    _toast(msg, type = '') {
      const id = ++this._toastSeq;
      this.toasts.push({ id, msg, type });
      setTimeout(() => {
        this.toasts = this.toasts.filter(t => t.id !== id);
      }, 4000);
    },
  });

});
