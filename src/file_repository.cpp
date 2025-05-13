#include "file_repository.h"

FileRepositoryError FileRepository::init()
{
    if (LittleFS.begin())
    {
        this->initialized = true;
        return FileRepositoryError::NO_ERROR;
    }
    else
        return FileRepositoryError::INIT_ERROR;
}

FileResult<JsonDocument &> FileRepository::readJsonFile(const String &path)
{
    if (!this->initialized)
    {
        return FileRepositoryError::NOT_INITIALIZED_ERROR;
    }
    if (!LittleFS.exists(path))
    {
        return FileRepositoryError::FILE_NOT_FOUND_ERROR;
    }

    File file = LittleFS.open(path, "r");
    if (!file)
    {
        return FileRepositoryError::FILE_OPEN_ERROR;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error)
    {
        log.print("file deserialize error: ");
        log.println(error.c_str());
        log.print("file:");
        log.println(path.c_str());
        return FileRepositoryError::DESERIALIZE_ERROR;
    }
    return doc;
}

FileRepositoryError FileRepository::writeJsonFile(const String &path, const JsonDocument &doc)
{
    if (!this->initialized)
    {
        return FileRepositoryError::NOT_INITIALIZED_ERROR;
    }
    File file = LittleFS.open(path, "w");
    if (!file)
    {
        return FileRepositoryError::FILE_OPEN_ERROR;
    }

    serializeJson(doc, file);
    file.close();
    return FileRepositoryError::NO_ERROR;
}