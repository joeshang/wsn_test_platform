/**
 * @file main.c
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "deployer.h"
#include "deployer_config_json.h"
#include "logger.h"

const char *out[] = {
    "",
    "[success]",
    "[failed]"
};

void deployer_event_cb(DeployerEventContext *event_cb_ctx, void *ctx)
{
    const char *result = out[event_cb_ctx->result];

    switch (event_cb_ctx->event)
    {
        case DEPLOYER_EVENT_TARGET_START:
            printf("====================================\n");
            printf("target: %s\n", event_cb_ctx->target);
            break;
        case DEPLOYER_EVENT_FTP_CONNECT:
            printf("--> FTP connect %s\n", result);
            break;
        case DEPLOYER_EVENT_FTP_LOGIN:
            printf("--> FTP login %s\n", result);
            break;
        case DEPLOYER_EVENT_FTP_SET_TYPE:
            printf("--> FTP set type %s\n", result);
            break;
        case DEPLOYER_EVENT_FTP_PUT_FILE:
            printf("--> FTP put file %s %s\n", event_cb_ctx->file, result);
            break;
        case DEPLOYER_EVENT_FTP_CLOSE:
            printf("--> FTP close %s\n", result);
            break;
        case DEPLOYER_EVENT_TELNET_CONNECT:
            break;
        case DEPLOYER_EVENT_TELNET_COMMAND:
            break;
        case DEPLOYER_EVENT_TELNET_CLOSE:
            break;
        case DEPLOYER_EVENT_TARGET_END:
            printf("RESULT: %s\n", out[event_cb_ctx->result]);
            printf("====================================\n");
            printf("\n");
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    char *conf_file_name = NULL;

    if (argc == 1)
    {
        conf_file_name = CONF_DEFAULT_FILE;
    }
    else if (argc == 2)
    {
        conf_file_name = argv[1];
    }
    else
    {
        fprintf(stderr, "Usage: %s [config_file_name]\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    g_logger = logger_create(LOGGER_OUTPUT_TYPE_STDOUT, NULL);

    Deployer *deployer = deployer_create(deployer_event_cb, NULL);
    if (deployer == NULL)
    {
        fprintf(stderr, "create deployer failed.\n");
        exit(EXIT_FAILURE);
    }

    if (deployer_init(deployer, conf_file_name) != RET_OK)
    {
        exit(EXIT_FAILURE);
    }

    deployer_do_deploy(deployer);

    deployer_destroy(deployer);
    logger_destroy(g_logger);

    return 0;
}
