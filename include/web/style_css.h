const char STYLE_CSS[] PROGMEM = R"=====(

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

    display: flex;
    justify-content: center;
    align-items: center;
    text-align: center;
    background-color: #1c1c1c;
    font-size: 5vmin;

    border-radius: 10px;
    border-width: 1px;
    border-color: #2196f3;
    border-style: solid;
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
    top: -3.7em;
    right: -1.8em;
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
    position: absolute;
    top: 8%;
    right: 2.2em;
    font-size: 0.7em;
    cursor: pointer;
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

#popup2:after {
    position: fixed;
    content: "";
    top: 0;
    left: 0;
    bottom: 0;
    right: 0;
    background: rgba(0, 0, 0, 0.5);
    z-index: -2;
}

#popup2:before {
    position: absolute;
    content: "";
    top: 0;
    left: 0;
    bottom: 0;
    right: 0;
    background: #FFF;
    border-width: 1px;
    border-color: #2196f3;
    border-style: solid;
    border-radius: 10px;
    z-index: -1;
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

/* .switch {
    position: relative;
    display: inline-block;
    width: 30px;
    height: 17px
}

.switch input {
    opacity: 0;
    width: 0;
    height: 0
} */

/* .slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    -webkit-transition: .4s;
    transition: .4s
}

.slider:before {
    position: absolute;
    content: '';
    height: 13px;
    width: 13px;
    left: 2px;
    bottom: 2px;
    background-color: #fff;
    -webkit-transition: .4s;
    transition: .4s
}

.slider.round {
    border-radius: 17px
}

.slider.round:before {
    border-radius: 50%
} */

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
        width: 76%;
        float: left;
    }

    #footer_center {
        display: none;
        padding-top: 2.5%;
        width: 48%;
    }

    #footer_right {
        width: 20%;
        /* font-size: 10vmin; */
    }

    .notification .badge {
        height: 2vh;
        width: 2vh;
    }

    td {
        float: left;
    }
    #dtuWarnings {
        font-size: 0.8em;
        top: 2.1em;
        right: 2.3em;
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

)=====";