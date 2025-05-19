/*
 curl -X POST http://<esp_ip>/wifi \
  -H "Content-Type: application/json" \
  -d '{"ssid":"my_ssid","pwd":"my_password"}'
 */
#include <WString.h>

#include <credentials.h>
#include "file_repository.h"
#include "wifi.h"
#include "schedule.h"
#include "logger.h"
#include "ntp_time.h"
#include "feeder.h"
#include "http_server.h"

ESP8266WebServer server(80);

Logger logger = Logger();
FileRepo fileRepo = FileRepo(logger);
WiFiConnection wifiConn = WiFiConnection(fileRepo, logger);
NtpTime ntpTime = NtpTime(fileRepo, logger);
Schedule schedule = Schedule(fileRepo, logger);
Feeder feeder = Feeder();
HttpServer httpServer = HttpServer(logger, fileRepo, wifiConn, ntpTime, feeder, schedule);

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
}

void loop()
{
  httpServer.processRequests();

  tm timeinfo = ntpTime.getTime();
  if (schedule.isFeedingTime(timeinfo.tm_hour, timeinfo.tm_min))
  {
    feeder.feed();
  }

  ntpTime.syncTimeLoop();

  // Allow for background processing
  yield();
}
