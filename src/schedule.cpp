#include <LittleFS.h>
#include <ArduinoJson.h>

#include "schedule.h"

bool ScheduleFeature::isSheduled(int hour, int minute)
{
    for (int i = 0; i < 6; i++)
    {
        if (timesList[i].hour == hour && timesList[i].minute == minute)
        {
            return true;
        }
    }
    return false;
}

ScheduleFeature::ScheduleFeature(String filePath, std::function<void()> callback = nullptr)
{
    this->filePath = filePath;
    this->scheduledCallback = callback;
}

ScheduleError ScheduleFeature::init()
{
    if (!LittleFS.exists(this->filePath))
    {
        return ScheduleError::FILE_NOT_FOUND_ERROR;
    }

    File file = LittleFS.open(this->filePath, "r");
    if (!file)
    {
        return ScheduleError::FILE_OPEN_ERROR;
    }

    // Read file into a String
    String jsonString = file.readString();
    file.close();

    if (jsonString.length() == 0)
    {
        return ScheduleError::FILE_READ_ERROR;
    }

    // Use the String version of setScheduleFromJson
    return this->setScheduleFromJson(jsonString);
}
String ScheduleFeature::getScheduleJson()
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

ScheduleError ScheduleFeature::setScheduleFromJson(const String &jsonString)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error)
    {
        return ScheduleError::JSON_PARSE_ERROR;
    }

    for (int i = 0; i < 6; i++)
    {
        timesList[i].hour = doc[i]["hour"].as<int>();
        timesList[i].minute = doc[i]["minute"].as<int>();
    }

    return NO_ERROR;
}
