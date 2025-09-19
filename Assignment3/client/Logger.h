/******************************************************************************
*  UNX511-Assignment3
*  I declare that this lab is my own work in accordance with Seneca Academic Policy.
*  No part of this assignment has been copied manually or electronically from any other source
*  (including web sites) or distributed to other students.
*
*  Name: ___________________  Student ID: _______________  Date: ____________________
******************************************************************************/
#ifndef LOGGER_H
#define LOGGER_H

#include <cstdint>

#ifndef LOG_BUF_LEN
#define LOG_BUF_LEN 2048
#endif

// Default server config (can be overridden by environment variables)
#define DEFAULT_LOG_SERVER_IP   "127.0.0.1"
#define DEFAULT_LOG_SERVER_PORT 55555

// Log levels (in increasing severity)
typedef enum {
    DEBUG = 0,
    WARNING = 1,
    ERROR = 2,
    CRITICAL = 3
} LOG_LEVEL;

// Initializes the logger.
// Reads LOG_SERVER_IP and LOG_SERVER_PORT env vars if present.
// Returns 0 on success, -1 on failure.
int InitializeLog();

// Sets the minimum severity the logger will forward to the server.
void SetLogLevel(LOG_LEVEL level);

// Logs one message if level >= current filter.
void Log(LOG_LEVEL level, const char *file, const char *func, int line, const char *message);

// Shuts down logger and join its receiver thread.
void ExitLog();

#endif // LOGGER_H
