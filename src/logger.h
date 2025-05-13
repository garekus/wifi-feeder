#ifndef LOGGER_H
#define LOGGER_H

class Logger
{
public:
    void print(const char *message);
    void print(const String &message);
    void println(const char *message);
    void println();
    void println(const String &message);
    void println(const int num);
    void init();
};

#endif