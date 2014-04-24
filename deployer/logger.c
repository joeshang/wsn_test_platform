/**
 * @file logger.c
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-18
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "logger.h"

static FILE *s_log_file = NULL;

Ret logger_init(const char *log_path)
{
    if (s_log_file != NULL)
    {
        logger_close();
    }

    if (log_path != NULL)
    {
        s_log_file = fopen(log_path, "w+");
        if (s_log_file == NULL)
        {
            fprintf(stderr, "open log file %s failed: %s\n", log_path, strerror(errno));
            return RET_FAIL;
        }
    }
    else 
    {
        s_log_file = stdout;
    }

    return RET_OK;
}

void logger_close()
{
    if (s_log_file != NULL && s_log_file != stdout)
    {
        fclose(s_log_file);
        s_log_file = NULL;
    }
}

void logger_printf(const char *format, ...)
{
    va_list arg = NULL;
    va_start(arg, format);
    vfprintf(s_log_file, format, arg);
    va_end(arg);
}

