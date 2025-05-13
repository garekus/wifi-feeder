#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <functional>
#include <WString.h>

#include "file_repository.h"

enum ScheduleError
{
    NO_ERROR,
    FILE_SCHEDULE_ERROR,
    NO_SCHEDULE_ERROR,
    JSON_PARSE_ERROR
};

struct ScheduleTime
{
    int hour;
    int minute;
};

class Schedule
{
public:
    ScheduleError init();
    String getScheduleJson();
    ScheduleError setSchedule(const String &jsonString);
    bool isSheduledTime(int hour, int minute);

    Schedule(FileRepository &fileRepo, Logger &logger);

private:
    FileRepository &fileRepo;
    Logger &logger;
    ScheduleTime timesList[6];
    bool isSet;
    ScheduleError setSchedule(const JsonDocument &doc);
    static const String filePath;
};

#endif