#ifndef FILE_REPOSITORY_H
#define FILE_REPOSITORY_H

#include <LittleFS.h>
#include <ArduinoJson.h>

#include "Result.h"
#include "logger.h"

enum FileRepositoryError
{
    NO_ERROR,
    INIT_ERROR,
    NOT_INITIALIZED_ERROR,
    FILE_NOT_FOUND_ERROR,
    FILE_OPEN_ERROR,
    FILE_READ_ERROR,
    DESERIALIZE_ERROR,
};

template <typename T>
using FileResult = Result<T, FileRepositoryError>;

class FileRepository
{
private:
    bool initialized = false;
    Logger logger;

public:
    FileRepository(Logger logger) : logger(logger) {};
    FileRepositoryError init();
    FileResult<JsonDocument &> readJsonFile(const String &path);
    FileRepositoryError writeJsonFile(const String &path, const JsonDocument &doc);
};

#endif