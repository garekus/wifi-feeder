#include <LittleFS.h>
#include <ArduinoJson.h>

#include "schedule.h"

Schedule::Schedule(FileRepository &fileRepo, Logger &logger) : fileRepo(fileRepo), logger(logger)
{
    isSet = false;
}

bool Schedule::isSheduledTime(int hour, int minute)
{
    if (!isSet)
        return false;

    for (int i = 0; i < 6; i++)
    {
        if (timesList[i].hour == hour && timesList[i].minute == minute)
        {
            return true;
        }
    }
    return false;
}

const String Schedule::filePath = "/schedule.json";

ScheduleError Schedule::init()
{
    FileResult<JsonDocument &> res = fileRepo.readJsonFile(filePath);
    if (!res.isSuccess())
    {
        logger.print("Failed to read schedule: ");
        logger.println(res.error());
        return ScheduleError::FILE_SCHEDULE_ERROR;
    }
    JsonDocument doc = res.value();
    if (doc.isNull())
    {
        return ScheduleError::NO_SCHEDULE_ERROR;
    }
    return setSchedule(doc);
}
String Schedule::getScheduleJson()
{
    JsonDocument doc;

    for (int i = 0; i < 6; i++)
    {
        JsonObject timeObj = doc.add<JsonObject>();
        timeObj["hour"] = timesList[i].hour;
        timeObj["minute"] = timesList[i].minute;
    }

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

ScheduleError Schedule::setSchedule(const String &jsonString)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error)
    {
        return ScheduleError::JSON_PARSE_ERROR;
    }

    this->setSchedule(doc);
    return NO_ERROR;
}

ScheduleError Schedule::setSchedule(const JsonDocument &doc)
{
    isSet = true;
    for (int i = 0; i < 6; i++)
    {
        timesList[i].hour = doc[i]["hour"].as<int>();
        timesList[i].minute = doc[i]["minute"].as<int>();
    }
    return NO_ERROR;
}