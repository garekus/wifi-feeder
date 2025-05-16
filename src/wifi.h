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

struct WiFiErr
{
    enum Value : int
    {
        NO_ERROR,
        INIT_ERROR,
        EMPTY_JSON_ERROR,
        DESERIALIZE_ERROR,
        CONNECT_ERROR,
        CONFIG_SAVE_ERROR,
        FILE_CONFIG_ERROR
    };
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
};

class WiFiConnection
{
private:
    static const char *config_path;
    static const WiFiConfig defaultConf;
    WiFiConfig currentConf;
    FileRepository &fileRepo;
    Logger &logger;

    WiFiErr::Value preserveCurrentConfig();
    WiFiErr::Value connect();
    void loadConfigFromJsonDoc(const JsonDocument &doc);
    void loadDefaultConfig();
    WiFiErr::Value tryPreservedConfig();

public:
    WiFiConnection(FileRepository &fileRepo, Logger &logger) : fileRepo(fileRepo), logger(logger) {};
    WiFiErr::Value resetTo(const JsonDocument &doc);
    String &getStatusJson();
    WiFiErr::Value init();
};

#endif