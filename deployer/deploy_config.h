/**
 * @file deploy_config.h
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-22
 */

#ifndef __DEPLOY_CONFIG_H__
#define __DEPLOY_CONFIG_H__

#include "common.h"

DECLS_BEGIN

#define CONF_DEFAULT_FILE           "deployer.conf"

#define CONF_JSON_FTP               "ftp"
#define CONF_JSON_FTP_USERNAME      "username"
#define CONF_JSON_FTP_PASSWORD      "password"

#define CONF_JSON_TELNET            "telnet"
#define CONF_JSON_TELNET_USERNAME   "username"
#define CONF_JSON_TELNET_PASSWORD   "password"
#define CONF_JSON_TELNET_COMMAND    "command file"

#define CONF_JSON_DEPLOY_TARGETS    "deploy targets"
#define CONF_JSON_DEPLOY_FILES      "deploy files"

DECLS_END

#endif
