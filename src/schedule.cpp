#include <LittleFS.h>
#include <ArduinoJson.h>

#include "schedule.h"

Schedule::Schedule(FileRepo &fileRepo, Logger &logger) : fileRepo(fileRepo), logger(logger)
{
    isSet = false;
}

bool Schedule::isFeedingTime(int hour, int minute)
{
    if (!isSet)
        return false;

    for (int i = 0; i < 5; i++)
    {
        if (timesList[i].hour == hour && timesList[i].minute == minute && !isFed[i])
        {
            cleanIsFed();
            isFed[i] = true;
            return true;
        }
    }
    return false;
}

const String Schedule::filePath = "/schedule.json";

ScheduleErr::Value Schedule::init()
{
    JsonDocument doc;
    FileRepoErr::Value fileErr = fileRepo.readJsonFile(filePath, doc);
    if (fileErr != FileRepoErr::NO_ERROR)
    {
        logger.print("Failed to read schedule: ");
        logger.println(fileErr);
        return ScheduleErr::FILE_SCHEDULE_ERROR;
    }
    if (doc.isNull())
    {
        return ScheduleErr::NO_SCHEDULE_ERROR;
    }
    return setSchedule(doc);
}
String Schedule::getScheduleJson()
{
    JsonDocument doc;

    for (int i = 0; i < 5; i++)
    {
        JsonObject timeObj = doc.add<JsonObject>();
        timeObj["hour"] = timesList[i].hour;
        timeObj["minute"] = timesList[i].minute;
    }

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

/**
 * @brief Set the schedule times from a JSON document
 *
 * The document should contain an array of five objects, each with "hour" and "minute" properties.
 * The times are stored in the timesList array.
 *
 * @param doc The JSON document containing the schedule times
 * @return NO_ERROR on success
 */
ScheduleErr::Value Schedule::setSchedule(const JsonDocument &doc)
{
    for (int i = 0; i < 5; i++)
    {
        timesList[i].hour = doc[i]["hour"].as<int>();
        timesList[i].minute = doc[i]["minute"].as<int>();
    }
    isSet = true;
    cleanIsFed();

    FileRepoErr::Value res = fileRepo.writeJsonFile(filePath, doc);
    if (res != FileRepoErr::NO_ERROR)
    {
        logger.print("Failed to save schedule: ");
        logger.println(res);
        return ScheduleErr::FILE_SCHEDULE_ERROR;
    }
    logger.println("Schedule saved successfully!");
    return ScheduleErr::NO_ERROR;
}

void Schedule::cleanIsFed()
{
    for (int i = 0; i < 5; i++)
    {
        isFed[i] = false;
    }
}