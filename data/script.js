blocks = ['wifiConfigContainer', 'wifiStatusContainer'];

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