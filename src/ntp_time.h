#ifndef TIME_REPOSITORY_H
#define TIME_REPOSITORY_H

#include <time.h>
#include <WString.h>

#include "file_repository.h"

struct NtpTimeErr
{
    enum Value : int
    {
        NO_ERROR,
        CONFIG_READ_ERROR,
        TIME_SYNC_ERROR
    };
};

class NtpTime
{
private:
    String currentTZ;
    // Default time zone (see "zones.json" from [github repo](https://github.com/nayarsystems/posix_tz_db))
    const String defaultTZ = "EET-2EEST,M3.5.0/3,M10.5.0/4";
    FileRepo fileRepo;
    const String configPath = "/time.json";
    Logger &logger;

    NtpTimeErr::Value loadPreservedConfig();
    void preserveConfig();
    void loadDefaultConfig();
    NtpTimeErr::Value tryToSyncTimeZone(String tz, int maxTimeoutSecs = 10);

public:
    NtpTime(FileRepo &fileRepo, Logger &logger);
    void init();
    NtpTimeErr::Value setTimeZone(String tz, int maxTimeoutSecs = 10);
    tm getTime();
    String getTimeStatusJson();
};

#endif
