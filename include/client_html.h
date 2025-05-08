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
    <h1>ESP8266 Motor Control</h1>
    <div class="btn-container">
        <button class="btn" onclick="turnMotorRun()">Run</button>
    </div>
    
    <div class="response-container">
        <p>Status: <span id="responseText">Ready</span></p>
    </div>

    <script>
        function turnMotorOn() {
            document.getElementById('responseText').textContent = 'Sending request...';
            fetch('/motorRun')
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
</body>
</html>
)"""";
