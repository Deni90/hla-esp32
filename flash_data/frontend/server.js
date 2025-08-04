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

class LiftPlan {
    constructor(name, isDisabled = false) {
        this.name = name;
        this.isDisabled = isDisabled;
    }

    #createCell(lifted) {
        const td = document.createElement('td');
        td.className = lifted ? 'lifted' : 'idle';
        if (this.isDisabled) {
            td.className = lifted ? 'lifted-disabled' : 'idle';
            td.style.pointerEvents = "none";
        } else {
            td.onclick = () => {
                td.classList.toggle('lifted');
                td.classList.toggle('idle');
            };
        }
        return td;
    }

    addRow(hex = null) {
        const tr = document.createElement('tr');
        let binaryStr = hex ? hex.toString(2).padStart(8, '0') : "00000000";
        for (let j = 0; j < 8; j++) {
            tr.appendChild(this.#createCell(binaryStr[7 - j] === '1', this.isDisabled));
        }
        let liftplanTable = document.getElementById(this.name);
        liftplanTable.appendChild(tr);
    }

    removeLastRow() {
        let liftplanTable = document.getElementById(this.name);
        if (liftplanTable.rows.length > 0) {
            liftplanTable.deleteRow(-1);
        }
    }

    clearTable() {
        let liftplanTable = document.getElementById(this.name);
        while (liftplanTable.rows.length > 0) {
            liftplanTable.deleteRow(-1);
        }
    }

    toHexArray() {
        let liftplanTable = document.getElementById(this.name);
        let resultArray = [];
        for (let i = 0; i < liftplanTable.rows.length; i++) {
            let row = liftplanTable.rows[i];
            let binaryStr = '';
            for (let j = 0; j < row.cells.length; j++) {
                binaryStr += row.cells[row.cells.length - 1 - j].classList.contains('lifted') ? '1' : '0';
            }
            let hexVal = parseInt(binaryStr, 2).toString(16).padStart(2, '0');
            resultArray.push(`0x${hexVal}`);
        }
        return resultArray;
    }

    getRowsLength() {
        let liftplanTable = document.getElementById(this.name);
        return liftplanTable.rows.length;
    }

    populateFromArray(array) {
        this.clearTable();
        array.forEach(value => {
            this.addRow(parseInt(value.replace('0x', ''), 16));
        });
    }

    highlightRow(index) {
        let liftplanTable = document.getElementById(this.name);
        if (index >= liftplanTable.rows.length) {
            return;
        }
        for (var r = 0, n = liftplanTable.rows.length; r < n; r++) {
            for (var c = 0, m = liftplanTable.rows[r].cells.length; c < m; c++) {
                if (r == index) {
                    liftplanTable.rows[r].cells[c].style["boxShadow"] = "0 0 10px rgba(0, 0, 255, 1)";
                } else {
                    liftplanTable.rows[r].cells[c].style["boxShadow"] = "0 1px 3px rgba(0, 0, 0, .2)";
                }
            }
        }
    }
}

var liftplanEditableTable = new LiftPlan("liftplanEditableTable");
var loomIntervalId = 0;
var loomInfo = null;

function openTab(id, tabName) {
    var i, tabcontent;
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
}

function disableTab(id, tabName) {
    document.getElementById(id + "Button" + tabName).disabled = true;
}

function enableTab(id, tabName) {
    document.getElementById(id + "Button" + tabName).disabled = false;
}

function setEqualTabButtonWidth(id) {
    var tabcontent = document.getElementById(id).children;
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.width = 100.0 / tabcontent.length + "%";
    }
}

window.addEventListener('load', function () {
    setEqualTabButtonWidth("mainTab");
    setEqualTabButtonWidth("liftplanTab");
    openMainTab("mainTab", "Dashboard");
});

function openMainTab(id, tabName) {
    if (tabName == "Dashboard") {
        getLoomState();
        handleLiftplanSelection();
    } else if (tabName == "Settings") {
        getWifiInfo();
    }
    openTab(id, tabName)
}

function getWifiInfo() {
    fetch('/api/v1/wifi')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            console.log("Wifi info = " + data);
            let wifiInfo = new WifiInfo.Builder().fromJson(data);
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
    var wifiInfo = new WifiInfo(
        document.getElementById("hostname").value,
        document.getElementById("ssid").value,
        btoa(document.getElementById("password").value)
    );
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(wifiInfo.toJson())
    };
    fetch("/api/v1/wifi", requestOptions)
        .then(response => {
            if (response.ok) {
                logLabel.innerHTML = "Restarting...";
                logLabel.style.color = "black";
            } else {
                throw new Error('Network response was not ok');
            }
        });
}

