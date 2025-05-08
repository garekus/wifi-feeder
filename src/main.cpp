/*
 curl -X POST http://<esp_ip>/wifi \
  -H "Content-Type: application/json" \
  -d '{"ssid":"my_ssid","pwd":"my_password"}'
 */

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>

#include <credentials.h>
#include <client_html.h>


#define PROBE_PIN     D7
#define MOTOR_PIN     D6


String current_ssid = my_default_ssid;
String current_password = my_default_password;

IPAddress staticIP(192, 168, 50, 201);
IPAddress gateway(192, 168, 50, 1);     // Default gateway (router) IP
IPAddress subnet(255, 255, 255, 0);     // Subnet mask
IPAddress dns1(8, 8, 8, 8);

ESP8266WebServer server(80);
const char* config_path = "/wifi.json";

// Add these global variables
const int UTC_OFFSET = 0; // UTC timezone (0 for UTC)
const int UTC_FEED_HOUR = 13; // 1PM UTC
const int UTC_FEED_MINUTE = 0;
bool motorRanToday = false;
time_t lastFeedTime = 0;

// Add this function to setup NTP
void setupTime(int maxTimeoutSecs = 10) {
  int retryDelay = 1000;
  configTime(UTC_OFFSET * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  
  // Wait for time to be set
  time_t now = time(nullptr);
  int retry = maxTimeoutSecs;
  while (now < 24 * 3600 && retry > 0) {
    delay(retryDelay);
    Serial.print(".");
    now = time(nullptr);
    retry--;
  }
  
  Serial.println();
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.println(asctime(&timeinfo));
}

void loadWifiConfig() {
  if (LittleFS.exists(config_path)) {
    File file = LittleFS.open(config_path, "r");
    if (file) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, file);
      if (!error) {
        current_ssid = doc["ssid"].as<String>();
        current_password = doc["pwd"].as<String>();
        Serial.println("Loaded Wi-Fi config from FS");
      }
      file.close();
    }
  }
}

void saveWifiConfig(const String& ssid, const String& pwd) {
  JsonDocument doc;
  doc["ssid"] = ssid;
  doc["pwd"] = pwd;

  File file = LittleFS.open(config_path, "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
    Serial.println("Wi-Fi config saved");
  }
}

wl_status_t wifiConnect(ESP8266WiFiClass &theWifi, String ssid, String pwd, int maxRetry = 20) {
  theWifi.config(staticIP, gateway, subnet, dns1);
  theWifi.begin(ssid.c_str(), pwd.c_str());
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < maxRetry) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  return WiFi.status();
}


void handleRoot() {
  server.send(200, "text/html", clientHtml);
}

void runMotor() {
  digitalWrite(MOTOR_PIN, HIGH);   // Turn motor on
  
  // Debounce variables
  const int debounceDelay = 50;    // Debounce time in milliseconds
  int probeState = LOW;            // Current state of the probe pin
  int lastProbeState = LOW;        // Previous state of the probe pin
  unsigned long lastDebounceTime = 0;  // Last time the probe pin was toggled
  
  // State tracking variables
  bool sawHigh = false;
  bool sawLow = false;
  
  // Wait for the probe pin to go HIGH->LOW->HIGH
  while (!(sawHigh && sawLow && probeState == HIGH)) {
    // Read the current state of the probe pin
    int reading = digitalRead(PROBE_PIN);
    
    // If the reading changed, reset the debounce timer
    if (reading != lastProbeState) {
      lastDebounceTime = millis();
    }
    
    // If the reading has been stable for longer than the debounce delay
    if ((millis() - lastDebounceTime) > debounceDelay) {
      // If the debounced state has changed
      if (reading != probeState) {
        probeState = reading;
        
        // Update our state tracking
        if (probeState == HIGH) {
          if (!sawHigh) {
            sawHigh = true;  // First HIGH
          } else if (sawLow) {
            // We've seen HIGH->LOW->HIGH, break out of the loop
            break;
          }
        } else if (probeState == LOW) {
          if (sawHigh) {
            sawLow = true;  // HIGH->LOW
          }
        }
      }
    }
    
    lastProbeState = reading;
    
    // Let other processes run
    yield();
  }
  
  // After detecting the cycle, turn off the motor
  digitalWrite(MOTOR_PIN, LOW);
}

