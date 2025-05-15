/*
 curl -X POST http://<esp_ip>/wifi \
  -H "Content-Type: application/json" \
  -d '{"ssid":"my_ssid","pwd":"my_password"}'
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>

#include <credentials.h>
#include <client_html.h>
#include "file_repository.h"
#include "wifi.h"
#include "schedule.h"
#include "logger.h"
#include "ntp_time.h"
#include "feeder.h"
#include "http_server.h"

ESP8266WebServer server(80);

Logger logger = Logger();
FileRepository fileRepo = FileRepository(logger);
WiFiConnection wifiConn = WiFiConnection(fileRepo, logger);
NtpTime ntpTime = NtpTime(fileRepo, logger);
Schedule schedule = Schedule(fileRepo, logger);
Feeder feeder = Feeder();
HttpServer httpServer = HttpServer(logger, wifiConn, ntpTime, feeder);

bool isInitialized = false;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED off

  logger.init();
  fileRepo.init();
  wifiConn.init();
  ntpTime.init();
  schedule.init();
  feeder.init();

  httpServer.init();

  isInitialized = true;
}

void loop()
{
  httpServer.processRequests();

  tm timeinfo = ntpTime.getTime();
  if (schedule.isSheduledTime(timeinfo.tm_hour, timeinfo.tm_min))
  {
    feeder.feed();
  }

  // Allow for background processing
  yield();
}
