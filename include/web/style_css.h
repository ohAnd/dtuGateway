static const char *style_css PROGMEM = R"=====(


* {
    box-sizing: border-box;
}

body {
    background: #000b5e;
    font-family: sans-serif;
    font-size: 2vmin;
    color: #2196f3;
}

hr {
    /* display: block;
    height: 1px; */
    /* border: 0; */
    border-top: 1px solid #2196f3;
    /* margin: 1em 0;
    padding: 0; */
}

/* reload bar at the top */
.bar {
    display: flex;
    align-items: center;
    justify-content: center;
    transition: width 0.25s;
    height: 0.2%;
    background-color: #2196f3;
    border-radius: 2px;
}


/* Style the header */
.header {
    height: 12%;
    margin-bottom: 1%;
    /* position: relative; */

    display: flex;
    justify-content: center;
    /* justify-content: space-between; */
    align-items: center;
    text-align: center;
    background-color: #1c1c1c;
    font-size: 5vmin;

    border-radius: 10px;
    border-width: 1px;
    border-color: #2196f3;
    border-style: solid;
}

#titleHeader {
    font-size: 4.5vmin;
    padding-left: 1%;
    padding-right: 1%;
    width: 100%;
}

.row {
    height: 73%;

    /* border-color: #f3213d;
    border-radius: 5px;
    border-width: 1px;
    border-style: solid; */
    padding-right: 0.3%;
}

/* Clear floats after the columns */
.row:after {
    content: "";
    display: table;
    clear: both;
}

/* Create three equal columns that floats next to each other */
.column {
    float: left;
    width: 33.033%;
    height: 49.6%;
    padding: 1%;

    margin-bottom: 0.3%;
    margin-left: 0.3%;

    border-radius: 10px;
    border-width: 1px;
    border-color: #2196f3;
    border-style: solid;

    font-size: 3vmin;
}

.panelValueBox {
    padding-top: 4vmin;
}

.panelValue {
    float: left;
    font-size: 7.5vmin;
    width: 100%;
    padding-bottom: 5%;
}

.panelValueSmall {
    font-size: 3vmin;
}

.panelValueButton {
    font-size: 3vmin;
    border-width: 1px;
    border-color: #2196f3;
    border-style: solid;
    border-radius: 5px;
    padding-left: 2%;
    margin-right: 2%;
}

.panelValueButton:hover {
    border-color: white;
    border-width: 2px;
    cursor: pointer;
}

.panelValueBoxDetail {
    float: left;
    width: 50%;
    color: rgb(0, 89, 255);
    font-size: 2.5vmin;
    /* border-width: 1px;
    border-color: #21f333;
    border-style: solid; */
}

.panelHead {
    padding-bottom: 2px;
    font-size: x-small;
}

/* animated change of color if value chnaged */
.valueText {
    transition: color 0.75s;
}


/* Style the footer */
.footer {
    height: 12%;
    margin-top: 1%;

    display: flex;
    justify-content: center;
    align-items: center;

    font-size: 2em;

    background-color: #1c1c1c;
    text-align: center;
    border-radius: 10px;

    border-width: 1px;
    border-color: #2196f3;
    border-style: solid;
}

.footerButton {
    float: left;
    padding-top: 1.5vh;
    padding-left: 20px;
    font-size: 4v
}

.menuButton {
    /* padding-top: 10px; */
    padding-top: 1.5vh;
    padding-right: 20px;
    float: right;
    cursor: pointer;
    font-size: 5vmin;
}

.notification {
    /* background-color: #555; */
    /* color: white; */
    text-decoration: none;
    /* padding: 15px 26px; */
    position: relative;
    display: inline-block;
    /* border-radius: 2px; */
}

.notification:hover {
    /* background: red; */
    color: white;
}

.numBadge {
    position: relative;
    top: -1.2em;
    right: 0.5em;
    background-color: darkcyan;
    color: white;
    border-radius: 50%;
    padding: 0.9em;
    width: 1.5em;
    height: 1.5em;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 0.45em;
    font-weight: bold;
}

#dtuWarnings {
    display: flex;
    align-items: center;
    margin-left: auto;
    font-size: 0.8em;
    cursor: pointer;
}
#warningDetailsHint {
    display: none;
    color: #5f5d5d;
}

.warningColumnTime {
    flex: 0 0 auto;
    padding: 5px;
}

.warningColumnNum {
    flex: 0 0 5em;
    padding: 5px;
}

.warningColumnText,.warningColumnDetail {
    flex: 1;
    padding: 5px;
    text-align: left;
}

.warningTimestamp {
    text-align: center;
    font-size: smaller;
}

.warningData {
    display: flex;
    flex-direction: row;
}

.warningDataText {
    width: 70%;
    text-align: right;
    font-style: italic;
    font-size: 1vw;
}

.warningDataValue {
    width: 20%;
    text-align: right;
    font-size: smaller;
    font-size: 1vw;
}

.badge {
    font-size: x-small;
}

.notification .badge {

    position: absolute;
    top: 10%;
    right: 15%;
    /* padding: 5px 8px; */
    border-radius: 100%;
    background: red;
    color: white;
    height: 2vh;
    width: 2vh;
}

.alert {
    display: none;
    /* display: flex; */
    position: absolute;
    width: 42%;
    left: 57%;
    height: 5%;
    top: 2%;
    align-items: center;

    padding: .75rem 1.25rem;
    margin-bottom: 1rem;
    border: 1px solid transparent;
    border-radius: .25rem;

    color: #004085;
    background-color: #cce5ff;
    border-color: #b8daff;

    z-index: 21;
    font-size: large;
}

.alert-success {
    color: #155724;
    background-color: #d4edda;
    border-color: #c3e6cb;
}

.alert-danger {
    color: #721c24;
    background-color: #f8d7da;
    border-color: #f5c6cb;
}

