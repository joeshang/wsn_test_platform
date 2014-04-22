/**
 * @file deployer.c
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "deployer.h"
#include "deployer_config_json.h"
#include "ftp_uploader.h"
#include "cJSON.h"

#define DEFAULT_DEPLOY_NAME     "default_deploy_name"
#define PROC_LINK_LENGTH        50

struct _Deployer
{
    /* ftp related */
    FtpUploader *ftp_uploader;
    char *ftp_username;
    char *ftp_password;
    
    /* telnet related */
    char *telnet_username;
    char *telnet_password;
    int telnet_command_fd;

    /* deploy targets */
    int deploy_target_count;
    char **deploy_target_list;

    /* deploy files */
    int deploy_file_count;
    int *deploy_file_list;

    /* event callback related */
    DeployerEventContext *event_cb_ctx;
    DeployerEventCallBack event_cb;
    void *event_cb_data;
};

static Ret deployer_do_one_target(Deployer *thiz, const char *ip_address)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    int i;

    thiz->event_cb_ctx->result = DEPLOYER_EVENT_RESULT_SUCCESS;

    /* step . telnet connect*/
    thiz->event_cb_ctx->event = DEPLOYER_EVENT_TELNET_CONNECT;
    thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);

    /* step . ftp connect */
    thiz->event_cb_ctx->event = DEPLOYER_EVENT_FTP_CONNECT;
    if ((ret = ftp_uploader_connect(thiz->ftp_uploader, ip_address)) != RET_OK)
    {
        goto fail;
    }
    thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);

    /* step . ftp login and config */
    thiz->event_cb_ctx->event = DEPLOYER_EVENT_FTP_LOGIN;
    if ((ret = ftp_uploader_login(thiz->ftp_uploader, thiz->ftp_username, thiz->ftp_password)) != RET_OK)
    {
        goto fail;
    }
    thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);

    thiz->event_cb_ctx->event = DEPLOYER_EVENT_FTP_SET_TYPE;
    if ((ret = ftp_uploader_set_binary_mode(thiz->ftp_uploader)) != RET_OK)
    {
        goto fail;
    }
    thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);

    /* step . ftp upload every deploying file */
    char *deploy_name = NULL;
    char proc_link[PROC_LINK_LENGTH];
    char path_name[PATH_MAX_LENGTH];
    int n;
    for (i = 0; i < thiz->deploy_file_count; i++)
    {   
        /* get file name from fd by readlink */
        snprintf(proc_link, PROC_LINK_LENGTH, "/proc/self/fd/%d", thiz->deploy_file_list[i]);
        n = readlink(proc_link, path_name, PATH_MAX_LENGTH);
        if (n == -1)
        {
            perror("readlink to get file name from fd failed");
            snprintf(proc_link, PROC_LINK_LENGTH, "%s_%d", DEFAULT_DEPLOY_NAME, thiz->deploy_file_list[i]);
            deploy_name = proc_link;
        }
        else
        {
            char *save_ptr;
            char *next;
            path_name[n] = '\0';
            deploy_name = strtok_r(path_name, "/", &save_ptr);

            while ((next = strtok_r(NULL, "/", &save_ptr)) != NULL)
            {
                deploy_name = next;
            }
        }
        
        thiz->event_cb_ctx->event = DEPLOYER_EVENT_FTP_PUT_FILE;
        thiz->event_cb_ctx->file = deploy_name;
        if ((ret = ftp_uploader_put(thiz->ftp_uploader, thiz->deploy_file_list[i], deploy_name)) != RET_OK)
        {
            goto fail;
        }
        thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);
    }

    /* step . ftp close */
    thiz->event_cb_ctx->event = DEPLOYER_EVENT_FTP_CLOSE;
    thiz->event_cb_ctx->file = NULL;
    ftp_uploader_close(thiz->ftp_uploader);
    thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);

    /* step . telnet send command after ftp uploading */
    thiz->event_cb_ctx->event = DEPLOYER_EVENT_TELNET_COMMAND;
    thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);

    /* step . telnet close */
    thiz->event_cb_ctx->event = DEPLOYER_EVENT_TELNET_CLOSE;
    thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);

    return ret;
