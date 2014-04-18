/**
 * @file logger.h
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-18
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "common.h"

DECLS_BEGIN

struct _Logger;
typedef struct _Logger Logger;

Logger *g_logger;

enum _LoggerFileType
{
    LOGGER_OUTPUT_TYPE_FILE,
    LOGGER_OUTPUT_TYPE_STDOUT
};

Logger *logger_create(int log_file_type, char *log_path);
void logger_destroy(Logger *thiz);

void logger_printf(Logger *thiz, const char *format, ...);

DECLS_END

#endif
