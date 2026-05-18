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
    setupMode:      false,  // true when no WiFi configured (factory reset)
    wifiNetworks:   [],     // Available WiFi networks for setup screen
    setupSelectedSSID: '',  // Currently selected SSID in setup screen
    setupPassword:  '',     // Password entered in setup screen
    setupLoading:   false,  // true while connecting WiFi
    drawerOpen:     false,
    drawerTab:      'wifi',
    showWarnings:   false,
    showPowerLimit: false,
    showEvents:     false,
    showRebootOverlay: false,  // Show overlay while device reboots/reconnects
    rebootStatus:   'Applying settings...',
    willReboot:     false,     // Track if backend will actually reboot
    rebootTarget:   null,
    fwFile:         null,
    updateProgress: -1,
    _updateTimeout: 0,      // countdown timer during update (60 sec max)
    _updateInterval: null,  // interval handle for progress polling
    _updateStatus:  '',     // human-readable status message
    toasts:         [],
    _toastSeq:      0,
    _firstSetupDone: false, // Track if we've checked for first-setup condition

    passVis: { wifiPass: false, mqttPass: false },
    passActual: { wifiPass: '', mqttPass: '' }, // Store actual passwords (shown as dots in form)

    // Startup loading state
    isLoading:      true,   // true until first data is received
    _dataReceived:  false,  // Track if we've successfully fetched initial data
    _infoReceived:  false,  // Track if we've successfully fetched initial info

    // Connection status (weak network/unreachable detection)
    backendReachable:       true,   // false if backend unreachable for >10 sec
    _lastSuccessfulFetch:   null,   // timestamp of last successful data.json fetch
    _connectionLossTimeout: null,   // timer handle for connection loss detection
    _connectionCheckInterval: null, // timer handle for periodic connection checks

    // WiFi scan state
    _wifiScanInitiated: false, // Track if scan was explicitly requested (for UI feedback)

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
    /**
     * Fetch with timeout (10 seconds for data endpoints, 5 for others)
     * @param {string} url
     * @param {number} timeout - timeout in ms (default 10000)
     * @returns {Promise}
     */
    async _get(url, timeout = 10000) {
      const controller = new AbortController();
      const timeoutId = setTimeout(() => controller.abort(), timeout);
      try {
        const res = await fetch(url, { cache: 'no-store', signal: controller.signal });
        clearTimeout(timeoutId);
        if (!res.ok) throw new Error(`${url} → ${res.status}`);
        return res.json();
      } catch (err) {
        clearTimeout(timeoutId);
        throw err;
      }
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

        // Mark first successful data fetch & hide loading screen
        if (!this._dataReceived) {
          this._dataReceived = true;
          this._updateLoadingState();
        }

        // Update page title with current grid power
        const gridP = isNaN(d.grid?.p) ? '--' : d.grid.p.toFixed(0);
        document.title = `${gridP} W — dtuGateway`;

        // Connection recovery: clear loss state on successful fetch
        if (!this.backendReachable) {
          this.backendReachable = true;
          this._resetConnectionLossTimer();
          this.addToast('success', 'Connection restored');
        }

        // Reset connection loss timer
        this._lastSuccessfulFetch = Date.now();
        this._resetConnectionLossTimer();
      } catch (err) {
        // Mark connection as lost if fetch failed
        this._handleFetchError(err);
      }
    },

    async _fetchInfo() {
      try {
        this.info = await this._get('/api/info.json', 5000);
        this._waitMs = (this.info.dtuConnection?.dtuDataCycle ?? 31) * 1000;
        
        // Mark first successful info fetch & hide loading screen
        if (!this._infoReceived) {
          this._infoReceived = true;
          this._updateLoadingState();
        }
        
        // First-setup detection: if initMode === 1 (AP mode active = factory reset)
        // or wifiSsid is empty, enter setup mode
        if (!this._firstSetupDone && (this.info.initMode === 1 || this.info.wifiConnection?.wifiSsid === '')) {
          this._firstSetupDone = true;
          this.setupMode = true;
          this._scanWifiNetworks();
        }
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

    // ── Loading state helper ─────────────────────────────────────────
    _updateLoadingState() {
      // Hide loading screen once both data and info have been fetched
      if (this._dataReceived && this._infoReceived) {
        this.isLoading = false;
      }
    },

    // ── Connection loss detection helper ────────────────────────────
    _resetConnectionLossTimer() {
      // Clear any pending connection loss timeout
      if (this._connectionLossTimeout) {
        clearTimeout(this._connectionLossTimeout);
        this._connectionLossTimeout = null;
      }
      // Set a new timer: if no successful fetch in 10s, mark as lost
      this._connectionLossTimeout = setTimeout(() => {
        if (!this.backendReachable) return; // already marked as lost
        if (this._lastSuccessfulFetch && Date.now() - this._lastSuccessfulFetch > 10000) {
          this.backendReachable = false;
          this.addToast('error', 'Connection to gateway lost');
        }
      }, 10000);
    },

    _handleFetchError(err) {
      // Called when any fetch fails (timeout, network error, etc.)
      // If this is the first error, mark connection as lost after 10s
      if (this.backendReachable && !this._connectionLossTimeout) {
        this._connectionLossTimeout = setTimeout(() => {
          if (this.backendReachable) {
            this.backendReachable = false;
            this.addToast('error', 'Connection to gateway lost');
          }
        }, 10000);
      }
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
      // If WiFi tab and networks exist, do silent poll; if empty, show scanning UI
      if (tab === 'wifi') {
        if (this.info.wifiConnection?.foundNetworks?.length) {
          this._silentWifiScan();  // Background poll only
        } else {
          this.requestWifiScan();  // Show "Scanning..." UI
        }
      }
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
      // Show dots if password is set, store actual password separately
      if (wc.wifiPassword) {
        this.passActual.wifiPass = wc.wifiPassword;
        this.form.wifiPass = '••••••••';
      } else {
        this.passActual.wifiPass = '';
        this.form.wifiPass = '';
      }

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
      // Show dots if password is set, store actual password separately (note: backend returns 'mqttPass' not 'mqttPassword')
      if (mqtt.mqttPass) {
        this.passActual.mqttPass = mqtt.mqttPass;
        this.form.mqttPass = '••••••••';
      } else {
        this.passActual.mqttPass = '';
        this.form.mqttPass = '';
      }
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
          wifiPASSsend: this.form.wifiPass !== '••••••••' ? this.form.wifiPass : this.passActual.wifiPass, // If user typed new value, use it; otherwise use saved password
        });
        
        // Backend reboots if in setup mode (WiFi AP active)
        this.willReboot = this.info.initMode === 1;
        this.drawerOpen = false;
        
        if (this.willReboot) {
          this.showRebootOverlay = true;
          this.rebootStatus = 'Device is rebooting...';
          this._waitForDeviceReconnect();
        } else {
          this._toast('WiFi settings saved', 'success');
        }
      } catch (e) {
        this._toast('Save failed: ' + e.message, 'error');
      }
    },

    async saveDtu() {
      if (this.info.protectSettings) return;
      try {
        // Store current display settings to detect changes
        const currentRemoteDisplay = !!this.info.dtuConnection?.dtuRemoteDisplay;
        const currentSolarMonitor = !!this.info.dtuConnection?.dtuRemoteDisplay_SolarMonitor;
        const currentBatteryMonitor = !!this.info.dtuConnection?.dtuRemoteDisplay_BatteryMonitor;
        
        const newRemoteDisplay = (this.form.remoteDisplay && !this.form.remoteSummary && !this.form.batteryMonitor);
        const newSolarMonitor = (!this.form.remoteDisplay && this.form.remoteSummary);
        const newBatteryMonitor = (!this.form.remoteDisplay && this.form.batteryMonitor);
        
        // Backend reboots if any display setting changed
        this.willReboot = (currentRemoteDisplay !== newRemoteDisplay) || 
                         (currentSolarMonitor !== newSolarMonitor) || 
                         (currentBatteryMonitor !== newBatteryMonitor);
        
        await this._post('/updateDtuSettings', {
          dtuHostIpDomainSend:         this.form.dtuIp,
          dtuDataCycleSend:            this.form.dtuCycle,
          dtuCloudPauseSend:           this.form.dtuCloudPause ? '1' : '0',
          remoteDisplayActiveSend:            newRemoteDisplay ? '1' : '0',
          remoteSummaryDisplayActiveSend:     newSolarMonitor ? '1' : '0',
          remoteBatteryDisplayActiveSend:     newBatteryMonitor ? '1' : '0',
        });
        
        this.drawerOpen = false;
        
        if (this.willReboot) {
          this.showRebootOverlay = true;
          this.rebootStatus = 'Device is rebooting...';
          this._waitForDeviceReconnect();
        } else {
          this._toast('DTU settings saved', 'success');
          this._fetchInfo();
        }
      } catch (e) {
        this._toast('Save failed: ' + e.message, 'error');
      }
    },

    isSettingsProtected() {
      return !!this.info.protectSettings;
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
          mqttPasswordSend:           this.form.mqttPass !== '••••••••' ? this.form.mqttPass : this.passActual.mqttPass, // If user typed new value, use it; otherwise use saved password
          mqttMainTopicSend:          this.form.mqttTopic,
          mqttHAautoDiscoveryONSend:  this.form.mqttHA ? '1' : '0',
        });
        
        // Backend will reboot if MQTT is being enabled
        this.willReboot = this.form.mqttActive;
        
        // Show reboot/apply overlay
        this.drawerOpen = false;
        this.showRebootOverlay = true;
        if (this.willReboot) {
          this.rebootStatus = 'Device is rebooting...';
        } else {
          this.rebootStatus = 'Applying settings...';
        }
        
        this._waitForDeviceReconnect();
        
      } catch (e) {
        this._toast('Save failed: ' + e.message, 'error');
      }
    },

    async _waitForDeviceReconnect() {
      let attempts = 0;
      const maxAttempts = 150; // 150 attempts × 500ms = 75 seconds max (accounts for WiFi reconnection delays)
      const minDisplayTime = 2000; // Show overlay for at least 2 seconds after device comes back
      const initialDelayTime = 4000; // Backend reboot timer is 2-3s, device needs ~1-2s to fully restart, so 4s minimum
      const startTime = Date.now();
      
      const checkDevice = async () => {
        attempts++;
        const elapsed = Math.ceil((Date.now() - startTime) / 1000);
        this.rebootStatus = `${this.willReboot ? 'Rebooting' : 'Applying'}... (${elapsed}s)`;
        
        try {
          // Try to fetch info.json - if successful, device is back
          const res = await fetch('/api/info.json', { cache: 'no-store' });
          if (res.ok) {
            // Device is back online!
            // But respect minimum display time
            const displayedFor = Date.now() - startTime;
            if (displayedFor < minDisplayTime) {
              setTimeout(() => {
                this.showRebootOverlay = false;
                this._toast('Settings applied successfully!', 'success');
                this._fetchInfo();
              }, minDisplayTime - displayedFor);
              return;
            }
            
            this.showRebootOverlay = false;
            this._toast('Settings applied successfully!', 'success');
            // Trigger fresh data fetch
            this._fetchInfo();
            return;
          }
        } catch (e) {
          // Expected while device reboots or WiFi reconnects
        }
        
        if (attempts < maxAttempts) {
          setTimeout(checkDevice, 500);
        } else {
          // Timeout - device didn't come back within 75 seconds
          this.showRebootOverlay = false;
          this._toast('Device offline for too long. Please check WiFi connection and refresh manually.', 'error');
        }
      };
      
      // Wait longer before first check to account for backend reboot delay + device restart
      setTimeout(checkDevice, initialDelayTime);
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
      console.log('[UPDATE] Starting online update...');
      this.updateProgress = 0;
      this._updateTimeout = 60;
      this._updateStatus = 'preparing';
      try {
        console.log('[UPDATE] Posting to /doupdate');
        await this._post('/doupdate', {});
        console.log('[UPDATE] Post successful, starting progress poll');
        this._pollUpdateProgress();
      } catch (e) {
        console.error('[UPDATE] Update failed:', e);
        this._toast('Update failed: ' + e.message, 'error');
        this.updateProgress = -1;
        this._updateTimeout = 0;
        this._updateStatus = '';
      }
    },

    manualFwSelected(event) {
      this.fwFile = event.target.files[0] ?? null;
      console.log('[UPDATE] File selected:', this.fwFile?.name);
    },

    async uploadManualFirmware() {
      console.log('[UPDATE] Starting manual firmware upload...');
      if (!this.fwFile) return;
      const fd = new FormData();
      fd.append('update', this.fwFile);
      this.updateProgress = 0;
      this._updateTimeout = 60;
      this._updateStatus = 'preparing';
      
      // Start polling immediately - don't wait for upload to complete
      console.log('[UPDATE] Starting progress poll immediately');
      this._pollUpdateProgress();
      
      // Send upload in background - don't await it
      try {
        console.log('[UPDATE] Uploading file:', this.fwFile.name);
        fetch('/doupdate', { method: 'POST', body: fd })
          .then(res => {
            console.log('[UPDATE] Upload response received:', res.status);
            if (!res.ok) throw new Error('Upload returned: ' + res.statusText);
            console.log('[UPDATE] Upload completed successfully');
          })
          .catch(e => {
            console.error('[UPDATE] Upload error:', e);
            // Poll will catch the error state via /updateState
          });
      } catch (e) {
        console.error('[UPDATE] Upload failed to start:', e);
        this._toast('Upload failed: ' + e.message, 'error');
        this.updateProgress = -1;
        this._updateTimeout = 0;
        this._updateStatus = '';
        this.fwFile = null;
      }
    },

    _pollUpdateProgress() {
      console.log('[UPDATE] Starting poll for firmware update progress...');
      const MAX_TIMEOUT_SEC = 60;
      let timeoutCounter = MAX_TIMEOUT_SEC;
      this._updateTimeout = MAX_TIMEOUT_SEC;
      
      // Start timeout countdown (every second)
      const timeoutInterval = setInterval(() => {
        timeoutCounter--;
        this._updateTimeout = timeoutCounter;
        console.log('[UPDATE] Timeout countdown:', timeoutCounter + 's');
        
        if (timeoutCounter <= 0) {
          console.log('[UPDATE] Timeout reached - clearing intervals');
          clearInterval(timeoutInterval);
          clearInterval(this._updateInterval);
          this._toast('⏱ Update timeout (60s) — restoring control…', 'warn');
          this.updateProgress = -1;
          this._updateTimeout = 0;
          this._updateStatus = '';
          this.fwFile = null;
        }
      }, 1000);
      
      // Poll update state (every 500ms)
      this._updateInterval = setInterval(async () => {
        try {
          const s = await this._get('/updateState');
          console.log('[UPDATE] API Response:', s);
          
          this.updateProgress = s.updateProgress ?? 0;
          this._updateStatus = s.updateStateStr ?? 'unknown';
          
          console.log('[UPDATE] Progress:', this.updateProgress + '%, Status:', this._updateStatus);
          
          // Check for error state
          if (s.updateState === 6) { // UPDATE_STATE_FAILED = 6
            console.log('[UPDATE] Error state detected!');
            clearInterval(timeoutInterval);
            clearInterval(this._updateInterval);
            this._toast('✗ Update failed — check serial logs for details', 'error');
            this.updateProgress = -1;
            this._updateTimeout = 0;
            this._updateStatus = '';
            this.fwFile = null;
            return;
          }
          
          // Update complete when progress >= 100 and updateRunning is false
          if (s.updateProgress >= 100 && !s.updateRunning) {
            console.log('[UPDATE] Update complete!');
            clearInterval(timeoutInterval);
            clearInterval(this._updateInterval);
            this._toast('✓ Update complete — reloading in 3 seconds…', 'success');
            
            // Reload after brief delay to let user see completion message
            setTimeout(() => {
              this.updateProgress = -1;
              this._updateTimeout = 0;
              this._updateStatus = '';
              this.fwFile = null;  // Clear filename after update
              location.reload();
            }, 3000);
          }
        } catch (e) {
          // Network errors during poll don't immediately fail - timeout handles recovery
          console.warn('[UPDATE] Poll error:', e);
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
      const paramNames = { dtuGw: 'rebootDtuGw', dtu: 'rebootDtu', mi: 'rebootMi' };
      const url = endpoints[this.rebootTarget];
      if (!url) return;
      try {
        // Send the required parameter (e.g., rebootDtuGw=1)
        const params = {};
        params[paramNames[this.rebootTarget]] = '1';
        await this._post(url, params);
        this._toast(`${this.rebootLabel()} reboot initiated`, 'warn');
        this.rebootTarget = null;
      } catch (e) {
        this._toast(`${this.rebootLabel()} reboot failed: ${e.message}`, 'error');
      }
    },

    // ── WiFi scan ───────────────────────────────────────────────────
    // Silent scan: background poll when networks already exist (no UI changes)
    async _silentWifiScan() {
      try {
        await this._get('/getWifiNetworks');
        
        // Poll info until scan complete (max 15 s) - no UI flag set, runs silently
        let elapsed = 0;
        const poller = setInterval(async () => {
          elapsed += 250;
          await this._fetchInfo();
          if (!this.info.wifiConnection?.wifiScanIsRunning || elapsed >= 15000) {
            clearInterval(poller);
          }
        }, 250);
      } catch (_) {
        // Fail silently
      }
    },

    // User-initiated scan: clears networks and shows "Scanning..." UI
    async requestWifiScan() {
      try {
        // Ensure wifiConnection object exists, then clear networks and set scanning flag
        if (!this.info.wifiConnection) {
          this.info.wifiConnection = { foundNetworks: [], wifiScanIsRunning: 0 };
        }
        this.info.wifiConnection.foundNetworks = [];
        this.info.wifiConnection.wifiScanIsRunning = 1;
        this._wifiScanInitiated = true; // Mark that EXPLICIT scan was requested
        await this._get('/getWifiNetworks');

        // Poll info until scan complete (max 15 s)
        let elapsed = 0;
        const poller = setInterval(async () => {
          elapsed += 250;
          await this._fetchInfo();
          if (!this.info.wifiConnection?.wifiScanIsRunning || elapsed >= 15000) {
            clearInterval(poller);
            this._wifiScanInitiated = false; // Scan complete
          }
        }, 250);
      } catch (_) { 
        this._wifiScanInitiated = false; // Clear flag on error
      }
    },

    // ── Setup mode (first-launch WiFi configuration) ─────────────────
    _scanWifiNetworks() {
      // Called when entering setup mode to scan available networks
      this.setupLoading = true;
      this.requestWifiScan().then(() => {
        this.setupLoading = false;
      });
    },

    async setupConnectWifi() {
      if (!this.setupSelectedSSID || !this.setupPassword) {
        this._toast('Please select a network and enter password', 'error');
        return;
      }

      this.setupLoading = true;
      try {
        await this._post('/updateWifiSettings', {
          wifiSSIDsend: this.setupSelectedSSID,
          wifiPASSsend: this.setupPassword,
        });
        this._toast('WiFi settings saved. Reconnecting...', 'success');

        // Wait for device to reconnect, then exit setup mode
        setTimeout(() => {
          this.setupMode = false;
          this.setupSelectedSSID = '';
          this.setupPassword = '';
        }, 3000);
      } catch (e) {
        this._toast('Failed to save WiFi: ' + e.message, 'error');
        this.setupLoading = false;
      }
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
      // Swap between actual password and dots
      if (this.passVis[field]) {
        // Showing password - only swap if currently showing dots
        // This preserves user-typed changes and doesn't overwrite unsaved input
        if (this.form[field] === '••••••••') {
          this.form[field] = this.passActual[field];
        }
      } else {
        // Hiding password - swap actual for dots (only if we have a stored password)
        if (this.passActual[field]) {
          this.form[field] = '••••••••';
        }
      }
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
