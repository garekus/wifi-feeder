#include "file_repository.h"

FileRepoErr::Value FileRepository::init()
{
    if (LittleFS.begin())
    {
        this->initialized = true;
        return FileRepoErr::NO_ERROR;
    }
    else
        return FileRepoErr::INIT_ERROR;
}

FileRepoErr::Value FileRepository::readJsonFile(const String &path, JsonDocument &doc)
{
    if (!this->initialized)
    {
        return FileRepoErr::NOT_INITIALIZED_ERROR;
    }
    if (!LittleFS.exists(path))
    {
        return FileRepoErr::FILE_NOT_FOUND_ERROR;
    }

    File file = LittleFS.open(path, "r");
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

FileRepoErr::Value FileRepository::writeJsonFile(const String &path, const JsonDocument &doc)
{
    if (!this->initialized)
    {
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