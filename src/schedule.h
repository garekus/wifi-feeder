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
    bool isSheduledTime(int hour, int minute);
    ScheduleError setSchedule(const JsonDocument &doc);
    Schedule(FileRepository &fileRepo, Logger &logger);

private:
    FileRepository &fileRepo;
    Logger &logger;
    ScheduleTime timesList[6];
    bool isSet;
    static const String filePath;
};

#endif