#include "ntp_time.h"

#define TIMEZONE_KEY "timezone"

bool isTimeValid(time_t t)
{
    return t > 86400; // 1 day in seconds
}

NtpTime::NtpTime(FileRepo &fileRepo, Logger &logger) : fileRepo(fileRepo), logger(logger) {};

void NtpTime::init()
{
    NtpTimeErr::Value err = loadPreservedConfig();
    if (err != NtpTimeErr::NO_ERROR)
    {
        logger.println("Failed to load preserved time config, fallback to default");
    }
    err = tryToSyncTimeZone(currentTZ);
    if (err != NtpTimeErr::NO_ERROR)
    {
        logger.print("Failed to sync preserved time, tz: " + currentTZ + ", fallback to default");
    }

    loadDefaultConfig();
    err = tryToSyncTimeZone(currentTZ);
    if (err != NtpTimeErr::NO_ERROR)
    {
        logger.print("Failed to sync default time, tz: " + currentTZ);
    }
}

void NtpTime::loadDefaultConfig()
{
    currentTZ = defaultTZ;
}

void NtpTime::preserveConfig()
{
    JsonDocument doc;
    doc[TIMEZONE_KEY] = currentTZ;
    fileRepo.writeJsonFile(configPath, doc);
}
NtpTimeErr::Value NtpTime::loadPreservedConfig()
{
    JsonDocument doc;
    FileRepoErr::Value res = fileRepo.readJsonFile(configPath, doc);
    if (res != FileRepoErr::NO_ERROR)
    {
        logger.print("Failed to read time config: ");
        logger.println(res);
        return NtpTimeErr::CONFIG_READ_ERROR;
    }

    if (doc.isNull())
    {
        return NtpTimeErr::CONFIG_READ_ERROR;
    }
    currentTZ = doc[TIMEZONE_KEY].as<String>();
    return NtpTimeErr::NO_ERROR;
}

NtpTimeErr::Value NtpTime::setTimeZone(String tz, int maxTimeoutSecs)
{
    NtpTimeErr::Value err = tryToSyncTimeZone(tz, maxTimeoutSecs);
    if (err != NtpTimeErr::NO_ERROR)
    {
        return err;
    }

    currentTZ = tz;
    preserveConfig();
    return NtpTimeErr::NO_ERROR;
}

tm NtpTime::getTime()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    return timeinfo;
}

String NtpTime::getTimeStatusJson()
{
    struct tm timeinfo = getTime();
    JsonDocument doc;
    doc["hour"] = timeinfo.tm_hour;
    doc["minute"] = timeinfo.tm_min;
    doc["second"] = timeinfo.tm_sec;
    doc["timezone"] = currentTZ;
    String json;
    serializeJson(doc, json);
    return json;
}

NtpTimeErr::Value NtpTime::tryToSyncTimeZone(String tz, int maxTimeoutSecs)
{
    int retryDelay = 1000;
    configTzTime(tz.c_str(), "pool.ntp.org", "time.nist.gov");
    logger.println("Waiting for NTP time sync...");

    // Wait for time to be set
    time_t now = time(nullptr);
    int retry = maxTimeoutSecs;
    while (!isTimeValid(now) && retry > 0)
    {
        delay(retryDelay);
        logger.print(".");
        now = time(nullptr);
        retry--;
    }
    logger.println();

    if (!isTimeValid(now))
    {
        logger.println("NTP time sync failed");
        return NtpTimeErr::TIME_SYNC_ERROR;
    }

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    logger.print("Current time: ");
    logger.println(asctime(&timeinfo));

    return NtpTimeErr::NO_ERROR;
}