.alert-warning {
    color: #856404;
    background-color: #fff3cd;
    border-color: #ffeeba;
}

.popup {
    display: none;
    position: absolute;
    padding: 25px;
    width: 80%;
    left: 10%;
    height: 80%;
    top: 10%;
    background: #FFF;
    border-width: 1px;
    border-color: #2196f3;
    border-style: solid;
    border-radius: 10px;
    z-index: 20;
    font-size: 2.4vmin;
}

.popup2 {
    display: none;
    position: absolute;
    padding: 25px;
    width: 40%;
    left: 30%;
    height: 60%;
    top: 20%;
    /* background: #a5a4a4;
    border-width: 1px;
    border-color: #2196f3;
    border-style: solid;
    border-radius: 10px; */
    z-index: 20;
    font-size: 2.4vmin;
}

.popup2:after {
    position: fixed;
    content: "";
    top: 0;
    left: 0;
    bottom: 0;
    right: 0;
    background: rgba(0, 0, 0, 0.8);
    z-index: -2;
}

.popup2:before {
    position: absolute;
    content: "";
    top: 0;
    left: 0;
    bottom: 0;
    right: 0;
    background: #4e4e4e;
    border-width: 1px;
    border-color: #2196f3;
    border-style: solid;
    border-radius: 10px;
    z-index: -1;
}

/* .popup>.popupHeader>.selected { */
/* .popup>.popupHeader { */
.popupHeader {
    position: absolute;
    top: 5%;
    left: 25px;
    right: 25px;
    bottom: 80%;

    /* border-bottom: 1px solid; */
    /* border: 1px solid #e73701; */
    /* background-color: #2196f3; */
    color: #474444;
    z-index: 21;
}

.popupHeader>.popupHeaderTitle {
    position: relative;
    height: 50%;

    font-weight: bold;
    font-size: 1.5em;

    /* border: 1px solid #790079; */
    /* border-radius: 0px 0px 0px 0px; */
}

.popupHeader>.popupHeaderTabs {
    position: relative;
    height: 50%;

    /* border: 1px solid #e3f300; */
}

.popupHeader>.popupHeaderTabs>div {
    position: relative;
    height: 100%;
    padding: 1%;

    float: right;
    width: 33.33%;

    border: 1px solid #2196f3;
    border-radius: 5px 5px 0px 0px;

    display: grid;
    align-items: center;
    justify-content: center;

    cursor: pointer;

    font-size: 3.5vmin;
}

.selected {
    background-color: #2196f3;
    color: #FFF;
}

.popupHeader>.popupHeaderTabs>.selected {
    border-left: 1px solid #2196f3;
    border-top: 1px solid #2196f3;
    border-right: 1px solid #2196f3;
    border-bottom: 0px;
}

.popup>.popupContent {
    /* at start not visible, controlled by jQuery */
    display: none;
    position: absolute;
    top: 20%;
    left: 25px;
    right: 25px;

    bottom: 1%;
    overflow-y: scroll;

    padding: 1%;

    border-left: 1px solid;
    border-right: 1px solid;
    border-bottom: 1px solid;
    border-radius: 0px 0px 5px 5px;
}

.tableCell {
    float: left;
    width: 50%;
    padding: 1vmin;
}

.tablecell>div {
    border-bottom: 1px solid;
    font-weight: bold;
}

.tablecell>i {
    font-size: 1.3vmin;
}

#networks {
    max-height: 20%;
    overflow-x: hidden;
    overflow-y: auto;
    background-color: rgb(238, 237, 237);
    padding: 5px;
    border-radius: 4px;
    scrollbar-width: thin;
}

.form-button,
input {
    width: 100%;
    height: 44px;
    border-radius: 4px;
    margin: 10px auto;
    font-size: 15px;
    display: block;
    font-weight: bold;
}

input {
    background: #f1f1f1;
    border: 0;
    padding: 0 15px
}

input[type=checkbox] {
    width: 1.5em;
    height: 1.5em;
    display: inline;
    position: relative;
    top: 0.3em;
}

input[type=file] {
    width: 1.5em;
    height: 1.5em;
    display: inline;
    position: relative;
    top: 0.3em;
}

#frame {
    background: #000000;
    /* max-width: 100%; */
    /* margin: 5px; */
    padding: 20px;
    border-radius: 10px;
    height: 100%;
    text-align: center;
}

#file-input {
    padding: 0;
    border: 1px solid #ddd;
    line-height: 44px;
    text-align: left;
    display: block;
    cursor: pointer;
}

#bar,
#prgbar {
    background-color: #222222;
    border-radius: 10px
}

#bar {
    background-color: #3498db;
    width: 0%;
    height: 10px
}

/* form {
    background: #fff;
    max-width: 100%;
    margin: 50px auto;
    padding: 30px;
    border-radius: 5px;
    text-align: center
} */

.btn {
    background: #3498db;
    color: #fff;
    padding: 5px;
    cursor: pointer;
    padding-top: 0.8em;
}

table {
    font-size: 14px;
    width: 100%;
}

.col2 {
    text-align: right;
}

td {
    /* border-bottom-style: groove;
    border-bottom-width: 1px; */
    text-align: center;
}

.switch {
    top: -13px;
    position: relative;
    display: inline-block;
    width: 60px;
    /* Adjusted width */
    height: 34px;
    /* Adjusted height */
}

.switch input {
    opacity: 0;
    width: 0;
    height: 0;
}

.slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    -webkit-transition: .4s;
    transition: .4s;
    border-radius: 34px;
    /* Adjusted for roundness */
}

.slider:before {
    position: absolute;
    content: '';
    height: 26px;
    /* Adjusted size */
    width: 26px;
    /* Adjusted size */
    left: 4px;
    bottom: 4px;
    background-color: white;
    -webkit-transition: .4s;
    transition: .4s;
    border-radius: 50%;
}

