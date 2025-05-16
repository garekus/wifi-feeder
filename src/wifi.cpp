#include "wifi.h"
#include "credentials.h"

#define CONNECT_MAX_RETRIES 20

const char *WiFiConnection::config_path = "/wifi.json";
const WiFiConfig WiFiConnection::defaultConf = {
    ssid : default_ssid,
    pwd : default_password,
};

void WiFiConnection::loadDefaultConfig()
{
    currentConf.ssid = defaultConf.ssid;
    currentConf.pwd = defaultConf.pwd;
}

WiFiErr::Value WiFiConnection::init()
{
    logger.println("Trying preserved config...");
    WiFiErr::Value res = tryPreservedConfig();
    if (res == WiFiErr::NO_ERROR)
        return res;

    logger.println("Falling back to hardcoded credentials...");
    loadDefaultConfig();
    return connect();
}

WiFiErr::Value WiFiConnection::tryPreservedConfig()
{
    JsonDocument doc;
    FileRepoErr::Value res = fileRepo.readJsonFile(config_path, doc);
    if (res != FileRepoErr::NO_ERROR)
    {
        logger.print("Failed to read config: ");
        logger.println(res);
        return WiFiErr::FILE_CONFIG_ERROR;
    }

    loadConfigFromJsonDoc(doc);
    return connect();
}

WiFiErr::Value WiFiConnection::preserveCurrentConfig()
{
    String json;
    JsonDocument doc;

    doc["ssid"] = currentConf.ssid;
    doc["pwd"] = currentConf.pwd;

    serializeJson(doc, json);
    FileRepoErr::Value writeRes = fileRepo.writeJsonFile(config_path, doc);
    if (writeRes != FileRepoErr::NO_ERROR)
    {
        logger.println("Failed to save config");
        return WiFiErr::CONFIG_SAVE_ERROR;
    }

    return WiFiErr::NO_ERROR;
}

void WiFiConnection::loadConfigFromJsonDoc(const JsonDocument &doc)
{
    currentConf.ssid = doc["ssid"].as<String>();
    currentConf.pwd = doc["pwd"].as<String>();
}

WiFiErr::Value WiFiConnection::connect()
{
    logger.println("Trying Wi-Fi: ");
    logger.println(currentConf.ssid.c_str());

    // if (currentConf.staticIpConfig.ip.isSet())
    // {
    //     WiFi.config(currentConf.staticIpConfig.ip, currentConf.staticIpConfig.gateway, currentConf.staticIpConfig.subnet, currentConf.staticIpConfig.dns1);
    // }
    WiFi.begin(currentConf.ssid.c_str(), currentConf.pwd.c_str());
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < CONNECT_MAX_RETRIES)
    {
        delay(1000);
        logger.print(".");
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        return WiFiErr::NO_ERROR;
    }

    logger.print("Connection failed with status");
    logger.println(WiFi.status());

    return WiFiErr::CONNECT_ERROR;
}

WiFiErr::Value WiFiConnection::resetTo(const JsonDocument &doc)
{
    loadConfigFromJsonDoc(doc);

    WiFiErr::Value err = connect();
    if (err != WiFiErr::NO_ERROR)
    {
        return err;
    }
    return preserveCurrentConfig();
}

String &WiFiConnection::getStatusJson()
{
    String json;
    JsonDocument doc;
    doc["ssid"] = WiFi.SSID();
    doc["configured ssid"] = currentConf.ssid;
    doc["status"] = WiFi.status();
    doc["rssi"] = WiFi.RSSI();
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();

    serializeJson(doc, json);
    return json;
}
