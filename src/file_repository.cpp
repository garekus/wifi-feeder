#include "file_repository.h"

FileRepoErr::Value FileRepo::init()
{
    if (LittleFS.begin())
    {
        logger.println("file repo is initialized");
        initialized = true;
        return FileRepoErr::NO_ERROR;
    }
    logger.print("file repo initialization error");
    return FileRepoErr::INIT_ERROR;
}

File FileRepo::openForRead(const String &path)
{
    if (!initialized)
    {
        logger.print("FileRepo not initialized on FileRepo::openForRead: ");
        logger.println(path);
        return File();
    }

    if (!LittleFS.exists(path))
    {
        return File();
    }

    return LittleFS.open(path, "r");
}

FileRepoErr::Value FileRepo::readJsonFile(const String &path, JsonDocument &doc)
{
    File file = openForRead(path);
    if (!file)
    {
        return FileRepoErr::FILE_OPEN_ERROR;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error)
    {
        logger.print("file deserialize error: ");
        logger.println(error.c_str());
        logger.print("file:");
        logger.println(path.c_str());
        return FileRepoErr::DESERIALIZE_ERROR;
    }
    return FileRepoErr::NO_ERROR;
}

FileRepoErr::Value FileRepo::writeJsonFile(const String &path, const JsonDocument &doc)
{
    if (!initialized)
    {
        logger.print("FileRepo not initialized on FileRepo::writeJsonFile: ");
        logger.println(path);
        return FileRepoErr::NOT_INITIALIZED_ERROR;
    }
    File file = LittleFS.open(path, "w");
    if (!file)
    {
        return FileRepoErr::FILE_OPEN_ERROR;
    }

    serializeJson(doc, file);
    file.close();
    return FileRepoErr::NO_ERROR;
}