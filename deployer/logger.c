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
#include <assert.h>

#include "logger.h"

struct _Logger
{
    FILE *log_fd;
};

Logger *logger_create(int log_file_type, char *log_path)
{
    Logger *thiz = (Logger *)malloc(sizeof(Logger));

    if (thiz != NULL)
    {
        if (log_file_type == LOGGER_OUTPUT_TYPE_FILE)
        {
            thiz->log_fd = fopen(log_path, "w+");
            if (thiz->log_fd == NULL)
            {
                char error_msg[100];
                snprintf(error_msg, 100, "open log file %s failed", log_path);
                perror(error_msg);
                free(thiz);
                thiz = NULL;
            }
        }
        else if (log_file_type == LOGGER_OUTPUT_TYPE_STDOUT)
        {
            thiz->log_fd = stdout;
        }
        else
        {
            fprintf(stderr, "invalid log file type: %d\n", log_file_type);
            free(thiz);
            thiz = NULL;
        }
    }

    return thiz;
}

void logger_destroy(Logger *thiz)
{
    if (thiz == NULL)
    {
        return;
    }

   fclose(thiz->log_fd); 
   free(thiz);
}

void logger_printf(Logger *thiz, const char *format, ...)
{
    assert(thiz != NULL);
    assert(format != NULL);

    va_list arg = NULL;
    va_start(arg, format);
    vfprintf(thiz->log_fd, format, arg);
    va_end(arg);
}
