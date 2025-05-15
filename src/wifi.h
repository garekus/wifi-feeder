#ifndef WI_FI_H
#define WI_FI_H

#include <ESP8266WiFi.h>
#include "file_repository.h"
#include "logger.h"

/*
config json:
{
    "ssid": "my_ssid",
    "pwd": "my_password",
    "staticIp": "192.168.1.201", // optional
    "gateway": "192.168.1.1",
    "subnet": "255.255.255.0",
    "dns1": "192.168.1.1"
}
*/

enum WiFiError
{
    NO_ERROR,
    INIT_ERROR,
    EMPTY_JSON_ERROR,
    DESERIALIZE_ERROR,
    CONNECT_ERROR,
    CONFIG_SAVE_ERROR,
    FILE_CONFIG_ERROR
};

struct IpConfig
{
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns1;
};

struct WiFiConfig
{
    String ssid;
    String pwd;
    IpConfig staticIpConfig;
};

class WiFiConnection
{
private:
    static const char *config_path;
    static const WiFiConfig defaultConf;
    WiFiConfig currentConf;
    FileRepository &fileRepo;
    Logger &logger;

    WiFiError preserveCurrentConfig();
    WiFiError connect();
    void loadConfigFromJsonDoc(const JsonDocument &doc);
    void loadDefaultConfig();
    WiFiError tryPreservedConfig();

public:
    WiFiConnection(FileRepository &fileRepo, Logger &logger) : fileRepo(fileRepo), logger(logger) {};
    WiFiError resetTo(const JsonDocument &doc);
    String &getStatusJson();
    WiFiError init();
};

#endif