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

#include <credentials.h>
#include <client_html.h>


#define BUTTON_PIN     D7
#define MOTOR_PIN     D6


String current_ssid = my_default_ssid;
String current_password = my_default_password;

ESP8266WebServer server(80);
const char* config_path = "/wifi.json";

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

void handleRoot() {
  server.send(200, "text/html", clientHtml);
}

void handleMotorOn() {
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(MOTOR_PIN, HIGH);
  server.send(200, "text/plain", "Motor is ON");
}

void handleMotorOff() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(MOTOR_PIN, LOW);
  server.send(200, "text/plain", "Motor is OFF");
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
  digitalWrite(LED_BUILTIN, HIGH); // LED off
  digitalWrite(MOTOR_PIN, LOW); // Motor off

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    return;
  }

  // Attempt to load credentials from file system
  loadWifiConfig();

  Serial.printf("Trying Wi-Fi: %s\n", current_ssid.c_str());
  WiFi.begin(current_ssid.c_str(), current_password.c_str());

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nStored Wi-Fi failed. Falling back to default credentials.");

    // Use hardcoded defaults
    current_ssid = my_default_ssid;
    current_password = my_default_password;

    WiFi.begin(current_ssid.c_str(), current_password.c_str());

    retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
      delay(500);
      Serial.print(".");
      retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nFallback Wi-Fi connected.");
    } else {
      Serial.println("\nFallback Wi-Fi also failed.");
    }
  } else {
    Serial.println("\nConnected with stored credentials.");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/motorOn", handleMotorOn);
  server.on("/motorOff", handleMotorOff);
  server.on("/wifi", HTTP_POST, handleWiFiPost);
  server.begin();
  Serial.println("HTTP server started");
}


void loop() {
  server.handleClient();
}
