#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <ESP8266WebServer.h>

#include "logger.h"
#include "wifi.h"
#include "schedule.h"
#include "feeder.h"
#include "ntp_time.h"

enum HttpServerError
{
    NO_ERROR,
    BODY_PARSE_ERROR
};

class HttpServer
{
private:
    Logger logger;
    WiFiConnection &wifiConn;
    NtpTime &ntpTime;
    Feeder &feeder;
    ESP8266WebServer *server;
    Schedule &schedule;

    void handleRoot();
    void handleFeed();
    void handlePostWiFi(const JsonDocument &body);
    void handleGetWiFiStatus();
    void handleGetTimeStatus();
    void handlePostTime(const JsonDocument &body);
    void handlePostSchedule(const JsonDocument &body);
    void handleGetSchedule();

    std::function<void(void)> createJsonHandler(int maxBodyLength, std::function<void(const JsonDocument &doc)> handler);

    void sendJson(int code, const JsonDocument &json);

public:
    HttpServer(Logger logger, WiFiConnection &wifiConn, NtpTime &ntpTime, Feeder &feeder, Schedule &schedule);
    void init();
    void processRequests();
};

#endif