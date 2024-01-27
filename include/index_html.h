const char INDEX_HTML[] PROGMEM = R"=====(

<html lang="de">

<head>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
    <title id="title">HoymilesGW</title>
    <meta name="viewport"
        content="user-scalable=no, initial-scale=1, maximum-scale=1, minimum-scale=1, width=device-width">
    <link rel="stylesheet" type="text/css" href="style.css">
    <!-- <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css"> -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
</head>

<body>
    <div style="width: 100%;float:left;">
        <div class="bar" id="updateTime"
            style="width: 100%;">
        </div>
    </div>
    <div class="popup" id="updateMenu">
        <h2>Update</h2>
        <hr>
        <div style="padding-bottom: 10px;">
            <p id="updateState">Aktuell kein Update verf&uuml;gbar</p>
        </div>
        <div>
            <table>
                <tr>
                    <td style="text-align: right;border: 0px; border-bottom: 1px;border-style: solid;">installed version
                    </td>
                    <td><b id="firmwareVersion" style="border: 0px; border-bottom: 1px;border-style: solid;">0.0.0</b>
                    </td>
                    <td style="text-align: left;border: 0px; border-bottom: 1px;border-style: solid;">from <b
                            id="builddateVersion">Jan 01 2024 - 00:00:00</b></td>
                </tr>
                <tr>
                    <td style="text-align: right;">available version</td>
                    <td><b id="firmwareVersionServer">0.0.0</b></td>
                    <td style="text-align: left;">from <b id="builddateVersionServer">Jan 01 2024 - 00:00:00</b></td>
                </tr>
            </table>
        </div>
        <hr>
        <div style="text-align: center;">
            <!-- <input id="btnUpdateStart" class="btn" type="submit" name="doUpdate" value="Update starten"> -->
            <b onclick="" id="btnUpdateStart" class="form-button btn">Update starten</b>
        </div>
        <hr>
        <div style="text-align: center;">
            <b onclick="hide('#updateMenu')" class="form-button btn">Schliessen</b>
        </div>
        <hr>
        <div>
            <small style="text-align:center;"><a href="/update">Manuelles Update</a></small>
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
                        <small class="panelHead">DTU connect</small>
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
                    <i class="fa fa-cloud-download" onclick="show('#updateMenu')" alt="update"></i>
                    <span class="badge" id="updateBadge" style="display: none;"></span>
                </div>
            </div>
        </div>
    </div>

    <script>
        let timerRemainingProgess = 0;
        const waitTime = 31000;
        let remainingTime = waitTime;
        let cacheJSONtemp = {};

        $(document).ready(function () {
            console.log("document loading done");
            initValueChanges();
            // first data refresh
            getBaseDataFromLocal();
            requestVersionData();
            window.setInterval(function () {
                getBaseDataFromLocal();
            }, 5000);
            // check every minute (62,5s) for an available update
            window.setInterval(function () {
                requestVersionData();
            }, 62500);

            timerRemainingProgess = window.setInterval(function () {
                remainingResponse();
            }, 100);
        });

        var show = function (id) {
            console.log("show " + id)
            $(id).show(200);
            if (id == '#changeWifiSettings') {
                getWIFIdata();
            }
        }

        var hide = function (id) {
            console.log("hide " + id)
            $(id).hide(200);
        }

        function remainingResponse() {
            if (remainingTime > 0) {
                var remainingTime_width = (remainingTime / waitTime) * 100;
                $('#updateTime').width(remainingTime_width + "%");
            }
            remainingTime = remainingTime - 100;
            if (remainingTime < 0) {
                remainingTime = -1;
            }
        }

        function refreshData(data) {

            var wifiGWPercent = Math.round(data.wifiGW);
            $('#rssitext_local').html(wifiGWPercent + '%');
            var wifiDTUPercent = Math.round(data.wifiDtu);
            $('#rssitext_dtu').html(wifiDTUPercent + '%');

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
            $('#firmware').html("fw version: " + data.version);

            // temp data
            return true;
        }

        function getBaseDataFromLocal() {
            $.ajax({
                url: '/api/summary',
                //url: 'http://192.168.1.90/api/summary',
                //url: 'test.json',

                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 1000,
                success: function (data) {
                    cacheJSONtemp = data;
                    refreshData(data);
                    getVersionData();
                },
                error: function () {
                    console.log("timeout getting data in local network");
                }
            });
        }

        function getVersionData() {
            $('#firmwareVersion').html(cacheJSONtemp.version);
            $('#builddateVersion').html(cacheJSONtemp.versiondate);
            $('#firmwareVersionServer').html(cacheJSONtemp.versionServer);
            $('#builddateVersionServer').html(cacheJSONtemp.versiondateServer);
            if (cacheJSONtemp.updateAvailable == 1) {
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
            $('#newVersionProgress').html(cacheJSONtemp.versionServer);

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
                if (timeout < 0 || cacheJSONtemp.updateAvailable == 0) {
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
            var mon = ("0" + date.getMonth() + 1).substr(-2);
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

    </script>



</body>

</html>

)=====";