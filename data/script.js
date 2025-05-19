blocks = ['wifiContainer', 'scheduleContainer'];

currentBlock = '';

function feed() {
    setStatus('Feeding...');
    fetch('/feed')
        .then(response => response.text())
        .then(data => {
            setStatus("Feed completed", "success");
        })
        .catch(error => {
            console.error('Error:', error);
            setStatus('Error: ' + error.message, "error");
        });
}

function toggleBlock(elId, fetchDataCallback) {
    const container = document.getElementById(elId);
    if (container.style.display === 'none' || container.style.display === '') {
        if (fetchDataCallback) {
            fetchDataCallback();
        }
        container.style.display = 'block';
        closeAllBlocks(elId);
    } else {
        container.style.display = 'none';
    }
}
function closeAllBlocks(except) {
    currentBlock = '';
    blocks.filter(elId => elId !== except).forEach(block => {
        document.getElementById(block).style.display = 'none';
    });
}

document.getElementById('wifiForm').addEventListener('submit', function (e) {
    e.preventDefault();

    const formData = {
        ssid: document.getElementById('ssid').value,
        pwd: document.getElementById('pwd').value,
        staticIp: document.getElementById('staticIp').value,
        gateway: document.getElementById('gateway').value,
        subnet: document.getElementById('subnet').value,
        dns1: document.getElementById('dns1').value
    };

    fetch('/wifi', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(formData)
    })
        .then(response => response.json())
        .then(data => {
            if (data.status === 'success') {
                setStatus('WiFi configuration updated successfully!', 'success');
            } else {
                setStatus('WiFi configuration error: ' + data.message, 'error');
            }
        })
        .catch(error => {
            setStatus('WiFi configuration error: ' + data.message, 'error');
        });
});

function refreshTimeStatus() {
    setStatus("Fetching time...");
    fetch('/time')
        .then(response => response.json())
        .then(data => {
            document.getElementById('currentTime').textContent = data.hour + ":" + data.minute + ":" + data.second;
            document.getElementById('currentTZ').textContent = data.timezone;
            setStatus("Time fetched successfully");
        })
        .catch(error => {
            document.getElementById('currentTime').textContent = "-";
            document.getElementById('currentTZ').textContent = "-";
            console.error('Error fetching time:', error);
            setStatus("Error fetching time: " + error.message);
        });
}

document.getElementById('timeForm').addEventListener('submit', function (e) {
    e.preventDefault();

    const formData = {
        timezone: document.getElementById('timezone').value
    };

    fetch('/time', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(formData)
    })
        .then(response => response.json())
        .then(data => {
            if (data.status === 'success') {
                setStatus('Time configuration updated successfully!', 'success');
            } else {
                setStatus('Time configuration error: ' + data.message, 'error');
            }
        })
        .catch(error => {
            setStatus('Time configuration error: ' + error.message, 'error');
        });
})

document.getElementById('scheduleForm').addEventListener('submit', function (e) {
    e.preventDefault();

    const formData = [];
    // Create 5 time entries from form data
    for (let i = 0; i < 5; i++) {
        formData.push({
            hour: parseInt(document.getElementsByName(`hour_${i}`)[0].value),
            minute: parseInt(document.getElementsByName(`minute_${i}`)[0].value)
        });
    }

    fetch('/schedule', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(formData)
    })
        .then(response => response.json())
        .then(data => {
            if (data.status === 'success') {
                setStatus('Schedule updated successfully!', 'success');
            } else {
                setStatus('Schedule update error: ' + data.message, 'error');
            }
        })
        .catch(error => {
            setStatus('Schedule update error: ' + error.message, 'error');
        });
});

// possible states: "success", "error" and "info"
function setStatus(msg, state = "info") {
    document.getElementById('responseText').textContent = msg;
    switch (state) {
        case "success":
            document.getElementById('responseText').className = 'response-message success';
            break;
        case "error":
            document.getElementById('responseText').className = 'response-message error';
            break;
        case "info":
            document.getElementById('responseText').className = 'response-message';
            break;
    }
}

async function refreshWifiStatus() {
    setStatus("Fetching WiFi status...");
    await fetch('/wifi')
        .then(response => response.json())
        .then(data => {
            document.getElementById('currentSsid').textContent = data.ssid;
            document.getElementById('configuredSsid').textContent = data["configured ssid"];
            document.getElementById('wifiStatus').textContent = data.status;
            document.getElementById('wifiRssi').textContent = data.rssi + " dBm";
            document.getElementById('wifiIp').textContent = data.ip;
            document.getElementById('wifiMac').textContent = data.mac;
            setStatus("Fetched WiFi status");
        })
        .catch(error => {
            console.error('Error fetching WiFi status:', error);
            setStatus("Error fetching WiFi status" + error.message);
        });
}

function loadSchedule() {
    setStatus('Loading schedule...');
    fetch('/schedule')
        .then(response => response.json())
        .then(data => {
            const container = document.getElementById('scheduleEntries');
            container.innerHTML = ''; // Clear existing entries

            // Add each time slot
            data.forEach((time, index) => {
                const entry = document.createElement('div');
                entry.className = 'form-group schedule-entry';
                entry.innerHTML = `
                    <label>Feeding Time ${index + 1}:</label>
                    <div class="time-inputs">
                        <input type="number" min="0" max="23" name="hour_${index}" value="${time.hour}" required> :
                        <input type="number" min="0" max="59" name="minute_${index}" value="${time.minute}" required>
                    </div>
                `;
                container.appendChild(entry);
            });

            setStatus('Schedule loaded', 'success');
        })
        .catch(error => {
            console.error('Error:', error);
            setStatus('Error loading schedule: ' + error.message, 'error');
        });
}