fail:
    thiz->event_cb_ctx->result = DEPLOYER_EVENT_RESULT_FAIL;
    thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);
    return ret;
}

static Ret deployer_parse_config_file(Deployer *thiz, cJSON *root)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(root != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    int i;

    /* ftp related */
    cJSON *ftp = cJSON_GetObjectItem(root, CONF_JSON_FTP);
    if (ftp == NULL)
    {
        fprintf(stderr, "parse config file failed: can't find [ftp] section.\n");
        goto fail;
    }

    /* ftp/username */
    cJSON *ftp_username = cJSON_GetObjectItem(ftp, CONF_JSON_FTP_USERNAME); 
    if (ftp_username == NULL)
    {
        fprintf(stderr, "parse config file failed: can't find [ftp/username] section.\n");
        goto fail;
    }
    thiz->ftp_username = strdup(ftp_username->valuestring);
    debug("-> ftp/username: %s\n", ftp_username->valuestring);

    /* ftp/password */
    cJSON *ftp_password = cJSON_GetObjectItem(ftp, CONF_JSON_FTP_PASSWORD);
    if (ftp_password == NULL)
    {
        thiz->ftp_password = strdup("");
    }
    else
    {
        thiz->ftp_password = strdup(ftp_password->valuestring);
    }
    debug("-> ftp/password: %s\n", ftp_password->valuestring);

    /* telnet related */
    cJSON *telnet = cJSON_GetObjectItem(root, CONF_JSON_TELNET);
    if (telnet == NULL)
    {
        fprintf(stderr, "parse config file failed: can't find [telnet] section.\n");
        goto fail;
    }

    /* telnet/username */
    cJSON *telnet_username = cJSON_GetObjectItem(telnet, CONF_JSON_TELNET_USERNAME);
    if (telnet_username == NULL)
    {
        fprintf(stderr, "parse config file failed: can't find [telnet/username] section.\n");
        goto fail;
    }
    thiz->telnet_username = strdup(telnet_username->valuestring);
    debug("-> telnet/username: %s\n", telnet_username->valuestring);

    /* telnet/password */
    cJSON *telnet_password = cJSON_GetObjectItem(telnet, CONF_JSON_TELNET_PASSWORD);
    if (telnet_password == NULL)
    {
        thiz->telnet_password = strdup("");
    }
    else
    {
        thiz->telnet_password = strdup(telnet_password->valuestring);
    }
    debug("-> telnet/password: %s\n", telnet_password->valuestring);

    /* telnet/command file */
    cJSON *telnet_command_file = cJSON_GetObjectItem(telnet, CONF_JSON_TELNET_COMMAND);
    if (telnet_command_file == NULL)
    {
        fprintf(stderr, "parse config file failed: can't find [telnet/command file] section.\n");
        goto fail;
    }
    debug("-> telnet/command: %s\n", telnet_command_file->valuestring);
    thiz->telnet_command_fd = open(telnet_command_file->valuestring, O_RDONLY);
    if (thiz->telnet_command_fd == -1)
    {
        fprintf(stderr, "open telnet command file %s failed: %s\n", telnet_command_file->valuestring, strerror(errno));
        goto fail;
    }

    /* deploy targets */
    cJSON *targets = cJSON_GetObjectItem(root, CONF_JSON_DEPLOY_TARGETS);
    if (targets == NULL)
    {
        fprintf(stderr, "parse config file failed: can't find [deploy targets] section.\n");
        goto fail;
    }
    /* get deploy targets' count */
    thiz->deploy_target_count = cJSON_GetArraySize(targets);
    if (thiz->deploy_target_count == 0)
    {
        fprintf(stderr, "parse config file failed: deploy targets' count is 0.\n");
        goto fail;
    }
    /* malloc deploy target list */
    thiz->deploy_target_list = (char **)malloc(thiz->deploy_target_count);
    if (thiz->deploy_target_list == NULL)
    {
        fprintf(stderr, "malloc deploy target list failed.\n");
        goto fail;
    }
    for (i = 0; i < thiz->deploy_target_count; i++)
    {
        thiz->deploy_target_list[i] = NULL;
    }
    /* get deploy target address from json */
    cJSON *one_target;
    for (i = 0; i < thiz->deploy_target_count; i++)
    {
        one_target = cJSON_GetArrayItem(targets, i);
        if (one_target != NULL)
        {
            thiz->deploy_target_list[i] = strdup(one_target->valuestring);
            debug("-> deploy target[%d]: %s\n", i, one_target->valuestring);
        }
    }

    /* deploy files */
    cJSON *files = cJSON_GetObjectItem(root, CONF_JSON_DEPLOY_FILES);
    if (files == NULL)
    {
        fprintf(stderr, "parse config file failed: can't find [deploy files] section.\n");
        goto fail;
    }
    /* get deploy files' count */
    thiz->deploy_file_count = cJSON_GetArraySize(files);
    if (thiz->deploy_file_count == 0)
    {
        fprintf(stderr, "parse config file failed: deploy files' count is 0.\n");
        goto fail;
    }
    /* malloc deploy file list and init */
    thiz->deploy_file_list = (int *)malloc(thiz->deploy_file_count);
    if (thiz->deploy_file_list == NULL)
    {
        fprintf(stderr, "malloc deploy file list failed.\n");
        goto fail;
    }
    for (i = 0; i < thiz->deploy_file_count; i++)
    {
        thiz->deploy_file_list[i] = -1;
    }

    cJSON *one_file;
    for (i = 0; i < thiz->deploy_file_count; i++)
    {
        one_file = cJSON_GetArrayItem(files, i);
        if (one_file != NULL)
        {
            debug("-> deploy file[%d]: %s\n", i, one_file->valuestring);
            thiz->deploy_file_list[i] = open(one_file->valuestring, O_RDONLY);
            if (thiz->deploy_file_list[i] == -1)
            {
                fprintf(stderr, "open deploy file %s failed.\n", one_file->valuestring);
                goto fail;
            }
        }
    }

    return ret;

fail:
    cJSON_Delete(root);
    deployer_destroy(thiz);
    return RET_FAIL;
}