void handleMotorRun() {
  digitalWrite(LED_BUILTIN, LOW);  // Turn LED on
  runMotor();
  digitalWrite(LED_BUILTIN, HIGH);  // Turn LED off
  
  server.send(200, "text/plain", "Motor cycle completed");
}


void handleWiFiPost() {
  JsonDocument response;

  // Step 1: Check body size
  if (!server.hasArg("plain") || server.arg("plain").length() > 400) {
    response["status"] = "error";
    response["message"] = "Request too large or missing body (max 400 chars)";
    String resBody;
    serializeJson(response, resBody);
    server.send(400, "application/json", resBody);
    return;
  }

  const String& body = server.arg("plain");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    response["status"] = "error";
    response["message"] = "Invalid JSON format";
    String resBody;
    serializeJson(response, resBody);
    server.send(400, "application/json", resBody);
    return;
  }

  String ssid = doc["ssid"] | "";
  String pwd = doc["pwd"] | "";

  if (ssid.isEmpty() || ssid.length() > 150) {
    response["status"] = "error";
    response["message"] = "SSID is missing or too long (max 150)";
    String resBody;
    serializeJson(response, resBody);
    server.send(400, "application/json", resBody);
    return;
  }

  if (pwd.isEmpty() || pwd.length() > 200) {
    response["status"] = "error";
    response["message"] = "Password is missing or too long (max 200)";
    String resBody;
    serializeJson(response, resBody);
    server.send(400, "application/json", resBody);
    return;
  }

  saveWifiConfig(ssid, pwd);
  response["status"] = "success";
  response["message"] = "Wi-Fi credentials saved. Rebooting...";
  String resBody;
  serializeJson(response, resBody);
  server.send(200, "application/json", resBody);

  delay(1000);
  ESP.restart();
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(PROBE_PIN, INPUT_PULLUP);
  digitalWrite(LED_BUILTIN, HIGH); // LED off
  digitalWrite(MOTOR_PIN, LOW); // Motor off

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    return;
  }

  // Attempt to load credentials from file system
  loadWifiConfig();

  Serial.printf("Trying Wi-Fi: %s\n", current_ssid.c_str());
  wl_status_t status = wifiConnect(WiFi, current_ssid, current_password);

  if (status != WL_CONNECTED) {
    Serial.println("\nStored Wi-Fi failed. Falling back to default credentials.");

    // Use hardcoded defaults
    current_ssid = my_default_ssid;
    current_password = my_default_password;

    status = wifiConnect(WiFi, current_ssid, current_password);
    if (status == WL_CONNECTED) {
      Serial.println("\nFallback Wi-Fi connected.");
    } else {
      Serial.println("\nFallback Wi-Fi also failed.");
    }
  } else {
    Serial.println("\nConnected with stored credentials.");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Start time sync");
    delay(1000);
    setupTime();
    Serial.println("Time sync complete");
  }


  server.on("/", handleRoot);
  server.on("/motorRun", handleMotorRun);
  server.on("/wifi", HTTP_POST, handleWiFiPost);
  server.begin();
  Serial.println("HTTP server started");
}


void loop() {
  server.handleClient();

  // Check if it's time to feed
  time_t now = time(nullptr);
  if (now > 0) { // Valid time received
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    
    // Check if it's feed time (1PM UTC) and we haven't fed today
    if (timeinfo.tm_hour == UTC_FEED_HOUR && 
        timeinfo.tm_min == UTC_FEED_MINUTE && 
        !motorRanToday) {
      
      Serial.println("It's feeding time!");
      runMotor();
      motorRanToday = true;
      lastFeedTime = now;
      
    } else if (timeinfo.tm_hour != UTC_FEED_HOUR || timeinfo.tm_min != UTC_FEED_MINUTE) {
      // Reset the flag when we're not in the feeding minute
      motorRanToday = false;
    }
  }
  
  // Allow for background processing
  yield();
}