input:checked+.slider {
    background-color: #2196f3;
}

input:checked+.slider:before {
    -webkit-transform: translateX(26px);
    transform: translateX(26px);
}

#debug,
#tempOut,
#apiOut {
    background-color: lightgray;
    font-family: monospace;
    font-size: small;
    border-radius: 4px;
    text-align: left;
    padding: 4px;
}

#firmware,
#builddate {
    color: lightblue;
    font-size: 2vmin;
    font-style: italic;
}

#footer_left {
    float: left;
    width: 40%;
}

#footer_center {
    float: left;
    width: 20%;
}

#footer_right {
    float: left;
    width: 40%;
}

@media (orientation: portrait) {

    /* @media (max-width: 820px) { */
    /* @media (max-width: 739px) or (max-aspect-ratio: 10/9) { */
    /* @media (max-aspect-ratio: 10/8) { */
    body {
        /* font-size: 2.7vmin; */
    }

    .alert {
        /* display: flex; */
        left: 2%;
        width: 96%;
        height: 7%;
        font-size: small;
    }

    .column {
        width: 32.9%;
        height: 33%;
        /* (100 / 3) - (6 * 0,3) = 32,133 */
        font-size: 1.5em;
    }

    /* .panelValueBox {
        padding-top: 15%;
    } */

    #inverter_energy .panelValueBoxDetail,
    #connection_state .panelValueBoxDetail {
        /* color: rgb(119, 0, 255); */
        /* font-size: 3.0vmin; */
        font-size: 90%;
        float: left;
        width: 50%;
        margin-top: 0.1%;
    }

    .panelValueBoxDetail {
        /* display: none; */
        font-size: 3.0vmin;
        /* float: none; */
        width: 100%;
        border-bottom: #2196f3;
        border-width: 1px;
        margin-top: 5%;
    }

    .panelHead {
        /* float: left; */
        width: 100%;
        padding-bottom: 2px;
    }

    #time {
        display: none;
    }

    #inverter_energy,
    #connection_state {
        width: 99.3%;
        /* float:none; */
    }

    .panelValue {
        /* padding-top: 23%; */
        /* font-size: 7vmin; */
        font-size: 1.5em;
    }

    .panelValueSmall {
        /* font-size: 3vmin; */
        font-size: 1.4em;
    }

    #firmware,
    #builddate,
    #screensize {
        /* color: lightblue; */
        font-size: 2.5vmin;
        font-style: italic;
    }

    #footer_left {
        /* visibility: hidden; */
        /* display: none; */
        width: 70%;
        float: left;
    }

    #footer_center {
        display: none;
        padding-top: 2.5%;
        width: 48%;
    }

    #footer_right {
        width: 30%;
        /* font-size: 10vmin; */
    }

    .notification .badge {
        height: 2vh;
        width: 2vh;
    }

    td {
        float: left;
    }

    .warningColumnDetail {
        display: none;
    }
    #warningDetailsHint {
        display: block;
    }

    .popup2 {
        width: 80%;
        left: 10%;
        height: 60%;
        top: 20%;
    }
}

/* update progress */
.ui-progressbar-value {
    transition: width 0.25s;
    -webkit-transition: width 0.5s;
    background: #3498db;
}

.updateChannel {
    border: 1px solid #3498db;
    text-align: center;
    cursor: pointer;
    width: 50%;
}

.passcheck {
    cursor: pointer;
}

#wifiSearch::after {
    content: '';
    display: inline-block;
    width: 20px;
    height: 20px;
    border: 2px solid #3498db;
    border-top: 2px solid transparent;
    border-radius: 50%;
    margin-left: 5px;
    animation: spin 1s linear infinite;
}

@keyframes spin {
    0% {
        transform: rotate(0deg);
    }

    100% {
        transform: rotate(360deg);
    }
}

@keyframes fade {

    0%,
    100% {
        opacity: 1;
    }

    50% {
        opacity: 0;
    }
}

.fa-power-off {
    animation: fade 2s infinite;
}

