static const char *index_html PROGMEM = R"=====(

<html lang="de">

<head>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
    <title id="title">dtuGateway</title>
    <meta name="viewport"
        content="user-scalable=no, initial-scale=1, maximum-scale=1, minimum-scale=1, width=device-width">
    <link rel="stylesheet" type="text/css" href="style.css">
    <!-- Font Awesome CDN removed: icons embedded as SVG in style.css -->
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
            <div class="popupHeaderTitle" id="popHeadTitle">settings <span id="settings_message" style="border: 1px solid; padding: 2px; font-size: small; color: darksalmon;display:none;"></span>
                <!-- <span style="font-size: small;float:right;"> -->
                <span style="float:right;">
                    <span style="display: inline-block; text-align: center;">
                        <div style="position: relative; display: inline-block;">
                            <i class="fa-solid fa-rotate" style="position: absolute; top: 0; left: 65%; cursor: pointer; color: #2196f3; font-size: 0.45em; z-index: 1;"></i>
                            <i class="fa-solid fa-house-laptop" onclick="hide('#changeSettings');show('#rebootDtuGw')" id="showRebootDtuGwBtn" style="position: relative; font-size: 0.9em; cursor: pointer; color: #888;" title="Restart dtuGateway"></i>
                        </div>
                        <span style="display: block; font-size: 0.3em; line-height: 1; margin-top: 2px; color: #888;">dtuGateway</span>
                    </span>
                    &nbsp;
                    <span style="display: inline-block; text-align: center;">
                        <div style="position: relative; display: inline-block;">
                            <i class="fa-solid fa-rotate" style="position: absolute; top: 0; left: 65%; cursor: pointer; color: #2196f3; font-size: 0.45em;"></i>
                            <i class="fa-solid fa-wifi" onclick="hide('#changeSettings');show('#rebootDtu')" id="showRebootDtuBtn" style="position: relative; cursor: pointer; z-index: 1; color: #888;" title="Restart DTU"></i>
                        </div>
                        <span style="display: block; font-size: 0.3em; line-height: 1; margin-top: 2px; color: #888;">DTU</span>
                    </span>
                    &nbsp;
                    <span style="display: inline-block; text-align: center;">
                        <div style="position: relative; display: inline-block;">
                            <i class="fa-solid fa-rotate" style="position: absolute; top: 0; left: 65%; cursor: pointer; color: #2196f3; font-size: 0.45em;"></i>
                            <i class="fa-solid fa-plug-circle-bolt" onclick="hide('#changeSettings');show('#rebootMi')" id="showRebootMiBtn" style="position: relative; cursor: pointer; z-index: 1; color: #888;" title="Restart micro inverter"></i>
                        </div>
                        <span style="display: block; font-size: 0.3em; line-height: 1; margin-top: 2px; color: #888;">micro Inverter</span>
                    </span>
                    
                    <span style="display: inline-block; text-align: center;">
                        <a href="/config" target="_blank" style="color: #888; text-decoration: none;"><i class="fa-solid fa-gears" title="open advanced settings in a new tab"></i></a>
                        <span style="display: block; font-size: 0.3em; line-height: 1; margin-top: 2px; color: #888;">advanced settings</span>
                    </span>
                </span>
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
                <h3 id="mqttSelect"><input type="checkbox" id="mqttActive"> MQTT connection</h3>
                <div>
                    <small id="mqttSectionComment">publish all data to a specific MQTT broker and subscribing to the
                        requested powersetting<br></small>
                </div>
                <div>
                    <br>IP/port to MQTT broker (e.g. 192.168.178.100:1883):
                </div>
                <div>
                    <input type="text" id="mqttIP" class="ipv4Input" name="ipv4" placeholder="xxx.xxx.xxx.xxx">
                </div>
                <div>
                    <input type="checkbox" id="mqttUseTLS"> <small>TLS connection (e.g.
                        123456789.s1.eu.hivemq.cloud:8883) - works only with ESP32</small>
                </div>
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
                <div id="mqttMainTopicComment">
                    MQTT main topic for this dtu: <br><small>(e.g. dtu_12345678 will appear as 'dtu_12345678/grid/U' in
                        the broker -
                        has to be unique in your setup)</small>
                </div>
                <div>
                    <input type="text" id="mqttMainTopic" maxlength="64">
                </div>
                <div id="mqttHAautoDiscovery">
                    <input type="checkbox" id="mqttHAautoDiscoveryON"> HomeAssistant Auto Discovery <br><small>(On =
                        config is send once after every restart, Off = delete the sensor from HA instantly - using the
                        same main topic as set above)</small><br>
                </div>
            </div>
            <hr>
            <div style="text-align: center;">
                <b onclick="changeBindingsData()" id="btnSaveBindingsSettings" class="form-button btn">save</b>
                <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
            </div>
        </div>
        <div class="popupContent" id="dtu">
            <div id="remoteDisplaySettings">
                <input type="checkbox" id="remoteDisplayActive"> run as a remote display<br>
                <small>option to use this device as an remote display for a different dtu gateway and get current data
                    from a given MQTT broker</small>
            </div>
            <div id="remoteSummaryDisplaySettings">
                <input type="checkbox" id="remoteSummaryDisplayActive"> run as a remote summary display (Solar
                Monitor)<br>
                <small>option to use this device as an remote display based on current data from a given MQTT broker to
                    show PV power and yield of current day</small>
            </div>
            <hr>
            <div id="dtuSettings">
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
                    dtu cloud update pause (no cycle update every 5 min):
                    <input type="checkbox" id="dtuCloudPause">
                </div>
            </div>
            <hr>
            <div style="text-align: center;">
                <b onclick="changeDtuData()" id="btnSaveDtuSettings" class="form-button btn">save</b>
                <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
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
                    <input type="number" id="powerLimitSetNew" min="0" max="100" placeholder="">
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
    <div class="popup2" id="rebootMi" style="display: none;">
        <h2><i class="fa-solid fa-plug-circle-bolt"></i> Restart Micro Inverter</h2>
        <div style="padding-bottom: 10px;">
            <small>restarting the micro inverter <br /><br /><i style="font-size: small;">(hint: this will not rebooting
                    the communication unit 'dtu')</i></small>
        </div>
        <div>
            <div style="text-align: center;">
                <b onclick="rebootMi()" id="btnRebootMi" class="form-button btn" style="opacity: 1;">Reboot Mi</b>
            </div>

            <div style="text-align: center;">
                <b onclick="hide('#rebootMi')" class="form-button btn">close</b>
            </div>
        </div>
    </div>
    <div class="popup2" id="rebootDtu" style="display: none;">
        <h2><i class="fa-solid fa-repeat"></i> Restart DTU</h2>
        <div style="padding-bottom: 10px;">
            <small>restarting the DTU <br /><br /><i style="font-size: small;">(hint: this will not rebooting the micro
                    inverter)</i></small>
        </div>
        <div>
            <div style="text-align: center;">
                <b onclick="rebootDtu()" id="btnRebootDtu" class="form-button btn" style="opacity: 1;">Reboot DTU</b>
            </div>

            <div style="text-align: center;">
                <b onclick="hide('#rebootDtu')" class="form-button btn">close</b>
            </div>
        </div>
    </div>
    <div class="popup2" id="rebootDtuGw" style="display: none;">
        <h2><i class="fa-solid fa-repeat"></i> Restart dtuGateway</h2>
        <div style="padding-bottom: 10px;">
            <small>restarting the dtuGateway <br /><br /><i style="font-size: small;">(hint: this will not rebooting the
                    dtu or the micro inverter)</i></small>
        </div>
        <div>
            <div style="text-align: center;">
                <b onclick="rebootDtuGw()" id="btnRebootDtuGw" class="form-button btn" style="opacity: 1;">Reboot
                    dtuGateway</b>
            </div>

            <div style="text-align: center;">
                <b onclick="hide('#rebootDtuGw')" class="form-button btn">close</b>
            </div>
        </div>
    </div>
    <div class="popup" id="updateMenu">
        <h2>Update</h2>
        <h6 id="chipType">controller architecture type: ...</h6>
        <hr>
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
                    style="border-radius: 0px 5px 5px 0px;position:relative;top:-1.25em;left:50%;color: gray;">snapshot
                </div>
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
    <div class="popup" id="warningOverview" style="flex-direction: column;">
        <div>
            <h2>current dtu warnings</h2>
            <h6>Displays the entries in the DTU sorted by the time they occurred <i id="warningsLastUpdate">(last
                    updated: 01.01.2024 - 00:00:00)</i><br><i id="warningDetailsHint">... rotate the screen to see more
                    details at the warning entry ...</i></h6>
            <hr>
        </div>
        <div id="activeWarnings" style="flex-grow: 1;padding-bottom: 10px;text-align: center; overflow-y: auto;">
        </div>
        <hr><br>
        <div style="text-align: center;">
            <b onclick="hide('#warningOverview')" class="form-button btn">close</b>
        </div>
    </div>
    <div class="popup" id="summaryDisplay"
        style="flex-direction: column; width: 95%; left: 2.5%; height: 85%; top: 2.5%; background-color: #1c1c1c; border-radius: 10px; border-width: 1px; border-color: #2196f3; border-style: solid; box-shadow: 0px 0px 19px 11px rgb(255 165 0); z-index: 19;">
        <div>
            <h2>remote summary display</h2>
            <h6>this dtuGateway is configured as a remote summary display</h6>
            <i style="font-size: x-small;float:right;">change this in <a href="/config" target=_blank>advanced
                    config</a></i>
            <br>
            <hr>
        </div>
        <div>
            solar monitor
        </div>
        <div class="panelValueBox">
            <p>current PV power</p><br>
            <b id="grid_power2" class="panelValue valueText">--.- W</b><br><br>
            <small class="panelHead">solar yield today</small><br><br>
            <b id="grid_daily_energy2" class="panelValueSmall valueText">00.0 </b>kWh<br>
        </div>
    </div>
    </div>
    <div id="frame">
        <div class="header">
            <b id="titleHeader">Hoymiles HMS-800W-2T - Gateway</b>
            <div id="dtuWarnings" style="display: none;">
                <i class="fa fa-exclamation-triangle" style="color: darkcyan;" onclick="show('#warningOverview')"></i>
                <span class="numBadge" id="dtuWarningsBadge">20</span>
            </div>
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
                    <i id="infoInveterOff" class="fa fa-power-off" style="color: orange;display:none;"></i>
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
                    <i class="fa fa-signal" alt="wifi DTU"></i>
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
                    <i class="fa-solid fa-cloud-download" onclick="show('#updateMenu')" alt="update" id="updateBtn"></i>
                    <span class="badge" id="updateBadge" style="display: none;"></span>
                </div>
                <div class="menuButton notification">
                    <i class="fa-solid fa-sliders" onclick="show('#changeSettings')" alt="settings"
                        id="settingsBtn"></i>
                    <!-- <span class="badge">0</span> -->
                </div>
            </div>
        </div>
    </div>

    <script src="index.js"></script>

</body>

</html>

)=====";