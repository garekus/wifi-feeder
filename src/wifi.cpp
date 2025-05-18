#include "wifi.h"
#include "credentials.h"

#define CONNECT_MAX_RETRIES 40

const char *WiFiConnection::config_path = "/wifi.json";
const WiFiConfig WiFiConnection::defaultConf = {
    ssid : default_ssid,
    pwd : default_password,
    ipConf : {// static ip is required due to issue with dhcp on esp8266
              ip : IPAddress(192, 168, 50, 201),
              gateway : IPAddress(192, 168, 50, 1),
              subnet : IPAddress(255, 255, 255, 0),
              dns1 : IPAddress(192, 168, 50, 1)
    }
};

void WiFiConnection::loadDefaultConfig()
{
    currentConf.ssid = defaultConf.ssid;
    currentConf.pwd = defaultConf.pwd;
    currentConf.ipConf = defaultConf.ipConf;
}

WiFiErr::Value WiFiConnection::init()
{
    WiFi.mode(WIFI_STA);

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
    doc["staticIp"] = currentConf.ipConf.ip.toString();
    doc["gateway"] = currentConf.ipConf.gateway.toString();
    doc["subnet"] = currentConf.ipConf.subnet.toString();
    doc["dns1"] = currentConf.ipConf.dns1.toString();

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
    IPAddress staticIp = IPAddress();
    IPAddress gateway = IPAddress();
    IPAddress subnet = IPAddress();
    IPAddress dns1 = IPAddress();

    staticIp.fromString(doc["staticIp"].as<String>());
    gateway.fromString(doc["gateway"].as<String>());
    subnet.fromString(doc["subnet"].as<String>());
    dns1.fromString(doc["dns1"].as<String>());

    currentConf.ssid = doc["ssid"].as<String>();
    currentConf.pwd = doc["pwd"].as<String>();
    currentConf.ipConf.ip = staticIp;
    currentConf.ipConf.gateway = gateway;
    currentConf.ipConf.subnet = subnet;
    currentConf.ipConf.dns1 = dns1;
}

WiFiErr::Value WiFiConnection::connect()
{
    logger.println("Trying Wi-Fi: ");
    logger.println(currentConf.ssid.c_str());

    if (currentConf.ipConf.ip.isSet())
    {
        logger.print("Using static IP: ");
        logger.println(currentConf.ipConf.ip.toString());
        WiFi.config(currentConf.ipConf.ip, currentConf.ipConf.gateway, currentConf.ipConf.subnet, currentConf.ipConf.dns1);
    }
    else
    {
        logger.println("Using DHCP");
    }
    WiFi.begin(currentConf.ssid.c_str(), currentConf.pwd.c_str());
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < CONNECT_MAX_RETRIES)
    {
        delay(500);
        logger.print(".");
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        logger.println();
        logger.print("Connected to ");
        logger.print(WiFi.SSID());
        return WiFiErr::NO_ERROR;
    }

    logger.print("Connection failed with status: ");
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

void WiFiConnection::getStatusJson(String &dst)
{
    JsonDocument doc;
    doc["ssid"] = WiFi.SSID();
    doc["configured ssid"] = currentConf.ssid;
    doc["status"] = WiFi.status();
    doc["rssi"] = WiFi.RSSI();
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();

    serializeJson(doc, dst);
}
