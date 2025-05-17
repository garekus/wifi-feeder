function feed() {
    document.getElementById('responseText').textContent = 'Sending request...';
    fetch('/feed')
        .then(response => response.text())
        .then(data => {
            document.getElementById('responseText').textContent = data;
        })
        .catch(error => {
            console.error('Error:', error);
            document.getElementById('responseText').textContent = 'Error: ' + error.message;
        });
}

document.getElementById('wifiForm').addEventListener('submit', function (e) {
    e.preventDefault();

    const responseElem = document.getElementById('wifiResponse');
    responseElem.textContent = 'Sending request...';
    responseElem.className = 'response-message';
    responseElem.style.display = 'block';

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
                responseElem.textContent = 'WiFi configuration updated successfully!';
                responseElem.className = 'response-message success';
            } else {
                responseElem.textContent = 'Error: ' + data.message;
                responseElem.className = 'response-message error';
            }
        })
        .catch(error => {
            responseElem.textContent = 'Error: ' + error.message;
            responseElem.className = 'response-message error';
        });
});