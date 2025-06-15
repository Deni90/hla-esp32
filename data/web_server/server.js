class WifiInfo {
    constructor(hostname, ssid, password) {
        this.hostname = hostname
        this.ssid = ssid;
        this.password = password;
    }
    toJson() {
        return {
            "hostname": this.hostname,
            "SSID": this.ssid,
            "password": this.password,
        };
    }
    static Builder = class {
        fromJson(message) {
            return new WifiInfo(message.hostname, message.SSID, atob(message.password));
        }
    }
}

var wifiInfo = null;

function openTab(id, tabName) {
    var i, tabcontent, tablinks;
    tabcontent = document.getElementById(id).children;
    for (i = 0; i < tabcontent.length; i++) {
        var tab = tabcontent[i].id.toString().replace("Button", "");
        document.getElementById(tab).style.display = "none";
    }
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].className = tabcontent[i].className.replace(" active", "");
    }
    document.getElementById(id + tabName).style.display = "block";
    document.getElementById(id + "Button" + tabName).className += " active";

    var tmp = 0xaa;
    liftplanActiveTable.clearTable();
    for (let i = 0; i < 10; i++) {
        liftplanActiveTable.addRow(tmp);
        tmp = tmp + 1;
    }
    currentStatusTable = document.getElementById("currentStatusTable");
    while (currentStatusTable.rows.length > 0) {
            currentStatusTable.deleteRow(-1);
        }
    var selectedRow = 4;
    for (let i = 0; i < liftplanActiveTable.getRowsLength(); i++) {
        const tr = document.createElement('tr');
        const td = document.createElement('td');
        if(i == selectedRow) {
            const label = document.createElement('label');
            label.innerHTML = "&larr;";
            td.appendChild(label);
        }

        tr.appendChild(td);
        currentStatusTable.appendChild(tr);
    }
}

window.addEventListener('load', function () {
    getWifiInfo();
    var tabcontent = document.getElementById("mainTab").children;
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.width = 100.0 / tabcontent.length + "%";
    }
    openTab("mainTab", "Settings");
});

function getWifiInfo() {
    fetch('/wifi')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            wifiInfo = new WifiInfo.Builder().fromJson(data);
            document.getElementById("hostname").value = wifiInfo.hostname;
            document.getElementById("ssid").value = wifiInfo.ssid;
            document.getElementById("password").value = wifiInfo.password;
            document.getElementById("passwordVerify").value = wifiInfo.password;
        })
        .catch(error => {
            console.error('There was a problem with the getting wifi info:', error);
        });
}

function setWifiInfo() {
    var logLabel = document.getElementById("wifiLog");
    var errorColor = "#ff6457"
    if (document.getElementById("hostname").value == "") {
        console.log("Error: Hostname cannot be empty");
        logLabel.innerHTML = "Error: Hostname cannot be empty";
        logLabel.style.color = errorColor;
        return;
    }
    if (document.getElementById("ssid").value == "") {
        console.log("Error: Wifi SSID cannot be empty");
        logLabel.innerHTML = "Error: Wifi SSID cannot be empty";
        logLabel.style.color = errorColor;
        return;
    }
    if (document.getElementById("password").value == "") {
        console.log("Wifi password cannot be empty");
        logLabel.innerHTML = "Error: Wifi password cannot be empty";
        logLabel.style.color = errorColor;
        return;
    }
    if (document.getElementById("password").value != document.getElementById("passwordVerify").value) {
        console.log("Wifi passwords are not matching");
        logLabel.innerHTML = "Error: Passwords are not matching";
        logLabel.style.color = errorColor;
        return;
    }
    logLabel.innerHTML = "";
    wifiInfo.hostname = document.getElementById("hostname").value;
    wifiInfo.ssid = document.getElementById("ssid").value;
    wifiInfo.password = btoa(document.getElementById("password").value);
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(wifiInfo.toJson())
    };
    fetch("/wifi", requestOptions)
        .then(response => {
            if (response.ok) {
                logLabel.innerHTML = "Restarting...";
                logLabel.style.color = "black";
            } else {
                throw new Error('Network response was not ok');
            }
        });
}