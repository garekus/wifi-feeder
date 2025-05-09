#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <functional>
#include <WString.h>

enum ScheduleError
{
    NO_ERROR,
    FILE_NOT_FOUND_ERROR,
    FILE_OPEN_ERROR,
    FILE_READ_ERROR,
    JSON_PARSE_ERROR
};

struct ScheduleTime
{
    int hour;
    int minute;
};

class ScheduleFeature
{
public:
    ScheduleFeature(String filePath, std::function<void()> callback);
    ScheduleError init();
    String getScheduleJson();
    ScheduleError setScheduleFromJson(const String &jsonString);
    bool isSheduled(int hour, int minute);

private:
    ScheduleTime timesList[6];
    String filePath;
    std::function<void()> scheduledCallback;
};

#endif