function getLiftplan(name, dest) {
    fetch('/api/v1/liftplan?name=' + name)
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            let liftplanTable = new LiftPlan(dest, true);
            liftplanTable.populateFromArray(data);
        })
        .catch(error => {
            console.error('There was a problem with the getting liftplan:', error);
        });
}

function saveLiftplan(liftplan) {
    var logLabel = document.getElementById("liftplanEditorLog");
    var errorColor = "#ff6457"
    var liftplanName = document.getElementById("liftplanName").value;
    if (liftplanName == "") {
        logLabel.innerHTML = "Error: Liftplan name cannot be empty";
        logLabel.style.color = errorColor;
        return;
    } else {
        logLabel.innerHTML = "";
    }
    const data = liftplan.toHexArray();
    fetch('/api/v1/liftplan?name=' + liftplanName + ".json", {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(data, null, 2)
    }).then(res => {
        if (!res.ok) console.log("Failed to send liftplan");
    });
}

function deleteLiftplan() {
    const select = document.getElementById('selectLiftplanPreview');
    console.log("Delete liftplan" + select.value);
    fetch('/api/v1/liftplan?name=' + select.value, {
        method: 'DELETE'
    }).then(res => {
        if (res.ok) handleLiftplanPreview();
        else console.log("Failed to send liftplan");
    });
}

function handleLiftplanPreview() {
    fetch('/api/v1/liftplan')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            const liftplanSelect = document.getElementById('selectLiftplanPreview');
            // clear select to prevent adding duplicates
            while (liftplanSelect.length > 0) {
                liftplanSelect.remove(0);
            }
            // populate select with liftplans
            data.forEach(optionText => {
                const option = document.createElement('option');
                option.value = optionText;
                option.textContent = optionText.replace(".json", "");
                liftplanSelect.appendChild(option);
            });
            if (data.length == 0) {
                // if there are no liftplan file hide the delete button
                document.getElementById("previewContainer").style.display = "none";
                document.getElementById("buttonDeleteLiftplan").style.display = "none";
            } else {
                // fetch the first liftplan
                document.getElementById("previewContainer").style.display = "flex";
                document.getElementById("buttonDeleteLiftplan").style.display = "block";
                getLiftplan(data[0], "liftplanPreviewTable");
            }
        })
        .catch(error => {
            console.error('There was a problem with the getting a list of liftplans:', error);
            document.getElementById("previewContainer").style.display = "none";
            document.getElementById("buttonDeleteLiftplan").style.display = "none";
        });
}

function openLiftplanTab(id, tabName) {
    if (tabName == "CreateNew") {
        document.getElementById("liftplanName").value = "";
        liftplanEditableTable.clearTable();
        liftplanEditableTable.addRow();
    } else if ("ManageExisting") {
        handleLiftplanPreview();
    }
    openTab(id, tabName);
}

function updatePreviewSelect() {
    const select = document.getElementById('selectLiftplanPreview');
    getLiftplan(select.value, "liftplanPreviewTable");
}

function handleLiftplanSelection() {
    fetch('/api/v1/liftplan')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            console.log("handleLiftplanSelection =  " + JSON.stringify(data));
            const liftplanSelect = document.getElementById('selectLiftplan');
            // clear select to prevent adding duplicates
            while (liftplanSelect.length > 0) {
                liftplanSelect.remove(0);
            }
            // populate select with liftplans
            data.forEach(optionText => {
                const option = document.createElement('option');
                option.value = optionText;
                option.textContent = optionText.replace(".json", "");
                liftplanSelect.appendChild(option);
            });
            if (data.length > 0) {
                var index = 0;
                // fetch the first liftplan
                document.getElementById("activeLiftplanContainer").style.display = "flex";
                if (loomInfo && loomInfo.loom_state != "idle") {
                    for (var i = 0; i < liftplanSelect.options.length; i++) {
                        if (liftplanSelect.options[i].value === loomInfo.active_liftplan) {
                            liftplanSelect.selectedIndex = i;
                            index = i;
                            break;
                        }
                    }
                }
                getLiftplan(data[index], "liftplanActiveTable");
            } else {
                document.getElementById("activeLiftplanContainer").style.display = "none";
            }
        })
        .catch(error => {
            console.error('There was a problem with the getting a list of liftplans:', error);
            document.getElementById("activeLiftplanContainer").style.display = "none";
        });
}

