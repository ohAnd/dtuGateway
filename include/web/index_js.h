static const char *index_js PROGMEM = R"=====(
// ============================================================
// Minimal jQuery-compatible polyfill (replaces ~90KB jquery)
// ============================================================
(function (win) {
    function JQ(sel) {
        if (!(this instanceof JQ)) {
            if (typeof sel === 'function') {
                if (document.readyState === 'loading')
                    document.addEventListener('DOMContentLoaded', sel);
                else sel();
                return;
            }
            return new JQ(sel);
        }
        if (sel === document || sel === window || (sel && sel.nodeType)) {
            this._e = [sel];
        } else if (typeof sel === 'string') {
            this._e = Array.prototype.slice.call(document.querySelectorAll(sel));
        } else {
            this._e = [];
        }
        this.length = this._e.length;
    }
    var p = JQ.prototype;
    p.each = function (fn) { this._e.forEach(function (el, i) { fn.call(el, i, el); }); return this; };
    p.map  = function (fn) { this._e.forEach(function (el, i) { fn.call(el, i, el); }); return this; };
    p.get  = function (i)  { return this._e[i]; };
    p.html = function (v)  { return v === undefined ? (this._e[0] ? this._e[0].innerHTML  : '') : this.each(function () { this.innerHTML  = v; }); };
    p.text = function (v)  { return v === undefined ? (this._e[0] ? this._e[0].textContent : '') : this.each(function () { this.textContent = v; }); };
    p.val  = function (v)  { return v === undefined ? (this._e[0] ? this._e[0].value       : '') : this.each(function () { this.value       = v; }); };
    p.attr = function (n, v) {
        if (v === undefined) return this._e[0] ? this._e[0].getAttribute(n) : null;
        return this.each(function () {
            if (v === null || v === false) this.removeAttribute(n); else this.setAttribute(n, v);
        });
    };
    p.prop = function (n, v) {
        if (v === undefined) return this._e[0] ? this._e[0][n] : undefined;
        return this.each(function () { this[n] = v; });
    };
    p.css = function (n, v) {
        if (typeof n === 'object') {
            var s = this; Object.keys(n).forEach(function (k) { s.css(k, n[k]); }); return this;
        }
        if (v === undefined) return this._e[0] ? this._e[0].style[n] : '';
        return this.each(function () { this.style[n] = v; });
    };
    p.width = function (v) {
        if (v === undefined) return this._e[0] ? this._e[0].offsetWidth : 0;
        return this.each(function () { this.style.width = (typeof v === 'number' ? v + 'px' : v); });
    };
    p.show  = function () { return this.each(function () { this.style.display = ''; if (getComputedStyle(this).display === 'none') this.style.display = 'block'; }); };
    p.hide  = function () { return this.each(function () { this.style.display = 'none'; }); };
    p.addClass    = function (c) { return this.each(function () { this.classList.add(c); }); };
    p.removeClass = function (c) { return this.each(function () { this.classList.remove(c); }); };
    p.toggleClass = function (c) { return this.each(function () { this.classList.toggle(c); }); };
    p.is = function (sel) { return this._e[0] ? this._e[0].matches(sel) : false; };
    p.closest = function (sel) {
        var el = this._e[0];
        while (el && el !== document) {
            if (el.matches && el.matches(sel)) return new JQ(el);
            el = el.parentElement;
        }
        var r = new JQ(null); r._e = []; r.length = 0; return r;
    };
    p.find = function (sel) {
        var res = []; this._e.forEach(function (el) {
            Array.prototype.push.apply(res, el.querySelectorAll(sel));
        });
        var r = new JQ(null); r._e = res; r.length = res.length; return r;
    };
    p.append = function (html) {
        return this.each(function () {
            if (typeof html === 'string') this.insertAdjacentHTML('beforeend', html);
            else if (html && html.nodeType) this.appendChild(html);
        });
    };
    p.empty  = function () { return this.each(function () { this.innerHTML = ''; }); };
    p.focus  = function () { if (this._e[0]) this._e[0].focus(); return this; };
    p.on = function (evt, sel, fn) {
        if (typeof sel === 'function') { fn = sel; sel = null; }
        return this.each(function (i, el) {
            if (sel) {
                el.addEventListener(evt, function (e) {
                    var t = e.target;
                    while (t && t !== el) {
                        if (t.matches && t.matches(sel)) { fn.call(t, e); return; }
                        t = t.parentElement;
                    }
                });
            } else {
                el.addEventListener(evt, fn);
            }
        });
    };
    p.click  = function (fn) { return this.on('click',  fn); };
    p.change = function (fn) { return this.on('change', fn); };
    p.ready  = function (fn) {
        if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', fn);
        else fn();
        return this;
    };
    p.animate = function (props) {
        return this.each(function () {
            var el = this;
            Object.keys(props).forEach(function (k) {
                el.style.transition = k + ' 0.3s ease';
                el.style[k] = typeof props[k] === 'number' ? props[k] : props[k];
            });
        });
    };
    p.fadeOut = function (duration) {
        var ms = typeof duration === 'number' ? duration : 400;
        return this.each(function () {
            var el = this;
            el.style.transition = 'opacity ' + ms + 'ms';
            el.style.opacity = '0';
            setTimeout(function () { el.style.display = 'none'; el.style.opacity = ''; el.style.transition = ''; }, ms);
        });
    };
    JQ.ajax = function (opts) {
        var method = (opts.type || opts.method || 'GET').toUpperCase();
        var xhr = new XMLHttpRequest();
        xhr.open(method, opts.url, true);
        if (opts.timeout) xhr.timeout = opts.timeout;
        xhr.ontimeout = opts.error || function () {};
        xhr.onerror   = opts.error || function () {};
        xhr.onload = function () {
            if (xhr.status >= 200 && xhr.status < 300) {
                var data = xhr.responseText;
                try { data = JSON.parse(data); } catch (e) {}
                if (opts.success) opts.success(data);
            } else {
                if (opts.error) opts.error(xhr.status, xhr.statusText);
            }
        };
        if (opts.contentType && opts.contentType !== false)
            xhr.setRequestHeader('Content-Type', opts.contentType);
        xhr.send(opts.data || null);
    };
    win.$ = JQ;
    win.jQuery = JQ;
}(window));
// ============================================================

        let timerRemainingProgess = 0;
        let waitTime = 31000;
        let remainingTime = waitTime;

        let timerInfoUpdate = 0;
        let cacheInfoData = {};
        let cacheData = {};
        let cacheDtuData = {};

        $(document).ready(function () {
            console.log("document loading done");
            initValueChanges();
            // first data refresh
            getDataValues();
            getInfoValues();
            requestVersionData();
            getDtuDataValues();

            window.setInterval(function () {
                getDataValues();
            }, 1000);

            timerInfoUpdate = window.setInterval(function () {
                getInfoValues();
            }, 5000);

            timerDtuDataUpdate = window.setInterval(function () {
                getDtuDataValues();
            }, 7500);

            // check every minute (62,5s) for an available update
            window.setInterval(function () {
                requestVersionData();
            }, 300000);

            timerRemainingProgess = window.setInterval(function () {
                remainingResponse();
            }, 100);
        });

        // switching in popups between tabs
        $(document).on("click", ".popupHeaderTabs>div", function (event) {
            $('.popupHeaderTabs>div').each(function () {
                $(this).removeClass("selected");
                if ($(this).html() == event.target.innerHTML) $(this).addClass("selected");
            });

            $('.popup>.popupContent').each(function () {
                $(this).css("display", "none");
                if ($(this).attr("id") == event.target.innerHTML) $(this).css("display", "block");;
            });
        })

        // grey'ing the bindings sections according to activation
        $("input[type='checkbox']").change(function () {
            if ($(this).closest('div').get(0).id != '' && !$(this).closest('div').get(0).id.startsWith('remote')) {
                if (this.checked) {
                    $(this).closest('div').css('color', '');
                } else {
                    $(this).closest('div').css('color', 'grey');
                }
            }
        });

        // grey'ing the dtu settings if remote display is active
        $("input[type='checkbox'][id='remoteDisplayActive']").change(function () {
            if (this.checked) {
                $("#dtuSettings").hide();
                $("#openhabSection").hide();
                $("#mqttSelect").hide();
                $("#mqttSectionComment").text("remote display active - getting all data from a specific MQTT broker");
                $("#mqttMainTopicComment").html("MQTT main topic of the source dtuGatway: <br><small>(remote dtuGateway subribes to source dtuGateway topics)</small>");
                $("#mqttHAautoDiscovery").hide();
                $("#remoteSummaryDisplaySettings").hide();
                $("remoteSummaryDisplayActive").prop("checked", false);
            } else {
                $("#dtuSettings").show();
                $("#openhabSection").show();
                $("#mqttSelect").show();
                $("#mqttSectionComment").text("publish all data to a specific MQTT broker and subscribing to the requested powersetting");
                $("#mqttMainTopicComment").html("MQTT main topic for this dtu: <br><small>(e.g. dtu_12345678 will appear as 'dtu_12345678/grid/U' in the broker - has to be unique in your setup)</small>");
                $("#mqttHAautoDiscovery").show();
                $("#remoteSummaryDisplaySettings").show();
            }
        });

        // grey'ing the dtu settings if remote display is active
        $("input[type='checkbox'][id='remoteSummaryDisplayActive']").change(function () {
            if (this.checked) {
                $("#dtuSettings").hide();
                $("#openhabSection").hide();
                $("#mqttSelect").hide();
                $("#mqttSectionComment").text("remote summary display (Solar Monitor) active - getting the data from a specific MQTT broker and topic path");
                $("#mqttMainTopicComment").html("MQTT main topic for the solar monitor data: <br><small>(main pv power and yield of the day - needed specific topics in this path see readme)</small>");
                $("#mqttHAautoDiscovery").hide();
                $("#remoteDisplaySettings").hide();
                $("remoteDisplayActive").prop("checked", false);
            } else {
                $("#dtuSettings").show();
                $("#openhabSection").show();
                $("#mqttSelect").show();
                $("#mqttSectionComment").text("publish all data to a specific MQTT broker and subscribing to the requested powersetting");
                $("#mqttMainTopicComment").html("MQTT main topic for this dtu: <br><small>(e.g. dtu_12345678 will appear as 'dtu_12345678/grid/U' in the broker - has to be unique in your setup)</small>");
                $("#mqttHAautoDiscovery").show();
                $("#remoteDisplaySettings").show();
            }
        });

        var show = function (id) {
            console.log("show " + id)
            $(id).show(200);
            if (id == '#changeSettings') {
                getWIFIdata();
                getDTUdata();
                getBindingsData();
            }
            if (id == '#updatePowerLimit') {
                getPowerLimitData();
                $('#powerLimitSetNew').focus();
            }
            if (id == '#warningOverview') {
                $('#warningOverview').css('display', 'flex');
            }
            if (id == '#summaryDisplay') {
                $('#summaryDisplay').css('display', 'flex');
            }
        }

        var hide = function (id) {
            console.log("hide " + id)
            $(id).hide(200);
        }

        function checkInitToSettings(data) {
            // if not configured then start directly with settings dialogue
            var startUptext = "settings --- startup config mode";
            if (data.initMode == 1 && $('#popHeadTitle').text() != startUptext) {
                show('#changeSettings');
                remainingTime = 0.1; // no countdown on top of the site
                $('#popHeadTitle').text(startUptext);
                // disable close button
                $('#btnSettingsClose').css('opacity', '0.3');
                $('#btnSettingsClose').attr('onclick', "")
            }
            if(data.protectSettings) {
                // if settings are protected then disable the save button
                $('#btnSaveWifiSettings').css('opacity', '0.3');
                $('#btnSaveWifiSettings').attr('onclick', "");
                $('#btnSaveDtuSettings').css('opacity', '0.3');
                $('#btnSaveDtuSettings').attr('onclick', "");
                $('#btnSaveBindingsSettings').css('opacity', '0.3');
                $('#btnSaveBindingsSettings').attr('onclick', "");
                $('#settings_message').show();
                $('#settings_message').html("settings read-only - use serial console (USB connection) to unlock");
                
            } else {
                // enable save button
                $('#btnSaveWifiSettings').css('opacity', '1');
                $('#btnSaveWifiSettings').attr('onclick', "changeWifiData()");
                $('#btnSaveDtuSettings').css('opacity', '1');
                $('#btnSaveDtuSettings').attr('onclick', "changeDtuData()");
                $('#btnSaveBindingsSettings').css('opacity', '1');
                $('#btnSaveBindingsSettings').attr('onclick', "changeBindingsData()");
                $('#settings_message').hide();
                $('#settings_message').html("");               
            }
        }

        function remainingResponse() {
            if (remainingTime > 0) {
                var remainingTime_width = (remainingTime / waitTime) * 100;
                $('#updateTime').width(remainingTime_width + "%");
            }
            remainingTime = remainingTime - 100;
            if (remainingTime < 0) {
                remainingTime = -0.1;
            }
        }

        function refreshData(data) {

            $('#gwtime').html(getTime(data.localtime));
            $('#gwtime2').html(getTime(data.localtime, "date"));


            if ($('#last_response').html() != getTime(data.lastResponse)) {
                remainingTime = waitTime;
            }
            $('#last_response').html(getTime(data.lastResponse));

            $('#gwtime_small').html(getTime(data.localtime));
            $('#gwNTPtime').html(getTime(data.ntpStamp));

            $('#gwStartTime').html(getTime(data.starttime, "dateShort") + "&nbsp;" + getTime(data.starttime, "timeShort"));

            $('#uptime').html(getTime(data.lastResponse));

            checkValueUpdate('#pv0_power', ((isNaN(data.pv0.p)) ? "--.-" : (data.pv0.p).toFixed(1)), "W");
            checkValueUpdate('#pv0_voltage', (data.pv0.v).toFixed(1), "V");
            checkValueUpdate('#pv0_current', (data.pv0.c).toFixed(1), "A");
            checkValueUpdate('#pv0_daily_energy', (data.pv0.dE).toFixed(3));
            checkValueUpdate('#pv0_total_energy', (data.pv0.tE).toFixed(3));


            checkValueUpdate('#pv1_power', ((isNaN(data.pv1.p)) ? "--.-" : (data.pv1.p).toFixed(1)), "W");
            checkValueUpdate('#pv1_voltage', (data.pv1.v).toFixed(1), " V");
            checkValueUpdate('#pv1_current', (data.pv1.c).toFixed(1), "A");
            checkValueUpdate('#pv1_daily_energy', (data.pv1.dE).toFixed(3));
            checkValueUpdate('#pv1_total_energy', (data.pv1.tE).toFixed(3));

            checkValueUpdate('#grid_power', ((isNaN(data.grid.p)) ? "--.-" : (data.grid.p).toFixed(1)), "W");
            checkValueUpdate('#grid_voltage', (data.grid.v).toFixed(1) + "V");
            checkValueUpdate('#grid_current', (data.grid.c).toFixed(1) + "A");
            checkValueUpdate('#grid_daily_energy', (data.grid.dE).toFixed(3));
            checkValueUpdate('#grid_total_energy', (data.grid.tE).toFixed(3));

            checkValueUpdate('#powerLimitSet', ((isNaN(data.inverter.pLimSet)) ? "--.-" : data.inverter.pLimSet));
            checkValueUpdate('#powerLimit', data.inverter.pLim);
            checkValueUpdate('#powerLimitNow', data.inverter.pLim);

            checkValueUpdate('#inverterTemp', (data.inverter.temp).toFixed(1), "'C");

            checkValueUpdate('#grid_power2', ((isNaN(data.grid.p)) ? "--.-" : (data.grid.p).toFixed(1)), "W");
            checkValueUpdate('#grid_daily_energy2', (data.grid.dE).toFixed(3));

            var dtuConnect = "";
            switch (data.dtuConnState) {
                case 0:
                    dtuConnect = "offline";
                    break;
                case 1:
                    dtuConnect = "connected";
                    break;
                case 2:
                    dtuConnect = "cloud pause";
                    break;
                case 3:
                    dtuConnect = "try reconnect";
                    break;
                case 4:
                    dtuConnect = "dtu rebooting";
                    break;
                case 5:
                    dtuConnect = "connect error";
                    break;
                case 6:
                    dtuConnect = "stopped";
                    break;
                case 7:
                    dtuConnect = "reboot inverter";
                    break;
                default:
                    dtuConnect = "not known";
            }
            checkValueUpdate('#dtu_connect_state', dtuConnect);
            var dtuState = "";
            switch (data.dtuErrorState) {
                case 0:
                    dtuState = "ok";
                    break;
                case 1:
                    dtuState = "no time";
                    break;
                case 2:
                    dtuState = "time Error";
                    break;
                case 3:
                    dtuState = "data error";
                    break;
                case 4:
                    dtuState = "last TX done";
                    break;
                default:
                    dtuState = "no info";
            }
            checkValueUpdate('#dtu_error_state', dtuState);

            if (data.inverter.active == 0) {
                $('#infoInveterOff').show();
            } else {
                $('#infoInveterOff').hide();
            }

            return true;
        }

        function refreshInfo(data) {

            var wifiGWPercent = Math.round(data.wifiConnection.rssiGW);
            $('#rssitext_local').html(wifiGWPercent + '%');
            var wifiDTUPercent = Math.round(data.dtuConnection.dtuRssi);
            $('#rssitext_dtu').html(wifiDTUPercent + '%');

            $('#firmware').html("fw version: " + data.firmware.version + "<br><span style=\"font-size: smaller;\">DTU: " + data.dtuConnection.deviceData.dtu_version_string + " | MI: " + data.dtuConnection.deviceData.inverter_version_string + "</span>");
            $('#chipType').html("controller architecture type: " + data.chipType);

            if (data.firmware.selectedUpdateChannel == 0) { $("#relChanStable").addClass("selected"); $("#relChanSnapshot").removeClass("selected"); }
            else { $("#relChanStable").removeClass("selected"); $("#relChanSnapshot").addClass("selected"); }

            // setting timer value according to user setting
            waitTime = data.dtuConnection.dtuDataCycle * 1000;

            checkValueUpdate('#dtu_reboots_no', data.dtuConnection.dtuResetRequested);

            var gridP = ((isNaN(cacheData.pv0.p)) ? "--.-" : (cacheData.grid.p).toFixed(0));

            let inverter_model = data.dtuConnection.deviceData.inverter_model;
            if(inverter_model == "--") {
                inverter_model = "HMS-xxxW-xT";
            }
            if (data.dtuConnection.dtuRemoteDisplay) {
                $("#titleHeader").text("Hoymiles " + inverter_model + " - Remote Display");
                $("#title").text(gridP + "W - dtuGateway - Remote Display");
            } else {
                $("#titleHeader").text("Hoymiles " + inverter_model + " - Gateway");
                $("#title").text(gridP + "W - dtuGateway");
            }

            if (data.dtuConnection.dtuRemoteSummaryDisplay) {
                $('#summaryDisplay').css('display', 'flex');
                $('#summaryDisplay').css('flex-direction', 'column');
                $('#summaryDisplay').css('align-items', 'center');
                $('#summaryDisplay').css('justify-content', 'center');
            } else {
                $('#summaryDisplay').css('display', 'none');
            }

            return true;
        }

        function getWIFIdata() {
            // 
            $('#btnSaveWifiSettings').css('opacity', '1.0');
            $('#btnSaveWifiSettings').attr('onclick', "changeWifiData();")

            requestWifiScan();
            cacheInfoData.wifiConnection.wifiScanIsRunning = 1;

            let intervalId = setInterval(() => {
                console.log("Interval action");
                getInfoValues();
                displayWIFIdata();
                if (cacheInfoData.wifiConnection.wifiScanIsRunning == 0) {
                    clearInterval(intervalId);
                    $('#wifiSearchBox').hide();
                    $('#wifiContent').css('opacity', '1.0');
                    //console.log("Interval ended due to scan ends");
                }
            }, 250);

            setTimeout(() => {
                clearInterval(intervalId);
                $('#wifiSearchBox').hide();
                $('#wifiContent').css('opacity', '1.0');
                //console.log("Interval ended after 15 seconds");
            }, 15000);

            displayWIFIdata();
        }

        function displayWIFIdata() {
            // opacity until wifi scan done
            if (cacheInfoData.wifiConnection.wifiScanIsRunning == 1) {
                $('#wifiContent').css('opacity', '0.3');
                $('#wifiSearchBox').show();
            }

            wifiData = cacheInfoData.wifiConnection;
            wifiDataNw = wifiData.foundNetworks;
            // get networkdata
            //console.log("wifi scan: " + wifiData.wifiScanIsRunning);
            $('#wifiSSID').html(wifiData.wifiSsid);
            $('#wifiPASSsend').val(wifiData.wifiPassword);
            $('#networkCount').html(wifiData.networkCount);
            $('#networks').empty();
            wifiDataNw.sort(compare);
            for (let index = 0; index < wifiData.networkCount; index++) {
                var selected = "";
                if ($('#wifiSSIDsend').val() == wifiDataNw[index].name) selected = "checked";
                $('#networks').append('<label><input type="radio" id="wifi' + index + '" name="wifiselect" value="wifi' + index + '" style="width: auto; height: auto; display:inline" ' + selected + '> ' + wifiDataNw[index].wifi + ' % - ch: ' + wifiDataNw[index].chan + ' - ' + wifiDataNw[index].name + '</label><br>');
            }

            $('input[type=radio][name=wifiselect]').change(function () {
                console.log("select: " + this.value + " - " + (this.value).split("wifi")[1]);
                $('#wifiSSIDsend').val(wifiDataNw[(this.value).split("wifi")[1]].name);
            });
        }

        function getDTUdata() {
            // 
            $('#btnSaveDtuSettings').css('opacity', '1.0');
            $('#btnSaveDtuSettings').attr('onclick', "changeDtuData();")

            dtuData = cacheInfoData.dtuConnection;

            // get networkdata
            $('#dtuHostIpDomain').val(dtuData.dtuHostIpDomain);
            $('#dtuDataCycle').val(dtuData.dtuDataCycle);
            if (dtuData.dtuCloudPause) {
                $('#dtuCloudPause').prop("checked", true);
            } else {
                $('#dtuCloudPause').prop("checked", false);
            }

            if (dtuData.dtuRemoteDisplay) {
                $('#remoteDisplayActive').prop("checked", true).change();
            } else {
                $('#remoteDisplayActive').prop("checked", false).change();
            }

            if (dtuData.dtuRemoteSummaryDisplay) {
                $('#remoteSummaryDisplayActive').prop("checked", true).change();
            } else {
                $('#remoteSummaryDisplayActive').prop("checked", false).change();
            }

            $('#dtuSsid').val(dtuData.dtuSsid);
            $('#dtuPassword').val(dtuData.dtuPassword);

        }

        function getBindingsData() {
            // active
            $('#btnSaveDtuSettings').css('opacity', '1.0');
            $('#btnSaveDtuSettings').attr('onclick', "changeDtuData();")

            ohData = cacheInfoData.openHabConnection;
            mqttData = cacheInfoData.mqttConnection;

            // get networkdata
            if (ohData.ohActive) {
                $('#openhabActive').prop("checked", true);
                $('#openhabSection').css('color', '');
            } else {
                $('#openhabActive').prop("checked", false);
                $('#openhabSection').css('color', 'grey');
            }
            $('#openhabIP').val(ohData.ohHostIp);
            $('#ohItemPrefix').val(ohData.ohItemPrefix);

            if (mqttData.mqttActive) {
                $('#mqttActive').prop("checked", true);
                $('#mqttSection').css('color', '');
            } else {
                $('#mqttActive').prop("checked", false);
                $('#mqttSection').css('color', 'grey');
            }
            if (mqttData.mqttUseTLS) {
                $('#mqttUseTLS').prop("checked", true);
            } else {
                $('#mqttUseTLS').prop("checked", false);
            }
            $('#mqttIP').val(mqttData.mqttIp + ":" + mqttData.mqttPort);
            $('#mqttUser').val(mqttData.mqttUser);
            $('#mqttPassword').val(mqttData.mqttPass);
            $('#mqttMainTopic').val(mqttData.mqttMainTopic);

            if (mqttData.mqttHAautoDiscoveryON) {
                $('#mqttHAautoDiscoveryON').prop("checked", true);
            } else {
                $('#mqttHAautoDiscoveryON').prop("checked", false);
            }
        }

        $('.passcheck').click(function () {
            console.log("passcheck stat: " + $(this).attr("value") + " - id: " + $(this).attr("id"))
            if ($(this).attr("value") == 'invisible') {
                $('#wifiPASSsend').attr('type', 'text');
                $('#dtuPassword').attr('type', 'text');
                $('#mqttPassword').attr('type', 'text');
                $('.passcheck').attr('value', 'visibile');
                $('.passcheck').html("hide");
            } else {
                $('#wifiPASSsend').attr('type', 'password');
                $('#dtuPassword').attr('type', 'password');
                $('#mqttPassword').attr('type', 'password');
                $('.passcheck').attr('value', 'invisible');
                $('.passcheck').html("show");
            }
        });

        function getPowerLimitData() {
            // 
            $('#btnSetPowerLimit').css('opacity', '1.0');
            $('#btnSetPowerLimit').attr('onclick', "changePowerLimit();")

            // show last set value in input field
            $('#powerLimitSetNew').val(cacheData.inverter.pLimSet);
        }

        function changeWifiData() {
            var ssid = $('#wifiSSIDsend').val();
            var pwd = $('#wifiPASSsend').val();
            var data = {};
            data["wifiSSIDsend"] = ssid;
            data["wifiPASSsend"] = pwd;

            console.log("send to server: wifi: " + ssid + " - pass: " + pwd);

            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
                console.log("push: " + name);
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/updateWifiSettings", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            strResult = JSON.parse(xmlHttp.responseText);
            console.log("got from server: " + strResult);
            console.log("got from server - strResult.wifiSSIDUser: " + strResult.wifiSSIDUser + " - cmp with: " + ssid);
            console.log("got from server - strResult.wifiPassUser: " + strResult.wifiPassUser + " - cmp with: " + pwd);

            if (strResult.wifiSSIDUser == ssid && strResult.wifiPassUser == pwd) {
                console.log("check saved data - OK");
                showAlert('Wifi access data changed', 'connect to the choosen wifi and to the new ip within your network', 'alert-success');
            } else {
                showAlert('Some error occured!', 'change Wifi access data could not be saved. Please try again!', 'alert-danger');
            }

            $('#btnSaveWifiSettings').css('opacity', '0.3');
            $('#btnSaveWifiSettings').attr('onclick', "")

            hide('#changeSettings');
            return;
        }

        function changeDtuData() {
            var dtuHostIpDomainSend = $('#dtuHostIpDomain').val();
            var dtuDataCycleSend = $('#dtuDataCycle').val();
            if ($("#dtuCloudPause").is(':checked'))
                dtuCloudPauseSend = 1;
            else
                dtuCloudPauseSend = 0;
            if ($("#remoteDisplayActive").is(':checked'))
                remoteDisplayActiveSend = 1;
            else
                remoteDisplayActiveSend = 0;
            if ($("#remoteSummaryDisplayActive").is(':checked'))
                remoteSummaryDisplayActiveSend = 1;
            else
                remoteSummaryDisplayActiveSend = 0;

            var data = {};
            data["dtuHostIpDomainSend"] = dtuHostIpDomainSend;
            data["dtuDataCycleSend"] = dtuDataCycleSend;
            data["dtuCloudPauseSend"] = dtuCloudPauseSend;

            data["remoteDisplayActiveSend"] = remoteDisplayActiveSend;
            data["remoteSummaryDisplayActiveSend"] = remoteSummaryDisplayActiveSend;

            console.log("send to server: dtuHostIpDomain: " + dtuHostIpDomainSend + " dtuDataCycle: " + dtuDataCycleSend + " dtuCloudPause: " + dtuCloudPauseSend + " - remoteDisplayActive: " + remoteDisplayActiveSend + " - remoteSummaryDisplayActive: " + remoteSummaryDisplayActiveSend);

            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
                console.log("push: " + name);
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/updateDtuSettings", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            strResult = JSON.parse(xmlHttp.responseText);
            console.log("got from server: " + strResult);
            console.log("got from server - strResult.dtuHostIpDomain: " + strResult.dtuHostIpDomain + " - cmp with: " + dtuHostIpDomainSend);

            if (strResult.dtuHostIpDomain == dtuHostIpDomainSend) {
                console.log("check saved data - OK");
                showAlert('dtu connection settings changed', 'The new settings will be applied.', 'alert-success');
            } else {
                showAlert('Some error occured!', 'change dtu connection settings could not be saved. Please try again!', 'alert-danger');
            }

            //$('#btnSaveDtuSettings').css('opacity', '0.3');
            //$('#btnSaveDtuSettings').attr('onclick', "")

            hide('#changeSettings');
            return;
        }

        function changeBindingsData() {
            var openhabHostIpDomainSend = $('#openhabIP').val();
            var openhabPrefixSend = $('#ohItemPrefix').val();
            if ($("#openhabActive").is(':checked')) {
                openhabActiveSend = 1;
            } else {
                openhabActiveSend = 0;
            }

            var mqttIpPortString = $('#mqttIP').val().split(":");

            var mqttIpSend = mqttIpPortString[0];
            var mqttPortSend = "1883";
            if (mqttIpPortString[1] != undefined && !isNaN(mqttIpPortString[1])) {
                mqttPortSend = mqttIpPortString[1];
            }
            var mqttUseTLSSend = 0;
            var mqttUserSend = $('#mqttUser').val();
            var mqttPassSend = $('#mqttPassword').val();
            var mqttMainTopicSend = $('#mqttMainTopic').val();
            var mqttHAautoDiscoveryONSend = 0;

            if ($("#mqttActive").is(':checked')) {
                mqttActiveSend = 1;
            } else {
                mqttActiveSend = 0;
            }
            if ($("#mqttUseTLS").is(':checked')) {
                mqttUseTLSSend = 1;
            } else {
                mqttUseTLSSend = 0;
            }
            if ($("#mqttHAautoDiscoveryON").is(':checked')) {
                mqttHAautoDiscoveryONSend = 1;
            } else {
                mqttHAautoDiscoveryONSend = 0;
            }

            var data = {};
            data["openhabHostIpDomainSend"] = openhabHostIpDomainSend;
            data["openhabPrefixSend"] = openhabPrefixSend;
            data["openhabActiveSend"] = openhabActiveSend;

            data["mqttIpSend"] = mqttIpSend;
            data["mqttPortSend"] = mqttPortSend;
            data["mqttUserSend"] = mqttUserSend;
            data["mqttPassSend"] = mqttPassSend;
            data["mqttMainTopicSend"] = mqttMainTopicSend;
            data["mqttActiveSend"] = mqttActiveSend;
            data["mqttUseTLSSend"] = mqttUseTLSSend;
            data["mqttHAautoDiscoveryONSend"] = mqttHAautoDiscoveryONSend;


            console.log("send to server: openhabHostIpDomainSend: " + openhabHostIpDomainSend);

            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
                console.log("push: " + name + " - value: " + value);
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/updateBindingsSettings", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            strResult = JSON.parse(xmlHttp.responseText);
            console.log("got from server: " + strResult);
            console.log("got from server - strResult.dtuHostIpDomain: " + strResult.openhabHostIpDomain + " - cmp with: " + openhabHostIpDomainSend);

            if (strResult.openhabHostIpDomain == openhabHostIpDomainSend && strResult.mqttBrokerIpDomain == mqttIpSend && strResult.mqttBrokerUser == mqttUserSend) {
                console.log("check saved data - OK");
                showAlert('change bindings settings', 'Your settings were successfully saved and will be applied.', 'alert-success');
            } else {
                showAlert('Some error occured!', 'change bindings settings could not be saved. Please try again!', 'alert-danger');
            }

            hide('#changeSettings');
            return;
        }

        function rebootMi() {
            var data = {};
            data["rebootMi"] = true;
            console.log("send to server: rebootMi");
            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/rebootMi", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            hide('#rebootMi')
            return;
        }

        function rebootDtu() {
            var data = {};
            data["rebootDtu"] = true;
            console.log("send to server: rebootDtu");
            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/rebootDtu", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            hide('#rebootDtu')
            return;
        }

        function rebootDtuGw() {
            var data = {};
            data["rebootDtuGw"] = true;
            console.log("send to server: rebootDtuGw");
            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/rebootDtuGw", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            hide('#rebootDtuGw')
            return;
        }

        function changePowerLimit() {

            var powerLimitSend = $('#powerLimitSetNew').val();

            var data = {};
            data["powerLimitSend"] = powerLimitSend;

            console.log("send to server: powerLimitSend: " + powerLimitSend);

            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
                console.log("push: " + name + " - value: " + value);
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/updatePowerLimit", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            strResult = JSON.parse(xmlHttp.responseText);
            console.log("got from server: " + strResult);
            console.log("got from server - strResult.PowerLimit: " + strResult.PowerLimit + " - cmp with: " + powerLimitSend);

            hide('#updatePowerLimit');
            return;
        }

        function changeReleaseChannel(channel) {
            if (cacheInfoData.firmware.selectedUpdateChannel == channel) return;

            cacheInfoData.firmware.versionServer = "reloading";
            cacheInfoData.firmware.versiondateServer = "reloading";
            cacheInfoData.firmware.selectedUpdateChannel = channel;
            cacheInfoData.firmware.updateAvailable = 0;

            getVersionData(cacheInfoData);
            refreshInfo(cacheInfoData);

            clearInterval(timerInfoUpdate);
            timerInfoUpdate = window.setInterval(function () {
                getInfoValues();
            }, 7500);

            var data = {};
            data["releaseChannel"] = channel;

            console.log("send to server: releaseChannel: " + channel);

            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
                console.log("push: " + name);
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");

            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/updateOTASettings", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            try {
                strResult = '';//JSON.parse(xmlHttp.responseText);
                console.log("got from server: " + strResult);
            } catch (error) {
                console.log("error at request change release channel: " + error);
            }

            hide('#changeSettings');
            return;
        }

        function compare(a, b) {
            if (a.wifi > b.wifi) {
                return -1;
            }
            if (a.wifi < b.wifi) {
                return 1;
            }
            return 0;
        }

        function getVersionData(data) {
            if (data.firmware.selectedUpdateChannel == 1) { $('#firmwareVersionServer').html(data.firmware.versionServer); $('#builddateVersionServer').html(data.firmware.versiondateServer); }
            else { $('#firmwareVersionServer').html(data.firmware.versionServerRelease); $('#builddateVersionServer').html(data.firmware.versiondateServerRelease); }

            $('#firmwareVersion').html(data.firmware.version);
            $('#builddateVersion').html(data.firmware.versiondate);

            if (data.firmware.updateAvailable == 1) {
                $('#updateState').html("new update available");
                $('#btnUpdateStart').css('opacity', '1.0');
                $('#updateBadge').show();
                $('#btnUpdateStart').attr('onclick', "startUpdate()")
            } else {
                $('#updateState').html("no update available");
                $('#btnUpdateStart').css('opacity', '0.3');
                $('#updateBadge').hide();
                $('#btnUpdateStart').attr('onclick', "")
            }
        }

        function requestVersionData() {
            $.ajax({
                url: '/updateGetInfo',
                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 2500,
                success: function (data) {
                    console.log("requestVersionData - success");
                },
                error: function () {
                    console.log("requestVersionData - error");
                }
            });
        }

        function startUpdate() {
            hide('#updateMenu');
            show('#updateProgress');
            $('#newVersionProgress').html(cacheInfoData.firmware.versionServer);

            var timeoutStart = 50.0;
            var timeout = timeoutStart;
            var progress = 0;

            $('#updateProgressPercent').html("0 %");
            $('#updateTimeout').html("0 s");

            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("GET", "/updateRequest", false); // false for synchronous request
            xmlHttp.send(null);
            // //<!-- return xmlHttp.responseText; -->

            $('#btnUpdateStart').animate({ opacity: 0.3 });

            let timerTO = window.setInterval(function () {
                progress = (timeoutStart - timeout) / timeoutStart * 100;
                $('#progressbar').width(progress + "%");
                $('#updateProgressPercent').html(Math.round(progress) + " %");
                $('#updateTimeout').html(timeout.toFixed(0) + " s");

                console.log("check OTA progress - " + timeout);
                timeout = timeout - 0.25;
                if (timeout < 0 || cacheInfoData.updateAvailable == 0) {
                    clearInterval(timerTO);
                    window.location.href = "/";
                }
            }, 250);

            $('#btnUpdateStart').animate({ opacity: 1 });

            return;
        }

        function checkValueUpdate(elemId, value, unit = "") {
            if ($(elemId).html() != value + " " + unit) {
                $(elemId).html(value + " " + unit);
            }
            return true;
        }

        function changeUpdateType(checked) {
            if (checked) {
                $('#updateType').text("direct online update - ");
                $('#updateType').css('color', 'gray');
                $('#updateState').text(" currently no update available");
                $('#updateState').css('color', 'gray');
                $('#updateState').show();
                $('#autoUpdate').show();
                $('#updateManual').hide();
            } else {
                $('#updateType').text("manual update with firmware file");
                $('#updateType').css('color', '');
                $('#updateState').hide();
                $('#autoUpdate').hide();
                $('#updateManual').show();
            }
        }

        function showFileName(input) {
            var fileInput = document.getElementById('fileInput');
            var fileNameDisplay = document.getElementById('fileNameDisplay');
            if (fileInput.files.length > 0) {
                var fileName = fileInput.files[0].name;
                var fileSize = input.files[0].size / 1024 / 1024; // size in MB
                fileSize = fileSize.toFixed(2); // keeping two decimals
                // Display file size in the HTML
                fileName += ` (${fileSize} MB)`;

                $('#fileNameDisplay').html("<small>selected firmware file:</small> " + fileName);
                $('#manualUpdateStart').show();
            } else {
                fileNameDisplay.textContent = '';
            }
        }

        function updateManualWithFile() {
            var fileInput = document.getElementById('fileInput');
            if (fileInput.files.length === 0) {
                console.log("No file selected.");
                return;
            }
            var file = fileInput.files[0];
            var formData = new FormData();
            formData.append("fileInput", file);

            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/doupdate", true); // true for asynchronous request

            // Send the FormData
            xmlHttp.send(formData);
            showAlert('manual update', 'update was started', 'alert-success');
            startManualUpdate();
        }

        function startManualUpdate() {
            hide('#updateMenu');
            show('#updateProgress');
            $('#remainingTime').hide();
            $('#updateTimeout').hide();
            $('#updateStateNow').html("installing new firmware");
            $('#newVersionProgress').html("manual update");

            var timeoutStart = 50.0;
            var timeout = timeoutStart;
            var progress = 0;
            var updateRunning = 1;

            $('#updateProgressPercent').html("0 %");
            $('#updateTimeout').html("0 s");

            $('#btnUpdateStart').animate({ opacity: 0.3 });

            let timerTO = window.setInterval(function () {
                $.ajax({
                    url: '/updateState',
                    type: 'GET',
                    contentType: false,
                    processData: false,
                    timeout: 2000,
                    success: function (response) {
                        console.log("check OTA progress - " + response.updateProgress + " - run: " + response.updateRunning);
                        progress = response.updateProgress;
                        updateRunning = response.updateRunning;
                        $('#progressbar').width(progress + "%");
                        $('#updateProgressPercent').html(Math.round(progress) + " %");
                        if (progress > 0 && updateRunning == 0) {
                            clearInterval(timerTO);
                            showAlert('manual update', 'DONE', 'alert-success');
                            location.reload();
                        }
                    },
                    error: function () {
                        showAlert('manual update', 'got no response for progress', 'alert-danger');
                    }
                });

                console.log("check OTA progress TO - " + timeout);
                timeout = timeout - 0.25;
                if (timeout < 0 || updateRunning == 0) {
                    clearInterval(timerTO);
                    showAlert('manual update', 'DONE', 'alert-success');
                    location.reload();
                }
            }, 500);

            $('#btnUpdateStart').animate({ opacity: 1 });

            return;
        }

        function initValueChanges() {
            $(".valueText").map(function () {
                observer = new MutationObserver(function (mutationsList, observer) {
                    //console.log(mutationsList);
                    const elem = mutationsList[0].target;
                    // elem.classList.add("animateValue");
                    elem.style.color = "#eee";
                    setTimeout(function () {
                        elem.style.color = "#2196f3";
                    }, 4000);

                });
                observer.observe(this, { characterData: false, childList: true, attributes: false });
            }).get();
        }

        function getTime(unix_timestamp, dateTime = "time") {
            var date = new Date(unix_timestamp * 1000);
            var day = ("0" + date.getDate()).substr(-2);
            var mon = ("0" + (date.getMonth() + 1)).substr(-2);
            var year = date.getFullYear();
            var hours = ("0" + date.getHours()).substr(-2);
            var minutes = ("0" + date.getMinutes()).substr(-2);
            var seconds = ("0" + date.getSeconds()).substr(-2);

            if (dateTime == "date") {
                return day + "." + mon + "." + year;
            } else if (dateTime == "dateShort") {
                return day + "." + mon + ".";
            } else if (dateTime == "timeShort") {
                return hours + ':' + minutes;
            } else {
                return hours + ':' + minutes + ':' + seconds;
            }
        }

        function showDtuWarnings() {

            cacheDtuData.warnings.length > 0 ? $('#dtuWarnings').show() : $('#dtuWarnings').hide();
            numOfWarnings = cacheDtuData.warnings.length;
            numOfWarningsActive = 0;
            // clear the list berfore refill
            $('#activeWarnings').empty();
            // sort cacheDtuData.warnings by timestampStart - newest first
            cacheDtuData.warnings.sort((a, b) => (a.timestampStart < b.timestampStart) ? 1 : -1);
            var activeWarning = false;
            for (let index = 0; index < cacheDtuData.warnings.length; index++) {
                let warning = cacheDtuData.warnings[index];
                if (warning.timestampStop == 0) {
                    numOfWarningsActive++;
                    activeWarning = true;
                } else {
                    activeWarning = false;
                }
                let data0Text = "data0";
                let data0Value = warning.data0;
                let data1Text = "data1";
                let data1Value = warning.data1;

                if (warning.message.toLowerCase().includes('undervoltage')) {
                    data0Text = "measured voltage";
                    data0Value = (warning.data0 / 10).toFixed(2) + " V";
                    data1Text = "minimal voltage";
                    data1Value = (warning.data1 / 10).toFixed(2) + " V";
                }
                else if (warning.message.toLowerCase().includes('overvoltage')) {
                    data0Text = "measured voltage";
                    data0Value = (warning.data0 / 10).toFixed(2) + " V";
                    data1Text = "maximum voltage";
                    data1Value = (warning.data1 / 10).toFixed(2) + " V";
                }
                else if (warning.message.toLowerCase().includes('overfrequency')) {
                    data0Text = "measured frequency";
                    data0Value = (warning.data0 / 100).toFixed(2) + " Hz";
                    data1Text = "maximum frequency";
                    data1Value = (warning.data1 / 100).toFixed(2) + " Hz";
                }
                else if (warning.message.toLowerCase().includes('underfrequency')) {
                    data0Text = "measured frequency";
                    data0Value = (warning.data0 / 100).toFixed(2) + " Hz";
                    data1Text = "minimum frequency";
                    data1Value = (warning.data1 / 100).toFixed(2) + " Hz";
                }
                else if (warning.message.toLowerCase().includes('over temperature')) {
                    data0Text = "measured temperature";
                    data0Value = (warning.data0 / 100).toFixed(2) + " °C";
                    data1Text = "maximum temperature";
                    data1Value = (warning.data1 / 100).toFixed(2) + " °C";
                }

                let warningRow = `
                <div class="warningRow" style="display: flex; justify-content: space-between; ${!activeWarning ? `color: grey;` : ''}">
                    <div class="warningColumnTimeNum">
                        <div class="warningTimestamp">${new Date(warning.timestampStart * 1000).toLocaleString('de-DE', { day: '2-digit', month: '2-digit', year: '2-digit', hour: '2-digit', minute: '2-digit', second: '2-digit' }).replace(',', ' -')}</div>
                        ${warning.timestampStop !== 0 ? `<div class="warningTimestamp">${new Date(warning.timestampStop * 1000).toLocaleString('de-DE', { day: '2-digit', month: '2-digit', year: '2-digit', hour: '2-digit', minute: '2-digit', second: '2-digit' }).replace(',', ' -')}</div>` : ''}
                    </div>
                    <div class="warningColumnNum">
                        <div class="warningMessage">${warning.num}</div>
                    </div>
                    <div class="warningColumnText">
                        <div class="warningMessage">${warning.message}</div>
                    </div>
                    <div class="warningColumnDetail">
                    
                `;
                if (warning.data0 !== 0) {
                    warningRow = warningRow + `<div class="warningData">`;
                    warningRow = warningRow + `<div class="warningDataText">${data0Text}: </div>`;
                    warningRow = warningRow + `<div class="warningDataValue">${data0Value}</div>`;
                    warningRow = warningRow + `</div>`;
                    warningRow = warningRow + `<div class="warningData">`;
                    warningRow = warningRow + `<div class="warningDataText">${data1Text}: </div>`;
                    warningRow = warningRow + `<div class="warningDataValue">${data1Value}</div>`;
                    warningRow = warningRow + `</div>`;
                }
                warningRow = warningRow + `</div>
                </div>
                <hr style="border-top-color: lightgrey;">`;


                $('#activeWarnings').append(warningRow);
            }
            if (numOfWarningsActive != 0) {
                $('#dtuWarningsBadge').html(numOfWarningsActive);
                $('#dtuWarningsBadge').css('background-color', 'orange');
                $('#dtuWarningsBadge').css('color', 'black');
            } else {
                $('#dtuWarningsBadge').html(numOfWarnings);
                $('#dtuWarningsBadge').css('background-color', 'darkcyan');
                $('#dtuWarningsBadge').css('color', 'black');
            }
            $('#warningsLastUpdate').html("(last updated: " + getTime(cacheDtuData.warningsLastUpdate, "date") + " - " + getTime(cacheDtuData.warningsLastUpdate, "time") + ")");
        }

        // alarmState = alert-success, alert-danger, alert-warning
        function showAlert(text, info, alarmState = "") {
            $('#alertBox').attr('class', "alert " + alarmState);
            $('#alertText').html('<b>' + text + '</b><br><small>' + info + '</small>');
            $('#alertBox').css('display', 'flex');

            setTimeout(function () {
                $('#alertBox').fadeOut();
            }, 5000);
        }


        function requestWifiScan() {
            $.ajax({
                url: '/getWifiNetworks',

                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 2000,
                success: function (data) {
                    console.log("requestWifiScan - success: " + data.wifiNetworks);
                },
                error: function () {
                    console.log("timeout getting data in local network");
                }
            });
        }

        function getDataValues() {
            $.ajax({
                url: 'api/data.json',

                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 2000,
                success: function (data) {
                    cacheData = data;
                    refreshData(data);
                },
                error: function () {
                    console.log("timeout getting data in local network");
                }
            });
        }

        function getInfoValues() {
            $.ajax({
                url: 'api/info.json',

                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 2000,
                success: function (info) {
                    cacheInfoData = info;
                    checkInitToSettings(info);
                    refreshInfo(info);
                    getVersionData(info);
                },
                error: function () {
                    console.log("timeout getting data in local network");
                }
            });
        }

        function getDtuDataValues() {
            $.ajax({
                url: 'api/dtuData.json',

                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 2000,
                success: function (dtuData) {
                    cacheDtuData = dtuData;
                    showDtuWarnings();
                },
                error: function () {
                    console.log("timeout getting dtuData in local network");
                }
            });
        }
)=====";
