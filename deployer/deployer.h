/**
 * @file deployer.h
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-21
 */

#ifndef __DEPLOYER_H__
#define __DEPLOYER_H__

#include "common.h"

DECLS_BEGIN

typedef enum _DeployerEvent
{
    DEPLOYER_EVENT_INIT,
    DEPLOYER_EVENT_TARGET_START,
    DEPLOYER_EVENT_FTP_CONNECT,
    DEPLOYER_EVENT_FTP_LOGIN,
    DEPLOYER_EVENT_FTP_SET_TYPE,
    DEPLOYER_EVENT_FTP_PUT_FILE,
    DEPLOYER_EVENT_FTP_CLOSE,
    DEPLOYER_EVENT_TELNET_CONNECT,
    DEPLOYER_EVENT_TELNET_LOGIN,
    DEPLOYER_EVENT_TELNET_COMMAND,
    DEPLOYER_EVENT_TELNET_CLOSE,
    DEPLOYER_EVENT_TARGET_END
}DeployerEvent;

typedef enum _DeployerEventResult
{
    DEPLOYER_EVENT_RESULT_INIT,
    DEPLOYER_EVENT_RESULT_SUCCESS,
    DEPLOYER_EVENT_RESULT_FAIL
}DeployerEventResult;

typedef struct _DeployerEventContext
{
    DeployerEvent event;
    DeployerEventResult result;
    const char *target;
    const char *file;
}DeployerEventContext;

typedef void (*DeployerEventCallBack)(DeployerEventContext *event_cb_ctx, void * event_cb_data);

struct _Deployer;
typedef struct _Deployer Deployer;

/**
 * @brief create deployer object
 *
 * @param callback should not be NULL
 * @param cb_data
 *
 * @return return NULL if create failed
 */
Deployer *deployer_create(DeployerEventCallBack event_cb,
        void *event_cb_data);
void deployer_destroy(Deployer *thiz);

Ret deployer_init(Deployer *thiz,
        const char *conf_file_name);

void deployer_do_deploy(Deployer *thiz);

DECLS_END

#endif
