
#include "http_server.h"

HttpServer::HttpServer(Logger logger, FileRepo &fileRepo, WiFiConnection &wifiConn, NtpTime &ntpTime, Feeder &feeder, Schedule &schedule)
    : logger(logger), fileRepo(fileRepo), wifiConn(wifiConn), ntpTime(ntpTime), feeder(feeder), schedule(schedule)
{
    server = new ESP8266WebServer(80);
}

void HttpServer::init()
{
    server->on("/", std::bind(&HttpServer::handleRoot, this));
    server->on("/styles.css", HTTP_GET, std::bind(&HttpServer::handleCss, this));
    server->on("/script.js", HTTP_GET, std::bind(&HttpServer::handleJs, this));

    server->on("/feed", std::bind(&HttpServer::handleFeed, this));
    server->on("/wifi", HTTP_POST, createJsonHandler(400, [this](const JsonDocument &body)
                                                     { this->handlePostWiFi(body); }));
    server->on("/time", HTTP_POST, createJsonHandler(400, [this](const JsonDocument &body)
                                                     { this->handlePostTime(body); }));
    server->on("/schedule", HTTP_POST, createJsonHandler(400, [this](const JsonDocument &body)
                                                         { this->handlePostSchedule(body); }));
    server->on("/wifi", HTTP_GET, std::bind(&HttpServer::handleGetWiFiStatus, this));
    server->on("/time", HTTP_GET, std::bind(&HttpServer::handleGetTimeStatus, this));
    server->on("/schedule", HTTP_GET, std::bind(&HttpServer::handleGetSchedule, this));

    server->begin();
    logger.println("HTTP server started");
}

void HttpServer::processRequests()
{
    server->handleClient();
}

void HttpServer::handleRoot()
{
    File file = fileRepo.openForRead("/index.html");
    if (!file)
    {
        server->send(500, "text/plain", "File Not Found");
        return;
    }
    server->send(200, "text/html", file);
}

void HttpServer::handleCss()
{
    File file = fileRepo.openForRead("/styles.css");
    if (!file)
    {
        server->send(500, "text/plain", "File Not Found");
        return;
    }
    server->send(200, "text/css", file);
}

void HttpServer::handleJs()
{
    File file = fileRepo.openForRead("/script.js");
    if (!file)
    {
        server->send(500, "text/plain", "File Not Found");
        return;
    }
    server->send(200, "text/javascript", file);
}

void HttpServer::handleFeed()
{
    digitalWrite(LED_BUILTIN, LOW); // Turn LED on
    feeder.feed();
    digitalWrite(LED_BUILTIN, HIGH); // Turn LED off
    server->send(200, "text/plain", "Feeding completed");
}

/**
 * @brief Handle POST request on /wifi endpoint
 *
 * Resets the device's WiFi configuration to the provided values.
 *
 * The request body should contain a JSON document with the following structure:
 * {
 *     ssid: string,
 *     pwd: string,
 * }
 *
 * @param body The parsed JSON document from the request body
 */
void HttpServer::handlePostWiFi(const JsonDocument &body)
{
    JsonDocument response;

    String ssid = body["ssid"] | "";
    String pwd = body["pwd"] | "";

    if (ssid.isEmpty() || ssid.length() > 150)
    {
        response["status"] = "error";
        response["message"] = "SSID is missing or too long (max 150)";
        sendJson(400, response);
        return;
    }

    if (pwd.isEmpty() || pwd.length() > 200)
    {
        response["status"] = "error";
        response["message"] = "Password is missing or too long (max 200)";
        sendJson(400, response);
        return;
    }

    WiFiErr::Value err = wifiConn.resetTo(body);
    if (err != WiFiErr::NO_ERROR)
    {
        response["status"] = "error";
        response["message"] = "Failed to connect to wifi";
        response["error"] = err;
        sendJson(500, response);
        return;
    }
    response["status"] = "success";
    response["message"] = "New wifi config applied";
    sendJson(200, response);
}

void HttpServer::handleGetWiFiStatus()
{
    String json;
    wifiConn.getStatusJson(json);
    server->send(200, "application/json", json);
}
void HttpServer::handleGetTimeStatus()
{
    String json = ntpTime.getTimeStatusJson();
    server->send(200, "application/json", json);
}

void HttpServer::handleGetSchedule()
{
    String json = schedule.getScheduleJson();
    server->send(200, "application/json", json);
}

void HttpServer::handlePostSchedule(const JsonDocument &body)
{
    JsonDocument response;

    ScheduleErr::Value err = schedule.setSchedule(body);
    if (err != ScheduleErr::NO_ERROR)
    {
        response["status"] = "error";
        response["message"] = "Failed to set schedule";
        response["error"] = err;
        sendJson(500, response);
        return;
    }

    response["status"] = "success";
    response["message"] = "New schedule applied";
    sendJson(200, response);
}

void HttpServer::handlePostTime(const JsonDocument &body)
{
    JsonDocument response;

    String tz = body["timezone"] | "";
    if (tz.isEmpty() || tz.length() > 50)
    {
        response["status"] = "error";
        response["message"] = "Timezone is invalid";
        sendJson(400, response);
        return;
    }

    NtpTimeErr::Value err = ntpTime.setTimeZone(tz);
    if (err != NtpTimeErr::NO_ERROR)
    {
        response["status"] = "error";
        response["message"] = "Failed to set time";
        response["error"] = err;
        sendJson(500, response);
        return;
    }
    response["status"] = "success";
    response["message"] = "New time config applied";
    sendJson(200, response);
}

void responseWithJson(ESP8266WebServer *server, int code, const JsonDocument &json)
{
    String resBody;
    serializeJson(json, resBody);
    server->send(code, "application/json", resBody);
}

void HttpServer::sendJson(int code, const JsonDocument &json)
{
    responseWithJson(this->server, code, json);
}

std::function<void(void)> HttpServer::createJsonHandler(unsigned int maxBodyLength, std::function<void(const JsonDocument &doc)> handler)
{
    ESP8266WebServer *serv = this->server;
    return [&serv, maxBodyLength, &handler](void)
    {
        JsonDocument response;

        // Check body size
        if (!serv->hasArg("plain") ||
            serv->arg("plain").length() > maxBodyLength)
        {
            response["status"] = "error";
            response["message"] = "Request too large or missing body (max " +
                                  String(maxBodyLength) + " chars)";
            responseWithJson(serv, 400, response);
            return;
        }

        // Parse JSON
        const String &body = serv->arg("plain");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);

        if (error)
        {
            response["status"] = "error";
            response["message"] = "Invalid JSON format";
            response["error"] = error.c_str();
            responseWithJson(serv, 400, response);
            return;
        }

        // Call the handler with the parsed document
        handler(doc);
    };
};
