/**
 * @file telnet_commander.h
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-22
 */

#ifndef __TELNET_COMMANDER_H__
#define __TELNET_COMMANDER_H__

#include "common.h"

DECLS_BEGIN

struct _TelnetCommander;
typedef struct _TelnetCommander TelnetCommander;

TelnetCommander *telnet_commander_create();
void telnet_commander_destroy(TelnetCommander *thiz);

Ret telnet_commander_connect(TelnetCommander *thiz, const char *ip_address);
Ret telnet_commander_login(TelnetCommander *thiz, const char *username, const char *password);
Ret telnet_commander_send_one_line(TelnetCommander *thiz, const char *input);
Ret telnet_commander_close(TelnetCommander *thiz);

DECLS_END

#endif
