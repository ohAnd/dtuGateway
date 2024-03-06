const char INDEX_HTML[] PROGMEM = R"=====(

<html lang="de">

<head>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
    <title id="title">HoymilesGW</title>
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
    <div class="popup" id="changeSettings">
        <div class="popupHeader">
            <div class="popupHeaderTitle">settings
                <!-- <h2>settings</h2> -->
            </div>
            <div class="popupHeaderTabs">
                <div>openhab</div>
                <div>dtu</div>
                <div class="selected">wifi</div>
            </div>
        </div>
        <div class="popupContent" id="wifi" style="display: block;">
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
                <input type="text" id="wifiSSIDsend" value="please choose above or type in" required maxlength="32">
            </div>
            <div>
                wifi password (<i class="passcheck" value="invisible">show</i>):
            </div>
            <div>
                <input type="password" id="wifiPASSsend" value="admin12345" required maxlength="32">
            </div>
            <div style="text-align: center;">
                <b onclick="changeWifiData()" id="btnSaveWifiSettings" class="form-button btn">save</b>
                <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
            </div>
        </div>
        <div class="popupContent" id="openhab">
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
            <div style="text-align: center;">
                <b onclick="changeOpenhabData()" id="btnSaveWifiSettings" class="form-button btn">save</b>
                <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
            </div>
        </div>
        <div class="popupContent" id="dtu">
            <div>
                dtu host IP in your local network:
            </div>
            <div>
                <input type="text" id="dtuHostIp" class="ipv4Input" name="ipv4" placeholder="xxx.xxx.xxx.xxx">
            </div>
            <hr>
            <div>
                dtu local wifi:
            </div>
            <div>
                <input type="text" id="dtuSsid" value="please type in" required maxlength="32">
            </div>
            <div>
                dtu wifi password (<i class="passcheck" value="invisible">show</i>):
            </div>
            <div>
                <input type="password" id="dtuPassword" value="admin12345" required maxlength="32">
            </div>

            <div style="text-align: center;">
                <b onclick="changeDtuData()" id="btnSaveDtuSettings" class="form-button btn">save</b>
                <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
            </div>
        </div>
    </div>
    <div class="popup" id="updateMenu">
        <h2>Update</h2>
        <div>
            <div style="padding-bottom: 10px;">
                <p id="updateState">currently no update available</p>
            </div>
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
                    style="border-radius: 0px 5px 5px 0px;position:relative;top:-1.25em;left:50%;">snapshot</div>
                <i style="font-size:x-small;">switch update channels (stable/ latest snapshot)</i>
            </div>
            <hr>
            <div style="text-align: center;">
                <!-- <input id="btnUpdateStart" class="btn" type="submit" name="doUpdate" value="Update starten"> -->
                <b onclick="" id="btnUpdateStart" class="form-button btn">start update</b>
            </div>
            <hr>
            <div style="text-align: center;">
                <b onclick="hide('#updateMenu')" class="form-button btn">close</b>
            </div>
            <hr>
            <div>
                <small style="text-align:center;"><a href="/update">manual update</a></small>
            </div>
        </div>
    </div>
    <div class="popup" id="updateProgress">
        <h2>Update</h2>
        <hr>
        <div style="padding-bottom: 10px;text-align: center;">
            <p id="updateStateNow">update to version <span id="newVersionProgress">0.0.0</span> in progress
            </p>
            <p>remaining time: <span id="updateTimeout"></span></p>
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
            <b>Hoymiles HMS-800W-2T - Gateway</b>
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
                        <b id="powerLimitSet" class="panelValueSmall valueText ">00 </b>%
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
        const waitTime = 31000;
        let remainingTime = waitTime;

        let timerInfoUpdate = 0;
        let cacheInfoData = {};

        $(document).ready(function () {
            console.log("document loading done");
            initValueChanges();
            // first data refresh
            getDataValues();
            getInfoValues();
            requestVersionData();

            window.setInterval(function () {
                getDataValues();
            }, 5000);

            timerInfoUpdate = window.setInterval(function () {
                getInfoValues();
            }, 7500);

            // check every minute (62,5s) for an available update
            window.setInterval(function () {
                requestVersionData();
            }, 62500);

            timerRemainingProgess = window.setInterval(function () {
                remainingResponse();
            }, 100);
        });

        // switchung in popups between tabs
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

        var show = function (id) {
            console.log("show " + id)
            $(id).show(200);
            if (id == '#changeSettings') {
                getWIFIdata();
                getDTUdata();
                getOHdata();
            }
        }

        var hide = function (id) {
            console.log("hide " + id)
            $(id).hide(200);
        }

        function checkInitToSettings(data) {
            // if not configured then start directly with settings dialogue
            if (data.initMode == 1) {
                show("#changeSettings");
                remainingTime = 0.1; // no countdown on top of the site
                $('#settingsTitle').html("settings - in startup config mode");
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

            checkValueUpdate('#pv0_power', (data.pv0.p).toFixed(1), "W");
            checkValueUpdate('#pv0_voltage', (data.pv0.v).toFixed(1), "V");
            checkValueUpdate('#pv0_current', (data.pv0.c).toFixed(1), "A");
            checkValueUpdate('#pv0_daily_energy', (data.pv0.dE).toFixed(3));
            checkValueUpdate('#pv0_total_energy', (data.pv0.tE).toFixed(3));


            checkValueUpdate('#pv1_power', (data.pv1.p).toFixed(1), "W");
            checkValueUpdate('#pv1_voltage', (data.pv1.v).toFixed(1), " V");
            checkValueUpdate('#pv1_current', (data.pv1.c).toFixed(1), "A");
            checkValueUpdate('#pv1_daily_energy', (data.pv1.dE).toFixed(3));
            checkValueUpdate('#pv1_total_energy', (data.pv1.tE).toFixed(3));

            checkValueUpdate('#grid_power', (data.grid.p).toFixed(1), "W");
            checkValueUpdate('#grid_voltage', (data.grid.v).toFixed(1) + "V");
            checkValueUpdate('#grid_current', (data.grid.c).toFixed(1) + "A");
            checkValueUpdate('#grid_daily_energy', (data.grid.dE).toFixed(3));
            checkValueUpdate('#grid_total_energy', (data.grid.tE).toFixed(3));

            checkValueUpdate('#powerLimitSet', data.inverter.pLimSet);
            checkValueUpdate('#powerLimit', data.inverter.pLim);

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
                default:
                    dtuConnect = "no_info";
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

            return true;
        }

        function getWIFIdata() {
            // 
            $('#btnSaveWifiSettings').css('opacity', '1.0');
            $('#btnSaveWifiSettings').attr('onclick', "changeWifiData();")

            wifiData = cacheInfoData.wifiConnection;
            wifiDataNw = wifiData.foundNetworks;
            // get networkdata
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
            $('#dtuHostIp').val(dtuData.dtuHostIp);
            $('#dtuSsid').val(dtuData.dtuSsid);
            $('#dtuPassword').val(dtuData.dtuPassword);

        }

        function getOHdata() {
            // 
            $('#btnSaveDtuSettings').css('opacity', '1.0');
            $('#btnSaveDtuSettings').attr('onclick', "changeDtuData();")

            ohData = cacheInfoData.openHabConnection;

            // get networkdata
            $('#openhabIP').val(ohData.ohHostIp);
            $('#ohItemPrefix').val(ohData.ohItemPrefix);
        }

        $('.passcheck').click(function () {
            console.log("passcheck stat: " + $(this).attr("value") + " - id: " + $(this).attr("id"))
            if ($(this).attr("value") == 'invisible') {
                $('#wifiPASSsend').attr('type', 'text');
                $('#dtuPassword').attr('type', 'text');
                $('.passcheck').attr('value', 'visibile');
                $('.passcheck').html("hide");
            } else {
                $('#wifiPASSsend').attr('type', 'password');
                $('#dtuPassword').attr('type', 'password');
                $('.passcheck').attr('value', 'invisible');
                $('.passcheck').html("show");
            }
        });

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

            strResult = xmlHttp.responseText;
            console.log("got from server: " + strResult);
            alert("Wifi access data changed\n__________________________________\n\nplease connect to the choosen wifi and access with the new ip inside your network (maybe look inside your wifi router)");

            $('#btnSaveWifiSettings').css('opacity', '0.3');
            $('#btnSaveWifiSettings').attr('onclick', "")

            hide('#changeSettings');
            return;
        }

        function changeDtuData() {
            var dtuHostIpSend = $('#dtuHostIp').val();
            var dtuSsidSend = $('#dtuSsid').val();
            var dtuPasswordSend = $('#dtuPassword').val();
            var data = {};
            data["dtuHostIpSend"] = dtuHostIpSend;
            data["dtuSsidSend"] = dtuSsidSend;
            data["dtuPasswordSend"] = dtuPasswordSend;

            console.log("send to server: dtuHostIp: " + dtuHostIpSend + " - dtuSsid: " + dtuSsidSend + " - pass: " + dtuPasswordSend);

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
            console.log("got from server - strResult.dtuHostIp: " + strResult.dtuHostIp + " - cmp with: " + dtuHostIpSend);
            console.log("got from server - strResult.dtuSsid: " + strResult.dtuSsid + " - cmp with: " + dtuSsidSend);
            console.log("got from server - strResult.dtuPassword: " + strResult.dtuHostIp + " - cmp with: " + dtuPasswordSend);

            if (strResult.dtuHostIp == dtuHostIpSend && strResult.dtuSsid == dtuSsidSend && strResult.dtuPassword == dtuPasswordSend) {
                console.log("check saved data - OK");
                alert("dtu Settings change\n__________________________________\n\nYour settings were successfully changed.\n\nClient connection will be reconnected to the new IP.");
            } else {
                alert("dtu Settings change\n__________________________________\n\nSome error occured! Checking data from gateway are not as excpeted after sending to save.\n\nPlease try again!");
            }

            //$('#btnSaveDtuSettings').css('opacity', '0.3');
            //$('#btnSaveDtuSettings').attr('onclick', "")

            hide('#changeSettings');
            return;
        }

        function changeOpenhabData() {
            var openhabHostIpSend = $('#openhabIP').val();
            var data = {};
            data["openhabHostIpSend"] = openhabHostIpSend;
            
            console.log("send to server: openhabHostIpSend: " + openhabHostIpSend);

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
            xmlHttp.open("POST", "/updateOHSettings", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            strResult = JSON.parse(xmlHttp.responseText);
            console.log("got from server: " + strResult);
            console.log("got from server - strResult.dtuHostIp: " + strResult.openhabHostIp + " - cmp with: " + openhabHostIp);

            if (strResult.dtuHostIp == dtuHostIpSend && strResult.dtuSsid == dtuSsidSend && strResult.dtuPassword == dtuPasswordSend) {
                console.log("check saved data - OK");
                alert("openhab Settings change\n__________________________________\n\nYour settings were successfully changed.\n\nClient connection will be reconnected to the new IP.");
            } else {
                alert("openhab Settings change\n__________________________________\n\nSome error occured! Checking data from gateway are not as excpeted after sending to save.\n\nPlease try again!");
            }

            hide('#changeSettings');
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
                strResult = JSON.parse(xmlHttp.responseText);
                console.log("got from server: " + strResult);
                console.log("got from server - strResult.dtuHostIp: " + strResult.dtuHostIp + " - cmp with: " + dtuHostIpSend);

                if (strResult.dtuHostIp == dtuHostIpSend && strResult.dtuSsid == dtuSsidSend && strResult.dtuPassword == dtuPasswordSend) {
                    console.log("check saved data - OK");
                    alert("dtu Settings change\n__________________________________\n\nYour settings were successfully changed.\n\nClient connection will be reconnected to the new IP.");
                } else {
                    alert("dtu Settings change\n__________________________________\n\nSome error occured! Checking data from gateway are not as excpeted after sending to save.\n\nPlease try again!");
                }
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
            $('#newVersionProgress').html(cacheInfoData.versionServer);

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

        function getDataValues() {
            $.ajax({
                url: 'api/data',
                //url: 'data.json',

                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 1000,
                success: function (data) {
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
                timeout: 1000,
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