/* Inline SVG icons - replaces Font Awesome CDN */
.fa-solid,.fa{display:inline-block;width:1em;height:1em;background-color:currentColor;-webkit-mask-repeat:no-repeat;mask-repeat:no-repeat;-webkit-mask-position:center;mask-position:center;-webkit-mask-size:contain;mask-size:contain;vertical-align:-0.125em}
.fa-sliders{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M0 416c0 17.7 14.3 32 32 32l54.7 0c12.3 28.3 40.5 48 73.3 48s61-19.7 73.3-48L480 448c17.7 0 32-14.3 32-32s-14.3-32-32-32l-246.7 0c-12.3-28.3-40.5-48-73.3-48s-61 19.7-73.3 48L32 384c-17.7 0-32 14.3-32 32zm128 0a32 32 0 1 1 64 0 32 32 0 1 1 -64 0zM320 256a32 32 0 1 1 64 0 32 32 0 1 1 -64 0zm32-80c-32.8 0-61 19.7-73.3 48L32 224c-17.7 0-32 14.3-32 32s14.3 32 32 32l246.7 0c12.3 28.3 40.5 48 73.3 48s61-19.7 73.3-48l54.7 0c17.7 0 32-14.3 32-32s-14.3-32-32-32l-54.7 0c-12.3-28.3-40.5-48-73.3-48zM192 128a32 32 0 1 1 0-64 32 32 0 1 1 0 64zm73.3-64C253 35.7 224.8 16 192 16s-61 19.7-73.3 48L32 64C14.3 64 0 78.3 0 96s14.3 32 32 32l86.7 0c12.3 28.3 40.5 48 73.3 48s61-19.7 73.3-48L480 128c17.7 0 32-14.3 32-32s-14.3-32-32-32L265.3 64z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M0 416c0 17.7 14.3 32 32 32l54.7 0c12.3 28.3 40.5 48 73.3 48s61-19.7 73.3-48L480 448c17.7 0 32-14.3 32-32s-14.3-32-32-32l-246.7 0c-12.3-28.3-40.5-48-73.3-48s-61 19.7-73.3 48L32 384c-17.7 0-32 14.3-32 32zm128 0a32 32 0 1 1 64 0 32 32 0 1 1 -64 0zM320 256a32 32 0 1 1 64 0 32 32 0 1 1 -64 0zm32-80c-32.8 0-61 19.7-73.3 48L32 224c-17.7 0-32 14.3-32 32s14.3 32 32 32l246.7 0c12.3 28.3 40.5 48 73.3 48s61-19.7 73.3-48l54.7 0c17.7 0 32-14.3 32-32s-14.3-32-32-32l-54.7 0c-12.3-28.3-40.5-48-73.3-48zM192 128a32 32 0 1 1 0-64 32 32 0 1 1 0 64zm73.3-64C253 35.7 224.8 16 192 16s-61 19.7-73.3 48L32 64C14.3 64 0 78.3 0 96s14.3 32 32 32l86.7 0c12.3 28.3 40.5 48 73.3 48s61-19.7 73.3-48L480 128c17.7 0 32-14.3 32-32s-14.3-32-32-32L265.3 64z'/%3E%3C/svg%3E")}
.fa-cloud-download{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M144 480C64.5 480 0 415.5 0 336c0-62.8 40.2-116.2 96.2-135.9c-.1-2.7-.2-5.4-.2-8.1c0-88.4 71.6-160 160-160c59.3 0 111 32.2 138.7 80.2C409.9 102 428.3 96 448 96c53 0 96 43 96 96c0 12.2-2.3 23.8-6.4 34.6C596 238.4 640 290.1 640 352c0 70.7-57.3 128-128 128l-368 0zm79-167l80 80c9.4 9.4 24.6 9.4 33.9 0l80-80c9.4-9.4 9.4-24.6 0-33.9s-24.6-9.4-33.9 0l-39 39L344 184c0-13.3-10.7-24-24-24s-24 10.7-24 24l0 134.1-39-39c-9.4-9.4-24.6-9.4-33.9 0s-9.4 24.6 0 33.9z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M144 480C64.5 480 0 415.5 0 336c0-62.8 40.2-116.2 96.2-135.9c-.1-2.7-.2-5.4-.2-8.1c0-88.4 71.6-160 160-160c59.3 0 111 32.2 138.7 80.2C409.9 102 428.3 96 448 96c53 0 96 43 96 96c0 12.2-2.3 23.8-6.4 34.6C596 238.4 640 290.1 640 352c0 70.7-57.3 128-128 128l-368 0zm79-167l80 80c9.4 9.4 24.6 9.4 33.9 0l80-80c9.4-9.4 9.4-24.6 0-33.9s-24.6-9.4-33.9 0l-39 39L344 184c0-13.3-10.7-24-24-24s-24 10.7-24 24l0 134.1-39-39c-9.4-9.4-24.6-9.4-33.9 0s-9.4 24.6 0 33.9z'/%3E%3C/svg%3E")}
.fa-gears{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M308.5 135.3c7.1-6.3 9.9-16.2 6.2-25c-2.3-5.3-4.8-10.5-7.6-15.5L304 89.4c-3-5-6.3-9.9-9.8-14.6c-5.7-7.6-15.7-10.1-24.7-7.1l-28.2 9.3c-10.7-8.8-23-16-36.2-20.9L199 27.1c-1.9-9.3-9.1-16.7-18.5-17.8C173.9 8.4 167.2 8 160.4 8l-.7 0c-6.8 0-13.5 .4-20.1 1.2c-9.4 1.1-16.6 8.6-18.5 17.8L115 56.1c-13.3 5-25.5 12.1-36.2 20.9L50.5 67.8c-9-3-19-.5-24.7 7.1c-3.5 4.7-6.8 9.6-9.9 14.6l-3 5.3c-2.8 5-5.3 10.2-7.6 15.6c-3.7 8.7-.9 18.6 6.2 25l22.2 19.8C32.6 161.9 32 168.9 32 176s.6 14.1 1.7 20.9L11.5 216.7c-7.1 6.3-9.9 16.2-6.2 25c2.3 5.3 4.8 10.5 7.6 15.6l3 5.2c3 5.1 6.3 9.9 9.9 14.6c5.7 7.6 15.7 10.1 24.7 7.1l28.2-9.3c10.7 8.8 23 16 36.2 20.9l6.1 29.1c1.9 9.3 9.1 16.7 18.5 17.8c6.7 .8 13.5 1.2 20.4 1.2s13.7-.4 20.4-1.2c9.4-1.1 16.6-8.6 18.5-17.8l6.1-29.1c13.3-5 25.5-12.1 36.2-20.9l28.2 9.3c9 3 19 .5 24.7-7.1c3.5-4.7 6.8-9.5 9.8-14.6l3.1-5.4c2.8-5 5.3-10.2 7.6-15.5c3.7-8.7 .9-18.6-6.2-25l-22.2-19.8c1.1-6.8 1.7-13.8 1.7-20.9s-.6-14.1-1.7-20.9l22.2-19.8zM112 176a48 48 0 1 1 96 0 48 48 0 1 1 -96 0zM504.7 500.5c6.3 7.1 16.2 9.9 25 6.2c5.3-2.3 10.5-4.8 15.5-7.6l5.4-3.1c5-3 9.9-6.3 14.6-9.8c7.6-5.7 10.1-15.7 7.1-24.7l-9.3-28.2c8.8-10.7 16-23 20.9-36.2l29.1-6.1c9.3-1.9 16.7-9.1 17.8-18.5c.8-6.7 1.2-13.5 1.2-20.4s-.4-13.7-1.2-20.4c-1.1-9.4-8.6-16.6-17.8-18.5L583.9 307c-5-13.3-12.1-25.5-20.9-36.2l9.3-28.2c3-9 .5-19-7.1-24.7c-4.7-3.5-9.6-6.8-14.6-9.9l-5.3-3c-5-2.8-10.2-5.3-15.6-7.6c-8.7-3.7-18.6-.9-25 6.2l-19.8 22.2c-6.8-1.1-13.8-1.7-20.9-1.7s-14.1 .6-20.9 1.7l-19.8-22.2c-6.3-7.1-16.2-9.9-25-6.2c-5.3 2.3-10.5 4.8-15.6 7.6l-5.2 3c-5.1 3-9.9 6.3-14.6 9.9c-7.6 5.7-10.1 15.7-7.1 24.7l9.3 28.2c-8.8 10.7-16 23-20.9 36.2L315.1 313c-9.3 1.9-16.7 9.1-17.8 18.5c-.8 6.7-1.2 13.5-1.2 20.4s.4 13.7 1.2 20.4c1.1 9.4 8.6 16.6 17.8 18.5l29.1 6.1c5 13.3 12.1 25.5 20.9 36.2l-9.3 28.2c-3 9-.5 19 7.1 24.7c4.7 3.5 9.5 6.8 14.6 9.8l5.4 3.1c5 2.8 10.2 5.3 15.5 7.6c8.7 3.7 18.6 .9 25-6.2l19.8-22.2c6.8 1.1 13.8 1.7 20.9 1.7s14.1-.6 20.9-1.7l19.8 22.2zM464 304a48 48 0 1 1 0 96 48 48 0 1 1 0-96z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M308.5 135.3c7.1-6.3 9.9-16.2 6.2-25c-2.3-5.3-4.8-10.5-7.6-15.5L304 89.4c-3-5-6.3-9.9-9.8-14.6c-5.7-7.6-15.7-10.1-24.7-7.1l-28.2 9.3c-10.7-8.8-23-16-36.2-20.9L199 27.1c-1.9-9.3-9.1-16.7-18.5-17.8C173.9 8.4 167.2 8 160.4 8l-.7 0c-6.8 0-13.5 .4-20.1 1.2c-9.4 1.1-16.6 8.6-18.5 17.8L115 56.1c-13.3 5-25.5 12.1-36.2 20.9L50.5 67.8c-9-3-19-.5-24.7 7.1c-3.5 4.7-6.8 9.6-9.9 14.6l-3 5.3c-2.8 5-5.3 10.2-7.6 15.6c-3.7 8.7-.9 18.6 6.2 25l22.2 19.8C32.6 161.9 32 168.9 32 176s.6 14.1 1.7 20.9L11.5 216.7c-7.1 6.3-9.9 16.2-6.2 25c2.3 5.3 4.8 10.5 7.6 15.6l3 5.2c3 5.1 6.3 9.9 9.9 14.6c5.7 7.6 15.7 10.1 24.7 7.1l28.2-9.3c10.7 8.8 23 16 36.2 20.9l6.1 29.1c1.9 9.3 9.1 16.7 18.5 17.8c6.7 .8 13.5 1.2 20.4 1.2s13.7-.4 20.4-1.2c9.4-1.1 16.6-8.6 18.5-17.8l6.1-29.1c13.3-5 25.5-12.1 36.2-20.9l28.2 9.3c9 3 19 .5 24.7-7.1c3.5-4.7 6.8-9.5 9.8-14.6l3.1-5.4c2.8-5 5.3-10.2 7.6-15.5c3.7-8.7 .9-18.6-6.2-25l-22.2-19.8c1.1-6.8 1.7-13.8 1.7-20.9s-.6-14.1-1.7-20.9l22.2-19.8zM112 176a48 48 0 1 1 96 0 48 48 0 1 1 -96 0zM504.7 500.5c6.3 7.1 16.2 9.9 25 6.2c5.3-2.3 10.5-4.8 15.5-7.6l5.4-3.1c5-3 9.9-6.3 14.6-9.8c7.6-5.7 10.1-15.7 7.1-24.7l-9.3-28.2c8.8-10.7 16-23 20.9-36.2l29.1-6.1c9.3-1.9 16.7-9.1 17.8-18.5c.8-6.7 1.2-13.5 1.2-20.4s-.4-13.7-1.2-20.4c-1.1-9.4-8.6-16.6-17.8-18.5L583.9 307c-5-13.3-12.1-25.5-20.9-36.2l9.3-28.2c3-9 .5-19-7.1-24.7c-4.7-3.5-9.6-6.8-14.6-9.9l-5.3-3c-5-2.8-10.2-5.3-15.6-7.6c-8.7-3.7-18.6-.9-25 6.2l-19.8 22.2c-6.8-1.1-13.8-1.7-20.9-1.7s-14.1 .6-20.9 1.7l-19.8-22.2c-6.3-7.1-16.2-9.9-25-6.2c-5.3 2.3-10.5 4.8-15.6 7.6l-5.2 3c-5.1 3-9.9 6.3-14.6 9.9c-7.6 5.7-10.1 15.7-7.1 24.7l9.3 28.2c-8.8 10.7-16 23-20.9 36.2L315.1 313c-9.3 1.9-16.7 9.1-17.8 18.5c-.8 6.7-1.2 13.5-1.2 20.4s.4 13.7 1.2 20.4c1.1 9.4 8.6 16.6 17.8 18.5l29.1 6.1c5 13.3 12.1 25.5 20.9 36.2l-9.3 28.2c-3 9-.5 19 7.1 24.7c4.7 3.5 9.5 6.8 14.6 9.8l5.4 3.1c5 2.8 10.2 5.3 15.5 7.6c8.7 3.7 18.6 .9 25-6.2l19.8-22.2c6.8 1.1 13.8 1.7 20.9 1.7s14.1-.6 20.9-1.7l19.8 22.2zM464 304a48 48 0 1 1 0 96 48 48 0 1 1 0-96z'/%3E%3C/svg%3E")}
.fa-house-laptop{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M218.3 8.5c12.3-11.3 31.2-11.3 43.4 0l208 192c6.7 6.2 10.3 14.8 10.3 23.5l-144 0c-19.1 0-36.3 8.4-48 21.7l0-37.7c0-8.8-7.2-16-16-16l-64 0c-8.8 0-16 7.2-16 16l0 64c0 8.8 7.2 16 16 16l64 0 0 128-160 0c-26.5 0-48-21.5-48-48l0-112-32 0c-13.2 0-25-8.1-29.8-20.3s-1.6-26.2 8.1-35.2l208-192zM352 304l0 144 192 0 0-144-192 0zm-48-16c0-17.7 14.3-32 32-32l224 0c17.7 0 32 14.3 32 32l0 160 32 0c8.8 0 16 7.2 16 16c0 26.5-21.5 48-48 48l-48 0-192 0-48 0c-26.5 0-48-21.5-48-48c0-8.8 7.2-16 16-16l32 0 0-160z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M218.3 8.5c12.3-11.3 31.2-11.3 43.4 0l208 192c6.7 6.2 10.3 14.8 10.3 23.5l-144 0c-19.1 0-36.3 8.4-48 21.7l0-37.7c0-8.8-7.2-16-16-16l-64 0c-8.8 0-16 7.2-16 16l0 64c0 8.8 7.2 16 16 16l64 0 0 128-160 0c-26.5 0-48-21.5-48-48l0-112-32 0c-13.2 0-25-8.1-29.8-20.3s-1.6-26.2 8.1-35.2l208-192zM352 304l0 144 192 0 0-144-192 0zm-48-16c0-17.7 14.3-32 32-32l224 0c17.7 0 32 14.3 32 32l0 160 32 0c8.8 0 16 7.2 16 16c0 26.5-21.5 48-48 48l-48 0-192 0-48 0c-26.5 0-48-21.5-48-48c0-8.8 7.2-16 16-16l32 0 0-160z'/%3E%3C/svg%3E")}
.fa-plug-circle-bolt{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 576 512'%3E%3Cpath d='M96 0C78.3 0 64 14.3 64 32l0 96 64 0 0-96c0-17.7-14.3-32-32-32zM288 0c-17.7 0-32 14.3-32 32l0 96 64 0 0-96c0-17.7-14.3-32-32-32zM32 160c-17.7 0-32 14.3-32 32s14.3 32 32 32l0 32c0 77.4 55 142 128 156.8l0 67.2c0 17.7 14.3 32 32 32s32-14.3 32-32l0-67.2c12.3-2.5 24.1-6.4 35.1-11.5c-2.1-10.8-3.1-21.9-3.1-33.3c0-80.3 53.8-148 127.3-169.2c.5-2.2 .7-4.5 .7-6.8c0-17.7-14.3-32-32-32L32 160zM432 512a144 144 0 1 0 0-288 144 144 0 1 0 0 288zm47.9-225c4.3 3.7 5.4 9.9 2.6 14.9L452.4 356l35.6 0c5.2 0 9.8 3.3 11.4 8.2s-.1 10.3-4.2 13.4l-96 72c-4.5 3.4-10.8 3.2-15.1-.6s-5.4-9.9-2.6-14.9L411.6 380 376 380c-5.2 0-9.8-3.3-11.4-8.2s.1-10.3 4.2-13.4l96-72c4.5-3.4 10.8-3.2 15.1 .6z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 576 512'%3E%3Cpath d='M96 0C78.3 0 64 14.3 64 32l0 96 64 0 0-96c0-17.7-14.3-32-32-32zM288 0c-17.7 0-32 14.3-32 32l0 96 64 0 0-96c0-17.7-14.3-32-32-32zM32 160c-17.7 0-32 14.3-32 32s14.3 32 32 32l0 32c0 77.4 55 142 128 156.8l0 67.2c0 17.7 14.3 32 32 32s32-14.3 32-32l0-67.2c12.3-2.5 24.1-6.4 35.1-11.5c-2.1-10.8-3.1-21.9-3.1-33.3c0-80.3 53.8-148 127.3-169.2c.5-2.2 .7-4.5 .7-6.8c0-17.7-14.3-32-32-32L32 160zM432 512a144 144 0 1 0 0-288 144 144 0 1 0 0 288zm47.9-225c4.3 3.7 5.4 9.9 2.6 14.9L452.4 356l35.6 0c5.2 0 9.8 3.3 11.4 8.2s-.1 10.3-4.2 13.4l-96 72c-4.5 3.4-10.8 3.2-15.1-.6s-5.4-9.9-2.6-14.9L411.6 380 376 380c-5.2 0-9.8-3.3-11.4-8.2s.1-10.3 4.2-13.4l96-72c4.5-3.4 10.8-3.2 15.1 .6z'/%3E%3C/svg%3E")}
.fa-repeat{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M0 224c0 17.7 14.3 32 32 32s32-14.3 32-32c0-53 43-96 96-96l160 0 0 32c0 12.9 7.8 24.6 19.8 29.6s25.7 2.2 34.9-6.9l64-64c12.5-12.5 12.5-32.8 0-45.3l-64-64c-9.2-9.2-22.9-11.9-34.9-6.9S320 19.1 320 32l0 32L160 64C71.6 64 0 135.6 0 224zm512 64c0-17.7-14.3-32-32-32s-32 14.3-32 32c0 53-43 96-96 96l-160 0 0-32c0-12.9-7.8-24.6-19.8-29.6s-25.7-2.2-34.9 6.9l-64 64c-12.5 12.5-12.5 32.8 0 45.3l64 64c9.2 9.2 22.9 11.9 34.9 6.9s19.8-16.6 19.8-29.6l0-32 160 0c88.4 0 160-71.6 160-160z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M0 224c0 17.7 14.3 32 32 32s32-14.3 32-32c0-53 43-96 96-96l160 0 0 32c0 12.9 7.8 24.6 19.8 29.6s25.7 2.2 34.9-6.9l64-64c12.5-12.5 12.5-32.8 0-45.3l-64-64c-9.2-9.2-22.9-11.9-34.9-6.9S320 19.1 320 32l0 32L160 64C71.6 64 0 135.6 0 224zm512 64c0-17.7-14.3-32-32-32s-32 14.3-32 32c0 53-43 96-96 96l-160 0 0-32c0-12.9-7.8-24.6-19.8-29.6s-25.7-2.2-34.9 6.9l-64 64c-12.5 12.5-12.5 32.8 0 45.3l64 64c9.2 9.2 22.9 11.9 34.9 6.9s19.8-16.6 19.8-29.6l0-32 160 0c88.4 0 160-71.6 160-160z'/%3E%3C/svg%3E")}
.fa-rotate{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M142.9 142.9c-17.5 17.5-30.1 38-37.8 59.8c-5.9 16.7-24.2 25.4-40.8 19.5s-25.4-24.2-19.5-40.8C55.6 150.7 73.2 122 97.6 97.6c87.2-87.2 228.3-87.5 315.8-1L455 55c6.9-6.9 17.2-8.9 26.2-5.2s14.8 12.5 14.8 22.2l0 128c0 13.3-10.7 24-24 24l-8.4 0c0 0 0 0 0 0L344 224c-9.7 0-18.5-5.8-22.2-14.8s-1.7-19.3 5.2-26.2l41.1-41.1c-62.6-61.5-163.1-61.2-225.3 1zM16 312c0-13.3 10.7-24 24-24l7.6 0 .7 0L168 288c9.7 0 18.5 5.8 22.2 14.8s1.7 19.3-5.2 26.2l-41.1 41.1c62.6 61.5 163.1 61.2 225.3-1c17.5-17.5 30.1-38 37.8-59.8c5.9-16.7 24.2-25.4 40.8-19.5s25.4 24.2 19.5 40.8c-10.8 30.6-28.4 59.3-52.9 83.8c-87.2 87.2-228.3 87.5-315.8 1L57 457c-6.9 6.9-17.2 8.9-26.2 5.2S16 449.7 16 440l0-119.6 0-.7 0-7.6z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M142.9 142.9c-17.5 17.5-30.1 38-37.8 59.8c-5.9 16.7-24.2 25.4-40.8 19.5s-25.4-24.2-19.5-40.8C55.6 150.7 73.2 122 97.6 97.6c87.2-87.2 228.3-87.5 315.8-1L455 55c6.9-6.9 17.2-8.9 26.2-5.2s14.8 12.5 14.8 22.2l0 128c0 13.3-10.7 24-24 24l-8.4 0c0 0 0 0 0 0L344 224c-9.7 0-18.5-5.8-22.2-14.8s-1.7-19.3 5.2-26.2l41.1-41.1c-62.6-61.5-163.1-61.2-225.3 1zM16 312c0-13.3 10.7-24 24-24l7.6 0 .7 0L168 288c9.7 0 18.5 5.8 22.2 14.8s1.7 19.3-5.2 26.2l-41.1 41.1c62.6 61.5 163.1 61.2 225.3-1c17.5-17.5 30.1-38 37.8-59.8c5.9-16.7 24.2-25.4 40.8-19.5s25.4 24.2 19.5 40.8c-10.8 30.6-28.4 59.3-52.9 83.8c-87.2 87.2-228.3 87.5-315.8 1L57 457c-6.9 6.9-17.2 8.9-26.2 5.2S16 449.7 16 440l0-119.6 0-.7 0-7.6z'/%3E%3C/svg%3E")}
.fa-wifi{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M54.2 202.9C123.2 136.7 216.8 96 320 96s196.8 40.7 265.8 106.9c12.8 12.2 33 11.8 45.2-.9s11.8-33-.9-45.2C549.7 79.5 440.4 32 320 32S90.3 79.5 9.8 156.7C-2.9 169-3.3 189.2 8.9 202s32.5 13.2 45.2 .9zM320 256c56.8 0 108.6 21.1 148.2 56c13.3 11.7 33.5 10.4 45.2-2.8s10.4-33.5-2.8-45.2C459.8 219.2 393 192 320 192s-139.8 27.2-190.5 72c-13.3 11.7-14.5 31.9-2.8 45.2s31.9 14.5 45.2 2.8c39.5-34.9 91.3-56 148.2-56zm64 160a64 64 0 1 0 -128 0 64 64 0 1 0 128 0z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M54.2 202.9C123.2 136.7 216.8 96 320 96s196.8 40.7 265.8 106.9c12.8 12.2 33 11.8 45.2-.9s11.8-33-.9-45.2C549.7 79.5 440.4 32 320 32S90.3 79.5 9.8 156.7C-2.9 169-3.3 189.2 8.9 202s32.5 13.2 45.2 .9zM320 256c56.8 0 108.6 21.1 148.2 56c13.3 11.7 33.5 10.4 45.2-2.8s10.4-33.5-2.8-45.2C459.8 219.2 393 192 320 192s-139.8 27.2-190.5 72c-13.3 11.7-14.5 31.9-2.8 45.2s31.9 14.5 45.2 2.8c39.5-34.9 91.3-56 148.2-56zm64 160a64 64 0 1 0 -128 0 64 64 0 1 0 128 0z'/%3E%3C/svg%3E")}
.fa-exclamation-triangle{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M256 32c14.2 0 27.3 7.5 34.5 19.8l216 368c7.3 12.4 7.3 27.7 .2 40.1S486.3 480 472 480L40 480c-14.3 0-27.6-7.7-34.7-20.1s-7-27.8 .2-40.1l216-368C228.7 39.5 241.8 32 256 32zm0 128c-13.3 0-24 10.7-24 24l0 112c0 13.3 10.7 24 24 24s24-10.7 24-24l0-112c0-13.3-10.7-24-24-24zm32 224a32 32 0 1 0 -64 0 32 32 0 1 0 64 0z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M256 32c14.2 0 27.3 7.5 34.5 19.8l216 368c7.3 12.4 7.3 27.7 .2 40.1S486.3 480 472 480L40 480c-14.3 0-27.6-7.7-34.7-20.1s-7-27.8 .2-40.1l216-368C228.7 39.5 241.8 32 256 32zm0 128c-13.3 0-24 10.7-24 24l0 112c0 13.3 10.7 24 24 24s24-10.7 24-24l0-112c0-13.3-10.7-24-24-24zm32 224a32 32 0 1 0 -64 0 32 32 0 1 0 64 0z'/%3E%3C/svg%3E")}
.fa-power-off{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M288 32c0-17.7-14.3-32-32-32s-32 14.3-32 32l0 224c0 17.7 14.3 32 32 32s32-14.3 32-32l0-224zM143.5 120.6c13.6-11.3 15.4-31.5 4.1-45.1s-31.5-15.4-45.1-4.1C49.7 115.4 16 181.8 16 256c0 132.5 107.5 240 240 240s240-107.5 240-240c0-74.2-33.8-140.6-86.6-184.6c-13.6-11.3-33.8-9.4-45.1 4.1s-9.4 33.8 4.1 45.1c38.9 32.3 63.5 81 63.5 135.4c0 97.2-78.8 176-176 176s-176-78.8-176-176c0-54.4 24.7-103.1 63.5-135.4z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'%3E%3Cpath d='M288 32c0-17.7-14.3-32-32-32s-32 14.3-32 32l0 224c0 17.7 14.3 32 32 32s32-14.3 32-32l0-224zM143.5 120.6c13.6-11.3 15.4-31.5 4.1-45.1s-31.5-15.4-45.1-4.1C49.7 115.4 16 181.8 16 256c0 132.5 107.5 240 240 240s240-107.5 240-240c0-74.2-33.8-140.6-86.6-184.6c-13.6-11.3-33.8-9.4-45.1 4.1s-9.4 33.8 4.1 45.1c38.9 32.3 63.5 81 63.5 135.4c0 97.2-78.8 176-176 176s-176-78.8-176-176c0-54.4 24.7-103.1 63.5-135.4z'/%3E%3C/svg%3E")}
.fa-hourglass-start{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 384 512'%3E%3Cpath d='M32 0C14.3 0 0 14.3 0 32S14.3 64 32 64l0 11c0 42.4 16.9 83.1 46.9 113.1L146.7 256 78.9 323.9C48.9 353.9 32 394.6 32 437l0 11c-17.7 0-32 14.3-32 32s14.3 32 32 32l32 0 256 0 32 0c17.7 0 32-14.3 32-32s-14.3-32-32-32l0-11c0-42.4-16.9-83.1-46.9-113.1L237.3 256l67.9-67.9c30-30 46.9-70.7 46.9-113.1l0-11c17.7 0 32-14.3 32-32s-14.3-32-32-32L320 0 64 0 32 0zM288 437l0 11L96 448l0-11c0-25.5 10.1-49.9 28.1-67.9L192 301.3l67.9 67.9c18 18 28.1 42.4 28.1 67.9z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 384 512'%3E%3Cpath d='M32 0C14.3 0 0 14.3 0 32S14.3 64 32 64l0 11c0 42.4 16.9 83.1 46.9 113.1L146.7 256 78.9 323.9C48.9 353.9 32 394.6 32 437l0 11c-17.7 0-32 14.3-32 32s14.3 32 32 32l32 0 256 0 32 0c17.7 0 32-14.3 32-32s-14.3-32-32-32l0-11c0-42.4-16.9-83.1-46.9-113.1L237.3 256l67.9-67.9c30-30 46.9-70.7 46.9-113.1l0-11c17.7 0 32-14.3 32-32s-14.3-32-32-32L320 0 64 0 32 0zM288 437l0 11L96 448l0-11c0-25.5 10.1-49.9 28.1-67.9L192 301.3l67.9 67.9c18 18 28.1 42.4 28.1 67.9z'/%3E%3C/svg%3E")}
.fa-signal{-webkit-mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M576 0c17.7 0 32 14.3 32 32l0 448c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-448c0-17.7 14.3-32 32-32zM448 96c17.7 0 32 14.3 32 32l0 352c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-352c0-17.7 14.3-32 32-32zM352 224l0 256c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-256c0-17.7 14.3-32 32-32s32 14.3 32 32zM192 288c17.7 0 32 14.3 32 32l0 160c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-160c0-17.7 14.3-32 32-32zM96 416l0 64c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-64c0-17.7 14.3-32 32-32s32 14.3 32 32z'/%3E%3C/svg%3E");mask-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'%3E%3Cpath d='M576 0c17.7 0 32 14.3 32 32l0 448c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-448c0-17.7 14.3-32 32-32zM448 96c17.7 0 32 14.3 32 32l0 352c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-352c0-17.7 14.3-32 32-32zM352 224l0 256c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-256c0-17.7 14.3-32 32-32s32 14.3 32 32zM192 288c17.7 0 32 14.3 32 32l0 160c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-160c0-17.7 14.3-32 32-32zM96 416l0 64c0 17.7-14.3 32-32 32s-32-14.3-32-32l0-64c0-17.7 14.3-32 32-32s32 14.3 32 32z'/%3E%3C/svg%3E")}
)=====";