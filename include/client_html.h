const char *clientHtml = R""""(
    <!DOCTYPE html>
<html>
<head>
    <title>ESP8266 Motor Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 20px;
        }
        h1 {
            color: #333;
        }
        .btn-container {
            margin-top: 30px;
        }
        .btn {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 15px 32px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin: 10px;
            cursor: pointer;
            border-radius: 8px;
        }
        .btn-off {
            background-color: #f44336;
        }
        .response-container {
            margin-top: 20px;
            padding: 15px;
            border: 1px solid #ddd;
            border-radius: 8px;
            background-color: #f9f9f9;
            max-width: 400px;
            margin-left: auto;
            margin-right: auto;
        }
        #responseText {
            font-weight: bold;
            color: #555;
        }
    </style>
</head>
<body>
    <h1>ESP8266 Feeder</h1>
    <div class="btn-container">
        <button class="btn" onclick="feed()">Feed</button>
    </div>
    
    <div class="response-container">
        <p>Status: <span id="responseText">Ready</span></p>
    </div>

    <script>
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
    </script>

    <!-- wifi config form -->
    <div class="form-container">
  <h2>WiFi Configuration</h2>
  <form id="wifiForm">
    <div class="form-group">
      <label for="ssid">SSID:</label>
      <input type="text" id="ssid" name="ssid" required>
    </div>
    <div class="form-group">
      <label for="pwd">Password:</label>
      <input type="password" id="pwd" name="pwd" required>
    </div>
    <h3>Static IP (Optional)</h3>
    <div class="form-group">
      <label for="staticIp">IP Address:</label>
      <input type="text" id="staticIp" name="staticIp" placeholder="192.168.1.201">
    </div>
    <div class="form-group">
      <label for="gateway">Gateway:</label>
      <input type="text" id="gateway" name="gateway" placeholder="192.168.1.1">
    </div>
    <div class="form-group">
      <label for="subnet">Subnet Mask:</label>
      <input type="text" id="subnet" name="subnet" placeholder="255.255.255.0">
    </div>
    <div class="form-group">
      <label for="dns1">DNS:</label>
      <input type="text" id="dns1" name="dns1" placeholder="192.168.1.1">
    </div>
    <button type="submit" class="btn">Save WiFi Configuration</button>
  </form>
  <div id="wifiResponse" class="response-message"></div>
</div>

<style>
  .form-container {
    max-width: 500px;
    margin: 30px auto;
    padding: 20px;
    border: 1px solid #ddd;
    border-radius: 8px;
    background-color: #f9f9f9;
  }
  .form-group {
    margin-bottom: 15px;
    text-align: left;
  }
  .form-group label {
    display: block;
    margin-bottom: 5px;
    font-weight: bold;
  }
  .form-group input {
    width: 100%;
    padding: 8px;
    border: 1px solid #ddd;
    border-radius: 4px;
    box-sizing: border-box;
  }
  .response-message {
    margin-top: 15px;
    padding: 10px;
    border-radius: 4px;
    display: none;
  }
  .success {
    display: block;
    background-color: #dff0d8;
    color: #3c763d;
  }
  .error {
    display: block;
    background-color: #f2dede;
    color: #a94442;
  }
</style>

<script>
  document.getElementById('wifiForm').addEventListener('submit', function(e) {
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
</script>

</body>
</html>
)"""";