function updateLiftplanSelect() {
    const select = document.getElementById('selectLiftplan');
    getLiftplan(select.value, "liftplanActiveTable");
}

function handleLoomState(state) {
    document.getElementById("statusLabel").innerHTML = state.toUpperCase();
    if (state == "idle") {
        document.getElementById("startButton").style.display = "block";
        document.getElementById("pauseButton").style.display = "none";
        document.getElementById("continueButton").style.display = "none";
        document.getElementById("stopButton").style.display = "none";
        document.getElementById("selectLiftplan").disabled = false;
        if (loomIntervalId != 0) {
            clearInterval(loomIntervalId);
            loomIntervalId = 0;
        }
        enableTab("mainTab", "Liftplan");
        enableTab("mainTab", "Settings");
    } else if (state == "running") {
        document.getElementById("startButton").style.display = "none";
        document.getElementById("pauseButton").style.display = "block";
        document.getElementById("continueButton").style.display = "none";
        document.getElementById("stopButton").style.display = "block";
        document.getElementById("selectLiftplan").disabled = true;
        if (loomIntervalId == 0) {
            loomIntervalId = setInterval(getLoomLiftplanIndex, 1000);
        }
        disableTab("mainTab", "Liftplan");
        disableTab("mainTab", "Settings");
    } else if (state == "paused") {
        document.getElementById("startButton").style.display = "none";
        document.getElementById("pauseButton").style.display = "none";
        document.getElementById("continueButton").style.display = "block";
        document.getElementById("stopButton").style.display = "block";
        document.getElementById("selectLiftplan").disabled = true;
        if (loomIntervalId == 0) {
            loomIntervalId = setInterval(getLoomLiftplanIndex, 1000);
        }
        disableTab("mainTab", "Liftplan");
        disableTab("mainTab", "Settings");
    }
}

function getLoomState() {
    fetch('/api/v1/loom')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            console.log("Loom state =  " + JSON.stringify(data));
            loomInfo = data;
            handleLoomState(loomInfo.loom_state);

        })
        .catch(error => {
            console.error('There was a problem with the getting loom state:', error);
        });
}

function startLoom() {
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            'liftplan': document.getElementById('selectLiftplan').value,
            'start_position': 0
        })
    };
    console.log("JSON = " + JSON.stringify(requestOptions.body));
    fetch("/api/v1/loom/start", requestOptions)
        .then(response => response.json())
        .then(data => {
            console.log("Start loom response: " + JSON.stringify(data));
            if (data.status == true) {
                handleLoomState("running");
            }
        });
}

function pauseLoom() {
    const requestOptions = {
        method: 'POST',
    };
    console.log("JSON = " + JSON.stringify(requestOptions.body));
    fetch("/api/v1/loom/pause", requestOptions)
        .then(response => response.json())
        .then(data => {
            console.log("Pause loom response: " + JSON.stringify(data));
            handleLoomState("paused");
        });
}

function continueLoom() {
    const requestOptions = {
        method: 'POST',
    };
    console.log("JSON = " + JSON.stringify(requestOptions.body));
    fetch("/api/v1/loom/continue", requestOptions)
        .then(response => response.json())
        .then(data => {
            console.log("Continue loom response: " + JSON.stringify(data));
            handleLoomState("running");
        });
}

function stopLoom() {
    const requestOptions = {
        method: 'POST',
    };
    console.log("JSON = " + JSON.stringify(requestOptions.body));
    fetch("/api/v1/loom/stop", requestOptions)
        .then(response => response.json())
        .then(data => {
            console.log("Stop loom response: " + JSON.stringify(data));
            if (data.status == true) {
                handleLoomState("idle");
                // reset the liftplan table
                updateLiftplanSelect();
            }
        });
}

function getLoomLiftplanIndex() {
    fetch('/api/v1/loom/liftplan_index')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            console.log("Loom liftplan index =  " + JSON.stringify(data));
            liftplanActiveTable = new LiftPlan("liftplanActiveTable", true);
            liftplanActiveTable.highlightRow(data.index);
        })
        .catch(error => {
            console.error('There was a problem with the getting loom liftplan index:', error);
        });
}