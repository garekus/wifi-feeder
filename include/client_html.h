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
        </style>
    </head>
    <body>
        <h1>ESP8266 Motor Control</h1>
        <div class="btn-container">
            <button class="btn" onclick="turnMotorOn()">Turn Motor ON</button>
            <button class="btn btn-off" onclick="turnMotorOff()">Turn Motor OFF</button>
        </div>
    
        <script>
            function turnMotorOn() {
                fetch('/motorOn')
                    .then(response => response.text())
                    .then(data => alert(data))
                    .catch(error => console.error('Error:', error));
            }
    
            function turnMotorOff() {
                fetch('/motorOff')
                    .then(response => response.text())
                    .then(data => alert(data))
                    .catch(error => console.error('Error:', error));
            }
        </script>
    </body>
    </html>
)"""";
