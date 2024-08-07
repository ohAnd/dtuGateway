const char INDEX_HTML[] PROGMEM = R"=====(

<html lang="de">

<head>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
    <title id="title">dtuGateway</title>
    <meta name="viewport"
        content="user-scalable=no, initial-scale=1, maximum-scale=1, minimum-scale=1, width=device-width">
    <link rel="stylesheet" type="text/css" href="style.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
    <script src="jquery.min.js"></script>
</head>

<body>
    <div style="width: 100%;float:left;">
        <div class="bar" id="updateTime" style="width: 100%;">
        </div>
    </div>
    <div class="alert" id="alertBox">
        <button onclick="this.parentElement.style.display='none';" style="margin-right: 10px;">X</button>
        <div id="alertText">...</div>
    </div>
    <div class="popup" id="changeSettings">
        <div class="popupHeader">
            <div class="popupHeaderTitle">settings <i style="font-size: x-small;float:right;"><a href="/config"
                        target=_blank>advanced config</a></i>
                <!-- <h2>settings</h2> -->
            </div>
            <div class="popupHeaderTabs">
                <div>bindings</div>
                <div>dtu</div>
                <div class="selected">wifi</div>
            </div>
        </div>
        <div class="popupContent" id="wifi" style="display: block;">
            <div id="wifiSearchBox"
                style="display: none; position: relative;top: 45%;z-index: 1;text-align: center;background-color: lightgray;">
                <h2 id="wifiSearch">searching for wifi networks ... </h2>
            </div>
            <div id="wifiContent">
                <div style="padding-bottom: 10px;">
                    <p>available wifi's (<b id="networkCount">0</b>) - currently connected: <b id="wifiSSID"></b>
                    </p>
                    <div id="networks">
                    </div>
                </div>
                <div>
                    connect to wifi:
                </div>
                <div>
                    <input type="text" id="wifiSSIDsend" value="please choose above or type in" required maxlength="64">
                </div>
                <div>
                    wifi password (<i class="passcheck" value="invisible">show</i>):
                </div>
                <div>
                    <input type="password" id="wifiPASSsend" value="admin12345" required maxlength="64">
                </div>
                <div style="text-align: center;">
                    <b onclick="changeWifiData()" id="btnSaveWifiSettings" class="form-button btn">save</b>
                    <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
                </div>
            </div>
        </div>
        <div class="popupContent" id="bindings">
            <div id="openhabSection">
                <h3 id="test"><input type="checkbox" id="openhabActive"> openhab</h3>
                <div>
                    <p>define your openhab instance</p>
                </div>
                <div>
                    IP to openhab:
                </div>
                <div>
                    <input type="text" id="openhabIP" class="ipv4Input" name="ipv4" placeholder="xxx.xxx.xxx.xxx">
                </div>
                <div>
                    openHab item prefix for U,I,P,dE,TE per channel:
                </div>
                <div>
                    <input type="text" id="ohItemPrefix" maxlength="32">
                </div>
            </div>
            <hr>
            <div id="mqttSection">
                <h3><input type="checkbox" id="mqttActive"> MQTT connection</h3>
                <div>
                    <p>publish all data to a specific MQTT broker and subscribing to the requested powersetting</p>
                </div>
                <div>
                    IP/port to MQTT broker (e.g. 192.168.178.100:1883):
                </div>
                <div>
                    <input type="text" id="mqttIP" class="ipv4Input" name="ipv4" placeholder="xxx.xxx.xxx.xxx">
                </div>
                <!-- <div> -->
                <!-- <input type="checkbox" id="mqttUseTLS"> TLS connection (e.g. 123456789.s1.eu.hivemq.cloud:8883) -->
                <!-- </div> -->
                <div>
                    <br>specify user on your mqtt broker instance:
                </div>
                <div>
                    <input type="text" id="mqttUser" value="please type in" required maxlength="64">
                </div>
                <div>
                    password for the given mqtt user (<i class="passcheck" value="invisible">show</i>):
                </div>
                <div>
                    <input type="password" id="mqttPassword" value="admin12345" required maxlength="64">
                </div>
                <div>
                    MQTT main topic for this dtu (e.g. dtu_12345678 will appear as 'dtu_12345678/grid/U' in the broker -
                    has to be unique in your setup):
                </div>
                <div>
                    <input type="text" id="mqttMainTopic" maxlength="32">
                </div>
                <div>
                    <input type="checkbox" id="mqttHAautoDiscoveryON"> HomeAssistant Auto Discovery <br><small>(On =
                        config is send once after every restart, Off = delete the sensor from HA instantly - using the
                        same main topic as set above)</small><br>
                </div>
            </div>
            <hr>
            <div style="text-align: center;">
                <b onclick="changeBindingsData()" id="btnSaveWifiSettings" class="form-button btn">save</b>
                <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
            </div>
        </div>
        <div class="popupContent" id="dtu">
            <div>
                dtu host IP in your local network:
            </div>
            <div>
                <input type="text" id="dtuHostIpDomain" class="ipv4Input" name="ipv4" placeholder="xxx.xxx.xxx.xxx">
            </div>
            <hr>
            <div>
                dtu request cycle in seconds (data update):
            </div>
            <div>
                <input type="number" id="dtuDataCycle" min="1" max="60" placeholder="31">
            </div>
            <div>
                dtu cloud update pause (no cycle update every full 15 min):
                <input type="checkbox" id="dtuCloudPause">
            </div>
            <hr>
            <div style="color: gray;">
                <div><small><i>currently not supported/ needed</i></small><br>
                    dtu local wireless access point:
                </div>
                <div>
                    <input type="text" id="dtuSsid" value="please type in" required maxlength="64" disabled>
                </div>
                <div>
                    dtu wifi password (<i class="passcheck" value="invisible">show</i>):
                </div>
                <div>
                    <input type="password" id="dtuPassword" value="admin12345" required maxlength="64" disabled>
                </div>
            </div>

            <div style="text-align: center;">
                <b onclick="changeDtuData()" id="btnSaveDtuSettings" class="form-button btn">save</b>
                <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
            </div>
        </div>
    </div>
    </div>
    <div class="popup" id="updatePowerLimit" style="display: none;">
        <h2>Update power limit</h2>
        <div>
            <div id="PowerLimitInfo">
                <div> power limit now in %
                    <p id="powerLimitNow"></p>
                </div>
                <hr>
                <div> power limit set in %
                    <input type="number" id="powerLimitSetNew" min="2" max="100" placeholder="">
                </div>
            </div>

            <hr>

            <div style="text-align: center;">
                <b onclick="changePowerLimit()" id="btnSetPowerLimit" class="form-button btn" style="opacity: 1;">set
                    power limit</b>
            </div>

            <div style="text-align: center;">
                <b onclick="hide('#updatePowerLimit')" class="form-button btn">close</b>
            </div>
        </div>
    </div>
    <div class="popup" id="updateMenu">
        <h2>Update</h2>
        <div style="padding-bottom: 10px;">

            <div style="padding-bottom: 10px;"></div>

            <label class="switch">
                <input type="checkbox" checked onChange="changeUpdateType(this.checked)">
                <span class="slider"></span>
            </label>
            <label id="updateSwitch">manual/ auto</label>
        </div>
        <label id="updateType" style="color: gray;">direct online update - </label>
        <i id="updateState" style="color: gray;">currently no update available</i>
        <div id="autoUpdate" style="color: gray;">
            <div id="updateInfo">
                <div>
                    <div class="tableCell" style="text-align:right;">
                        <div id="firmwareVersion"></div>
                        <i>installed version</i>
                    </div>
                    <div class="tableCell">
                        <div id="builddateVersion"></div>
                        <i>release date</i>
                    </div>
                </div>
                <div>
                    <div class="tableCell" style="text-align:right;">
                        <div id="firmwareVersionServer"></div>
                        <i>available version</i>
                    </div>
                    <div class="tableCell">
                        <div id="builddateVersionServer"></div>
                        <i>release date</i>
                    </div>
                </div>
            </div>
            <hr>
            <div style="display: grid;align-items: center;justify-content: center;width:100%;">
                <div onclick="changeReleaseChannel(0)" id="relChanStable" class="updateChannel selected"
                    style="border-radius: 5px 0px 0px 5px;">stable</div>
                <div onclick="changeReleaseChannel(1)" id="relChanSnapshot" class="updateChannel"
                    style="border-radius: 0px 5px 5px 0px;position:relative;top:-1.25em;left:50%;color: gray;">snapshot</div>
                <i style="font-size:x-small;">switch update channels (stable/ latest snapshot)</i>
            </div>
            <hr>
            <div style="text-align: center;">
                <!-- <input id="btnUpdateStart" class="btn" type="submit" name="doUpdate" value="Update starten"> -->
                <b onclick="" id="btnUpdateStart" class="form-button btn">start update</b>
            </div>
        </div>
        <div id="updateManual" style="text-align: center; padding-top: 20px; display:none;">
            <!-- <input type='file' name='update'> -->
            <input type="file" id="fileInput" style="display: none;" accept=".bin" onchange="showFileName(this);">
            <label for="fileInput" class="form-button btn">choose file</label>
            <div id="fileNameDisplay" style="padding:20px 0 20px 0;"></div>
            <b id="manualUpdateStart" onClick="updateManualWithFile()" class="form-button btn"
                style="display: none;">update firmware</b>
        </div>
        <hr>
        <div style="text-align: center;">
            <b onclick="hide('#updateMenu')" class="form-button btn">close</b>
        </div>
    </div>
    </div>
    <div class="popup" id="updateProgress">
        <h2>Update</h2>
        <hr>
        <div style="padding-bottom: 10px;text-align: center;">
            <p id="updateStateNow">update to version <span id="newVersionProgress">0.0.0</span> in progress
            </p>
            <p id="remainingTime">remaining time: <span id="updateTimeout"></span></p>
        </div>
        <div style="border-color: #3498db; border-style: solid;border-radius: 5px;border-width: 1px;">
            <div id="progressbar" class="ui-progressbar-value" style="width:0%;">&nbsp;</div>
        </div>
        <b>
            <p id="updateProgressPercent" style="text-align: center;"></p>
        </b>
        <hr>
    </div>
    <div id="frame">
        <div class="header">
            <b>Hoymiles HMS-xxxW-2T - Gateway</b>
        </div>
        <div class="row">
            <div class="column">
                <div>
                    PV 0
                </div>
                <div class="panelValueBox">
                    <b id="pv0_power" class="panelValue valueText">--.- W</b>
                    <div class="panelValueBoxDetail">
                        <small>U</small>
                        <b id="pv0_voltage" class="panelValueSmall valueText">00.0 V</b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small>I</small>
                        <b id="pv0_current" class="panelValueSmall valueText">00.0 A</b>
                    </div>
                </div>
            </div>

            <div class="column">
                <div>
                    PV 1
                </div>
                <div class="panelValueBox">
                    <b id="pv1_power" class="panelValue valueText">--.- W</b>
                    <div class="panelValueBoxDetail">
                        <small>U</small>
                        <b id="pv1_voltage" class="panelValueSmall valueText">00.0 V</b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small>I</small>
                        <b id="pv1_current" class="panelValueSmall valueText">00.0 A</b>
                    </div>
                </div>
            </div>

            <div class="column">
                <div>
                    Grid
                </div>
                <div class="panelValueBox">
                    <b id="grid_power" class="panelValue valueText">--.- W</b>
                    <div class="panelValueBoxDetail">
                        <small>U</small>
                        <b id="grid_voltage" class="panelValueSmall valueText">00.0 V</b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small>I</small>
                        <b id="grid_current" class="panelValueSmall valueText">00.0 A</b>
                    </div>
                </div>
            </div>
            <div class="column" id="time">
                <div>
                    gateway local time
                </div>
                <div class="panelValueBox">
                    <b id="gwtime" class="panelValue">00:00:00</b><br>
                    <b id="gwtime2" class="panelValueSmall">00.00.</b>
                </div>
            </div>
            <div class="column" id="inverter_energy">
                <div>
                    inverter energy
                </div>
                <div class="panelValueBox">

                    <div class="panelValueBoxDetail">
                        <small class="panelHead">grid day</small>
                        <b id="grid_daily_energy" class="panelValueSmall valueText">00.0 </b>kWh
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">tot</small>
                        <b id="grid_total_energy" class="panelValueSmall valueText">00.0 </b>kWh
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">pv0 day</small>
                        <b id="pv0_daily_energy" class="panelValueSmall valueText">00.0 </b>kWh
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">tot</small>
                        <b id="pv0_total_energy" class="panelValueSmall valueText">00.0 </b>kWh
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">pv1 day</small>
                        <b id="pv1_daily_energy" class="panelValueSmall valueText">00.0 </b>kWh
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">tot</small>
                        <b id="pv1_total_energy" class="panelValueSmall valueText">00.0 </b>kWh
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">limit set</small>
                        <b id="powerLimitSet" class="panelValueButton valueText " onclick="show('#updatePowerLimit')">00
                        </b>%
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">limit now</small>
                        <b id="powerLimit" class="panelValueSmall valueText">00 </b>%
                    </div>
                </div>
            </div>
            <div class="column" id="connection_state">
                <div>
                    connection state
                </div>
                <div class="panelValueBox">
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">last DTU rx</small>
                        <b id="last_response" class="panelValueSmall" style="color: rgb(0, 153, 255);">00:00:00</b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">gw local</small>
                        <b id="gwtime_small" class="panelValueSmall" style="color: rgb(0, 153, 255);">00:00:00</b>
                    </div>

                    <div class="panelValueBoxDetail">
                        <small class="panelHead">gw start</small>
                        <b id="gwStartTime" class="panelValueSmall">00:00:00</b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">gw ntp</small>
                        <b id="gwNTPtime" class="panelValueSmall">00:00:00</b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">DTU</small>
                        <b id="dtu_connect_state" class="panelValueSmall valueText"> offline </b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">DTU state</small>
                        <b id="dtu_error_state" class="panelValueSmall valueText"> ok </b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">DTU reboots</small>
                        <b id="dtu_reboots_no" class="panelValueSmall valueText"> 0 </b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">temperature</small>
                        <b id="inverterTemp" class="panelValueSmall valueText">--.- &deg;C</b>
                    </div>
                </div>
            </div>
        </div>
        <div class="footer">
            <div id="footer_left">
                <div class="footerButton">
                    <i class="fa fa-hourglass-start" alt="uptime"></i>
                    <b id="uptime" style="text-align: right;top: 20px; font-size: 2vmin;">00:00:00</b>
                </div>
                <div class="footerButton">
                    <i class="fa fa-house-signal" alt="wifi DTU"></i>
                    <span id="rssitext_dtu" style="text-align: right;top: 20px; font-size: 2vmin;">50 %</span>
                </div>
                <div class="footerButton">
                    <i class="fa fa-wifi" alt="wifi local"></i>
                    <span id="rssitext_local" style="text-align: right;top: 20px; font-size: 2vmin;">50 %</span>
                </div>
            </div>
            <div id="footer_center">
                <br>
                <i id="firmware">version: 0.0.00</i>
                <br>
                <!-- <i id="builddate">Jan 01 2023 - 00:00:00</i> -->
            </div>
            <div id="footer_right">
                <div class="menuButton notification">
                    <i class="fa fa-cloud-download" onclick="show('#updateMenu')" alt="update" id="updateBtn"></i>
                    <span class="badge" id="updateBadge" style="display: none;"></span>
                </div>
                <div class="menuButton notification">
                    <i class="fa fa-sliders" onclick="show('#changeSettings')" alt="settings" id="settingsBtn"></i>
                    <!-- <span class="badge">0</span> -->
                </div>
            </div>
        </div>
    </div>

    <script>
        let timerRemainingProgess = 0;
        let waitTime = 31000;
        let remainingTime = waitTime;

        let timerInfoUpdate = 0;
        let cacheInfoData = {};
        let cacheData = {};

        $(document).ready(function () {
            console.log("document loading done");
            initValueChanges();
            // first data refresh
            getDataValues();
            getInfoValues();
            requestVersionData();

            window.setInterval(function () {
                getDataValues();
            }, 1000);

            timerInfoUpdate = window.setInterval(function () {
                getInfoValues();
            }, 5000);

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
            if ($(this).closest('div').get(0).id != '') {
                if (this.checked) {
                    $(this).closest('div').css('color', '');
                } else {
                    $(this).closest('div').css('color', 'grey');
                }
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
        }

        var hide = function (id) {
            console.log("hide " + id)
            $(id).hide(200);
        }

        function checkInitToSettings(data) {
            // if not configured then start directly with settings dialogue
            var startUptext = "settings --- startup config mode";
            if (data.initMode == 1 && $('.popupHeaderTitle').html() != startUptext) {
                show('#changeSettings');
                remainingTime = 0.1; // no countdown on top of the site
                $('.popupHeaderTitle').html(startUptext);
                // disable close button
                $('#btnSettingsClose').css('opacity', '0.3');
                $('#btnSettingsClose').attr('onclick', "")
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


            checkValueUpdate('#pv1_power', ((isNaN(data.pv0.p)) ? "--.-" : (data.pv1.p).toFixed(1)), "W");
            checkValueUpdate('#pv1_voltage', (data.pv1.v).toFixed(1), " V");
            checkValueUpdate('#pv1_current', (data.pv1.c).toFixed(1), "A");
            checkValueUpdate('#pv1_daily_energy', (data.pv1.dE).toFixed(3));
            checkValueUpdate('#pv1_total_energy', (data.pv1.tE).toFixed(3));

            checkValueUpdate('#grid_power', ((isNaN(data.pv0.p)) ? "--.-" : (data.grid.p).toFixed(1)), "W");
            checkValueUpdate('#grid_voltage', (data.grid.v).toFixed(1) + "V");
            checkValueUpdate('#grid_current', (data.grid.c).toFixed(1) + "A");
            checkValueUpdate('#grid_daily_energy', (data.grid.dE).toFixed(3));
            checkValueUpdate('#grid_total_energy', (data.grid.tE).toFixed(3));

            checkValueUpdate('#powerLimitSet', ((isNaN(data.inverter.pLimSet)) ? "--.-" : data.inverter.pLimSet));
            checkValueUpdate('#powerLimit', data.inverter.pLim);
            checkValueUpdate('#powerLimitNow', data.inverter.pLim);

            checkValueUpdate('#inverterTemp', (data.inverter.temp).toFixed(1), "'C");

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
            return true;
        }

        function refreshInfo(data) {

            var wifiGWPercent = Math.round(data.wifiConnection.rssiGW);
            $('#rssitext_local').html(wifiGWPercent + '%');
            var wifiDTUPercent = Math.round(data.dtuConnection.dtuRssi);
            $('#rssitext_dtu').html(wifiDTUPercent + '%');

            $('#firmware').html("fw version: " + data.firmware.version);

            if (data.firmware.selectedUpdateChannel == 0) { $("#relChanStable").addClass("selected"); $("#relChanSnapshot").removeClass("selected"); }
            else { $("#relChanStable").removeClass("selected"); $("#relChanSnapshot").addClass("selected"); }

            // setting timer value according to user setting
            waitTime = data.dtuConnection.dtuDataCycle * 1000;

            checkValueUpdate('#dtu_reboots_no', data.dtuConnection.dtuResetRequested);

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
            if ($("#dtuCloudPause").is(':checked')) {
                dtuCloudPauseSend = 1;
            } else {
                dtuCloudPauseSend = 0;
            }

            var dtuSsidSend = $('#dtuSsid').val();
            var dtuPasswordSend = $('#dtuPassword').val();

            var data = {};
            data["dtuHostIpDomainSend"] = dtuHostIpDomainSend;
            data["dtuDataCycleSend"] = dtuDataCycleSend;
            data["dtuCloudPauseSend"] = dtuCloudPauseSend;
            data["dtuSsidSend"] = dtuSsidSend;
            data["dtuPasswordSend"] = dtuPasswordSend;

            console.log("send to server: dtuHostIpDomain: " + dtuHostIpDomainSend + " dtuDataCycle: " + dtuDataCycleSend + " dtuCloudPause: " + dtuCloudPauseSend + " - dtuSsid: " + dtuSsidSend + " - pass: " + dtuPasswordSend);

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
            console.log("got from server - strResult.dtuSsid: " + strResult.dtuSsid + " - cmp with: " + dtuSsidSend);
            console.log("got from server - strResult.dtuPassword: " + strResult.dtuHostIpDomain + " - cmp with: " + dtuPasswordSend);

            if (strResult.dtuHostIpDomain == dtuHostIpDomainSend && strResult.dtuSsid == dtuSsidSend && strResult.dtuPassword == dtuPasswordSend) {
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

            // if (strResult.openhabHostIp == openhabHostIpSend && strResult.mqttBrokerIp == mqttIpSend && strResult.mqttBrokerUser == mqttUserSend) {
            //   console.log("check saved data - OK");
            //   alert("bindings Settings change\n__________________________________\n\nYour settings were successfully changed.\n\nChanges will be applied.");
            // } else {
            //    alert("bindings Settings change\n__________________________________\n\nSome error occured! Checking data from gateway are not as excpeted after sending to save.\n\nPlease try again!");
            // }

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
                //console.log("got from server - strResult.dtuHostIpDomain: " + strResult.dtuHostIpDomain + " - cmp with: " + dtuHostIpDomainSend);

                //if (strResult.dtuHostIpDomain == dtuHostIpDomainSend && strResult.dtuSsid == dtuSsidSend && strResult.dtuPassword == dtuPasswordSend) {
                //    console.log("check saved data - OK");
                //    showAlert('change release channel', 'Your settings were successfully changed.','alert-success');
                //} else {
                //    showAlert('Some error occured!', 'change release channel could not be saved. Please try again!','alert-danger');
                //}
            } catch (error) {
                console.log("error at request change release channel: " + error);
            }

            //$('#btnSaveDtuSettings').css('opacity', '0.3');
            //$('#btnSaveDtuSettings').attr('onclick', "")

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

            //xmlHttp.onload = function () {
            //    if (xmlHttp.status === 200) {
            //        var strResult = JSON.parse(xmlHttp.responseText);
            //        console.log("got from server: ", strResult);
            //        showAlert('manual update', 'update was started', 'alert-success');
            //    } else {
            //        console.log("Error", xmlHttp.statusText);
            //    }
            //};

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
                    //url: 'api/data',
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
            }, 250);

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
                    //elem.style.fontSize = "9vmin";
                    //console.log("event change in value for " + elem.id + " new innerHtml: " + elem.innerHTML);
                    setTimeout(function () {
                        elem.style.color = "#2196f3";
                        //elem.style.fontSize = "6.5vmin";
                        // console.log("timeout --- event change in value for " + elem.id + " new innerHtml: " + elem.innerHTML);
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

        // alarmState = alert-success, alert-danger, alert-warning
        function showAlert(text, info, alarmState = "") {
            $('#alertBox').attr('class', "alert " + alarmState);
            $('#alertText').html('<b>' + text + '</b><br><small>' + info + '</small>');
            $('#alertBox').css('display', 'flex');

            setTimeout(function () {
                //$('#alertBox').css('display', 'none');
                $('#alertBox').fadeOut();
            }, 5000);
        }

        // alternative if fontAwsome is not reachable
        document.addEventListener('DOMContentLoaded', function () {
            // Check if Font Awesome styles are applied to an element
            var iconElement = document.createElement('i');
            iconElement.className = 'fa';
            document.body.appendChild(iconElement);
            // Check if the 'fa' class is applied, indicating successful loading
            if (window.getComputedStyle(iconElement).fontFamily !== 'FontAwesome') {
                handleFontAwesomeError();
            }
            // Remove the temporary element
            document.body.removeChild(iconElement);
        });

        function handleFontAwesomeError() {
            var iconElement = document.getElementById('settingsBtn');
            if (iconElement) iconElement.innerHTML = '<span style="font-size: 4vmin;">Set</span>';
            var iconElement = document.getElementById('updateBtn');
            if (iconElement) iconElement.innerHTML = '<span style="font-size: 4vmin;">Upd</span>';
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
                url: 'api/data',
                //url: 'data.json',

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
                url: 'api/info',
                //url: 'info.json',

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

    </script>

</body>

</html>

)=====";