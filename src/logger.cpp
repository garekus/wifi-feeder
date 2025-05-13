#include <Arduino.h>

#include "logger.h"

void Logger::init()
{
    Serial.begin(115200);
}
void Logger::print(const char *message)
{
    Serial.print(message);
}

void Logger::print(const String &message)
{
    Serial.print(message);
}

void Logger::println(const char *message)
{
    Serial.println(message);
}

void Logger::println(const String &message)
{
    Serial.println(message);
}

void Logger::println(const int num)
{
    Serial.println(num);
}