Deployer *deployer_create(DeployerEventCallBack event_cb, 
        void *event_cb_data)
{
    if (event_cb == NULL)
    {
        fprintf(stderr, "deployer_create: callback is NULL\n");
        return NULL;
    }

    Deployer *thiz = (Deployer *)malloc(sizeof(Deployer));

    if (thiz != NULL)
    {
        thiz->ftp_uploader = ftp_uploader_create();
        if (thiz->ftp_uploader == NULL)
        {
            goto fail;
        }
        thiz->ftp_username = NULL;
        thiz->ftp_password = NULL;
        
        thiz->telnet_username = NULL;
        thiz->telnet_password = NULL;
        thiz->telnet_command_fd = -1;

        thiz->deploy_file_count = 0;
        thiz->deploy_file_list = NULL;

        thiz->deploy_file_list = 0;
        thiz->deploy_target_list = NULL;

        thiz->event_cb_ctx = (DeployerEventContext *)malloc(sizeof(DeployerEventContext));
        if (thiz->event_cb_ctx == NULL)
        {
            fprintf(stderr, "deployer_create: malloc DeployerEventContext failed\n");
            goto fail;
        }
        thiz->event_cb_ctx->event = DEPLOYER_EVENT_INIT;
        thiz->event_cb_ctx->result = DEPLOYER_EVENT_RESULT_INIT;
        thiz->event_cb_ctx->target = NULL;
        thiz->event_cb_ctx->file = NULL;

        thiz->event_cb = event_cb;
        thiz->event_cb_data = event_cb_data;
    }

    return thiz;
fail:
    deployer_destroy(thiz);
    return NULL;
}

