/**
 * @file config_json_generator.c
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-22
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "cJSON.h"
#include "deployer_config_json.h"

#define FTP_USERNAME        "plg"
#define FTP_PASSWORD        "plg"
#define TELNET_USERNAME     "root"
#define TELNET_PASSWORD     ""
#define TELNET_COMMAND      "db/telnet/command.sh"

const char *target_list[] = {
    "192.168.1.230"
};

const char *file_list[] = {
    "db/ftp/1.txt",
    "db/ftp/2.txt"
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s file_name\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
    {
        fprintf(stderr, "create config file %s failed: %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    int list_size;

    cJSON *ftp = cJSON_CreateObject();
    cJSON_AddStringToObject(ftp, CONF_JSON_FTP_USERNAME, FTP_USERNAME);
    cJSON_AddStringToObject(ftp, CONF_JSON_FTP_PASSWORD, FTP_PASSWORD);

    cJSON *telnet = cJSON_CreateObject();
    cJSON_AddStringToObject(telnet, CONF_JSON_TELNET_USERNAME, TELNET_USERNAME);
    cJSON_AddStringToObject(telnet, CONF_JSON_TELNET_PASSWORD, TELNET_PASSWORD);
    cJSON_AddStringToObject(telnet, CONF_JSON_TELNET_COMMAND, TELNET_COMMAND);

    list_size = sizeof(target_list) / sizeof(target_list[0]);
    cJSON *targets = cJSON_CreateStringArray(target_list, list_size);

    list_size = sizeof(file_list) / sizeof(file_list[0]);
    cJSON *files = cJSON_CreateStringArray(file_list, list_size);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, CONF_JSON_FTP, ftp);
    cJSON_AddItemToObject(root, CONF_JSON_TELNET, telnet);
    cJSON_AddItemToObject(root, CONF_JSON_DEPLOY_TARGETS, targets);
    cJSON_AddItemToObject(root, CONF_JSON_DEPLOY_FILES, files);

    char *out = cJSON_Print(root);
    write(fd, out, strlen(out));

    cJSON_Delete(root);
    free(out);

    return 0;
}
