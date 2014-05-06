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

Ret  logger_init(const char *log_path);
void logger_close();
void logger_printf(const char *format, ...);

DECLS_END

#endif
