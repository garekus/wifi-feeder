#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <functional>
#include <WString.h>

#include "file_repository.h"

struct ScheduleErr
{
    enum Value : int
    {
        NO_ERROR,
        FILE_SCHEDULE_ERROR,
        NO_SCHEDULE_ERROR,
        JSON_PARSE_ERROR
    };
};

struct ScheduleTime
{
    int hour;
    int minute;
};

class Schedule
{
public:
    ScheduleErr::Value init();
    String getScheduleJson();
    bool isSheduledTime(int hour, int minute);
    ScheduleErr::Value setSchedule(const JsonDocument &doc);
    Schedule(FileRepository &fileRepo, Logger &logger);

private:
    FileRepository &fileRepo;
    Logger &logger;
    ScheduleTime timesList[6];
    bool isSet;
    static const String filePath;
};

#endif