void deployer_destroy(Deployer *thiz)
{
    if (thiz == NULL)
    {
        return;
    }

    SAFE_FREE(thiz->event_cb_ctx);

    SAFE_FREE(thiz->ftp_uploader);
    SAFE_FREE(thiz->ftp_username);
    SAFE_FREE(thiz->ftp_password);

    SAFE_FREE(thiz->telnet_username);
    SAFE_FREE(thiz->telnet_password);
    close(thiz->telnet_command_fd);

    if (thiz->deploy_target_list != NULL)
    {
        int i;
        for (i = 0; i < thiz->deploy_target_count; i++)
        {
            if (thiz->deploy_target_list[i] != NULL)
            {
                SAFE_FREE(thiz->deploy_target_list[i]);
            }
        }

        free(thiz->deploy_target_list);
        thiz->deploy_target_list = NULL;
    }

    if (thiz->deploy_file_list != NULL)
    {
        int i;
        for (i = 0; i < thiz->deploy_file_count; i++)
        {
            if (thiz->deploy_file_list[i] != -1)
            {
                close(thiz->deploy_file_list[i]);
            }
        }

        free(thiz->deploy_file_list);
        thiz->deploy_file_list = NULL;
    }

    if (thiz->ftp_uploader != NULL)
    {
        ftp_uploader_destroy(thiz->ftp_uploader);
    }

    free(thiz);
}

Ret deployer_init(Deployer *thiz, const char *conf_file_name)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(conf_file_name != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    cJSON *conf_json = NULL;

    FILE *conf_file = fopen(conf_file_name, "r");
    if (conf_file == NULL)
    {
        fprintf(stderr, "open config file %s failed: %s\n", conf_file_name, strerror(errno));
        return RET_FAIL;
    }
    debug("open config file %s ok\n", conf_file_name);

    /* extract buffer from config file */
    fseek(conf_file, 0, SEEK_END);
    long length = ftell(conf_file);
    fseek(conf_file, 0, SEEK_SET);
    char *conf_buffer = (char *)malloc(length + 1);
    fread(conf_buffer, 1, length, conf_file);
    fclose(conf_file);

    /* get json root object */
    conf_buffer[length] = '\0';
    conf_json = cJSON_Parse(conf_buffer);
    // XXX: is free at here properly?
    free(conf_buffer);
    if (conf_json == NULL)
    {
        fprintf(stderr, "parse config file %s failed.\n", conf_file_name);
        return RET_FAIL;
    }

    debug("parsing config file ...\n");
    /* parse config file */
    if ((ret = deployer_parse_config_file(thiz, conf_json)) != RET_OK)
    {
        return ret;
    }

    return ret;
}

void deployer_do_deploy(Deployer *thiz)
{
    assert(thiz != NULL);

    int i;
    Ret ret;

    for (i = 0; i < thiz->deploy_target_count; i++)
    {
        if (thiz->deploy_target_list[i] != NULL)
        {
            thiz->event_cb_ctx->target = thiz->deploy_target_list[i];

            thiz->event_cb_ctx->event = DEPLOYER_EVENT_TARGET_START;
            thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);

            ret = deployer_do_one_target(thiz, thiz->deploy_target_list[i]);

            thiz->event_cb_ctx->event = DEPLOYER_EVENT_TARGET_END;
            if (ret == RET_OK)
            {
                thiz->event_cb_ctx->result = DEPLOYER_EVENT_RESULT_SUCCESS;
            }
            else
            {
                thiz->event_cb_ctx->result = DEPLOYER_EVENT_RESULT_FAIL;
            }
            thiz->event_cb(thiz->event_cb_ctx, thiz->event_cb_data);
        }
    }
}
