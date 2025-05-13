#include "time_repository.h"

#define TIMEZONE_KEY "timezone"

TimeRepository::TimeRepository(FileRepository &fileRepo, Logger &logger) : fileRepo(fileRepo), logger(logger) {};

void TimeRepository::init()
{
    TimeRepositoryError err = loadPreservedConfig();
    if (err != TimeRepositoryError::NO_ERROR)
    {
        logger.println("Failed to load preserved time config, fallback to default");
    }
    err = tryToSyncTimeZone(currentTZ);
    if (err != TimeRepositoryError::NO_ERROR)
    {
        logger.print("Failed to sync preserved time, tz: " + currentTZ + ", fallback to default");
    }

    loadDefaultConfig();
    err = tryToSyncTimeZone(currentTZ);
    if (err != TimeRepositoryError::NO_ERROR)
    {
        logger.print("Failed to sync default time, tz: " + currentTZ);
    }
}

void TimeRepository::loadDefaultConfig()
{
    currentTZ = defaultTZ;
}

void TimeRepository::preserveConfig()
{
    JsonDocument doc;
    doc[TIMEZONE_KEY] = currentTZ;
    fileRepo.writeJsonFile(configPath, doc);
}
TimeRepositoryError TimeRepository::loadPreservedConfig()
{
    FileResult<JsonDocument &> res = fileRepo.readJsonFile(configPath);
    if (!res.isSuccess())
    {
        logger.print("Failed to read time config");
        logger.println(res.error());
        return TimeRepositoryError::CONFIG_READ_ERROR;
    }

    JsonDocument doc = res.value();
    if (doc.isNull())
    {
        return TimeRepositoryError::CONFIG_READ_ERROR;
    }
    currentTZ = doc[TIMEZONE_KEY].as<String>();
    return TimeRepositoryError::NO_ERROR;
}

TimeRepositoryError TimeRepository::tryToSyncTimeZone(String tz, int maxTimeoutSecs)
{
    int retryDelay = 1000;
    configTzTime(tz.c_str(), "pool.ntp.org", "time.nist.gov");
    Serial.println("Waiting for NTP time sync...");

    // Wait for time to be set
    time_t now = time(nullptr);
    int retry = maxTimeoutSecs;
    while (!isTimeValid(now) && retry > 0)
    {
        delay(retryDelay);
        Serial.print(".");
        now = time(nullptr);
        retry--;
    }
    Serial.println();

    if (!isTimeValid(now))
    {
        Serial.println("NTP time sync failed");
        return TimeRepositoryError::TIME_SYNC_ERROR;
    }

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.println(asctime(&timeinfo));
}

TimeRepositoryError TimeRepository::setTimeZone(String tz, int maxTimeoutSecs = 10)
{
    TimeRepositoryError err = tryToSyncTimeZone(tz, maxTimeoutSecs);
    if (err != TimeRepositoryError::NO_ERROR)
    {
        return err;
    }

    currentTZ = tz;
    preserveConfig();
    return TimeRepositoryError::NO_ERROR;
}

tm TimeRepository::getTime()
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    return timeinfo;
}

String TimeRepository::getTimeStatusJson()
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

bool isTimeValid(time_t t)
{
    return t > 86400; // 1 day in seconds
}