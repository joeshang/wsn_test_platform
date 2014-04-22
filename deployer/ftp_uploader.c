/**
 * @file ftp_uploader.c
 * @brief The ftp uploader module: upload deploying files into ARM board.
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-18
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ftp_uploader.h"
#include "logger.h"

#define PORT_BUFFER_SIZE        10
#define SEND_BUFFER_SIZE        100
#define RESP_BUFFER_SIZE        100
#define DATA_BUFFER_SIZE        4096
#define COMMAND_END_SIZE        10

#define IP_MAX_LENGTH           16
#define PASSWORD_MAX_LENGTH     100
#define USER_NAME_MAX_LENGTH    32

#define FTP_COMMAND_PORT        21
#define FTP_DATA_PORT           20

struct _FtpUploader
{
    int command_socket;
    int data_socket;
    int is_connected;
    int is_transfered;
    char response[RESP_BUFFER_SIZE];
};

static Ret ftp_uploader_parse_reply_code(char *string, int *reply_code);
static Ret ftp_uploader_parse_passive_port(const char *string, char *ip, int *port);
static Ret ftp_uploader_read_response(FtpUploader *thiz);

static Ret ftp_uploader_read_response(FtpUploader *thiz)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    int readn;
    Ret ret = RET_OK;

    readn = read(thiz->command_socket, thiz->response, RESP_BUFFER_SIZE);
    if (readn < 0)
    {
        perror("ftp_uploader_read_response read failed.");
        ret = RET_FAIL;
    }
    else if (readn == 0)
    {
        fprintf(stderr, "ftp_uploader_read_response connection closed.\n");
        ret = RET_FAIL;
    }
    else
    {
        thiz->response[readn] = 0;
        logger_printf(g_logger, "%s", thiz->response);

        int reply_code;
        if (ftp_uploader_parse_reply_code(thiz->response, &reply_code) != RET_OK)
        {
            return RET_FAIL;
        }

        switch(reply_code / 100)
        {
            case 1:
            case 2:
            case 3:
                ret = RET_OK;
                break;
            case 4:
            case 5:
                ret = RET_FAIL;
                break;
            default:
                fprintf(stderr, "ftp_uploader_read_response invalid reply code.\n");
                ret = RET_FAIL;
                break;
        }
    }

    return ret;
}

static Ret ftp_uploader_parse_reply_code(char *string, int *reply_code)
{
   return_val_if_fail(string != NULL, RET_INVALID_PARAMS); 

   if (strlen(string) < 4)
   {
       fprintf(stderr, "response is too short\n");
       return RET_FAIL;
   }

   /* get reply code */
   char c = string[3];
   string[3] = '\0';
   *reply_code = atoi(string);
   string[3] = c;

   return RET_OK;
}

/**
 * @brief ftp_uploader_parse_passive_port 
 *
 * @param string
 * @param ip: ip buffer size must be at least 16(xxx.xxx.xxx.xxx\0)
 * @param port
 *
 * @return 
 */
static Ret ftp_uploader_parse_passive_port(const char *string, char *ip, int *port)
{
    return_val_if_fail(string != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(ip != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(port != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;

    /* 
     * string format: 227 entering passive mode (h1,h2,h3,h4,p1,p2)
     * ip address: h1.h2.h3.h4
     * port: p1 * 256 + p2 
     */
    int ip_index = 0;
    int port_index = 0;
    char port_buffer[PORT_BUFFER_SIZE];
    int p1;
    int p2;

    enum _ParseState
    {
        PARSE_START,
        IP_SECTION,
        PORT_SECTION,
        PARSE_END
    };
    const char *base = string;
    int index = 0;
    int state = PARSE_START;
    int delims_count = 0;
    char c;
    while (*(base + index) != '\0' || state != PARSE_END)
    {
        c = *(base + index);    
        switch (state)
        {
            case PARSE_START:
            {
                if (c == '(')
                {
                    state = IP_SECTION;
                }
                break;
            }
            case IP_SECTION:
            {
                if (isdigit(c))
                {
                    ip[ip_index++] = c;
                    if (ip_index == IP_MAX_LENGTH)
                    {
                        return RET_OOM;
                    }
                }
                else if (c == ',')
                {
                    delims_count++;
                    
                    if (delims_count < 4)  /* in the ip section */ 
                    {
                        ip[ip_index++] = '.';
                        if (ip_index == IP_MAX_LENGTH)
                        {
                            return RET_OOM;
                        }
                    }
                    else /* in the port section */
                    {
                        ip[ip_index] = '\0';
                        state = PORT_SECTION;
                    }
                }
                break;
            }
            case PORT_SECTION:
            {
                if (isdigit(c))
                {
                    port_buffer[port_index++] = c;

                    if (port_index == PORT_BUFFER_SIZE)
                    {
                        fprintf(stderr, "port index out of memory\n");
                        return RET_OOM;
                    }
                } 
                else if (c == ',')
                {
                    port_buffer[port_index] = '\0';
                    p1 = atoi(port_buffer);
                    port_index = 0;
                }
                else if (c == ')')
                {
                    port_buffer[port_index] = '\0';
                    p2 = atoi(port_buffer);
                    port_index = 0;
                    state = PARSE_END;
                }
                break;
            }
            default:
                break;
        }
         
        index++;
    }

    *port = p1 * 256 + p2;

    return ret;
}

FtpUploader *ftp_uploader_create()
{
    FtpUploader *thiz = (FtpUploader *)malloc(sizeof(FtpUploader));

    if (thiz != NULL)
    {
        thiz->is_connected = FALSE;
        thiz->is_transfered = FALSE;
    }

    return thiz;
}

void ftp_uploader_destroy(FtpUploader *thiz)
{
    if (thiz == NULL)
    {
        return;
    }

    free(thiz);
}

Ret ftp_uploader_connect(FtpUploader *thiz, const char *ip_address)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(ip_address != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    
    thiz->command_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in ftp_command_addr;
    bzero(&ftp_command_addr, sizeof(ftp_command_addr));
    ftp_command_addr.sin_family = AF_INET;
    ftp_command_addr.sin_port = htons(FTP_COMMAND_PORT);
    inet_pton(AF_INET, ip_address, &ftp_command_addr.sin_addr);

    /* connect to ftp server */
    if (connect(thiz->command_socket, (struct sockaddr *)&ftp_command_addr, sizeof(ftp_command_addr)) == -1)
    {
        fprintf(stderr, "connect to %s ftp command port failed: %s\n", ip_address, strerror(errno));
        return RET_FAIL;
    }

    thiz->is_connected = TRUE;

    if (ftp_uploader_read_response(thiz) != RET_OK)
    {
        return RET_FAIL;
    }

    return ret;
}

Ret ftp_uploader_login(FtpUploader *thiz, const char *user_name, const char *password)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(user_name != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(password != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    int user_name_size;
    int password_size; 
    int send_buf_size;
    char *send_buf = NULL;

    if ((user_name_size = strlen(user_name)) > USER_NAME_MAX_LENGTH)
    {
        fprintf(stderr, "ftp_uploader_login user name too long.\n");
        goto fail;
    }

    if ((password_size = strlen(password)) > PASSWORD_MAX_LENGTH)
    {
        fprintf(stderr, "ftp_uploader_login password too long.\n");
        goto fail;
    }

    send_buf_size = (user_name_size > password_size ? user_name_size : password_size) + COMMAND_END_SIZE;
    send_buf = (char *)malloc(send_buf_size);
    if (send_buf == NULL)
    {
        fprintf(stderr, "ftp_uploader_login malloc send buffer failed.\n");
        goto fail;
    }
    
    /* send command: USER user_namename\r\n */ 
    snprintf(send_buf, send_buf_size, "USER %s\r\n", user_name);
    logger_printf(g_logger, "%s", send_buf);
    write(thiz->command_socket, send_buf, strlen(send_buf));
    if (ftp_uploader_read_response(thiz) != RET_OK)
    {
        goto fail;
    }

    /* send command: PASS password\r\n */ 
    snprintf(send_buf, send_buf_size, "PASS %s\r\n", password);
    logger_printf(g_logger, "%s", send_buf);
    write(thiz->command_socket, send_buf, strlen(send_buf));
    if (ftp_uploader_read_response(thiz) != RET_OK)
    {
        goto fail;
    }

    free(send_buf);

    return ret;

fail:
    SAFE_FREE(send_buf);
    return RET_FAIL;
}

Ret ftp_uploader_set_binary_mode(FtpUploader *thiz)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    const char *send_buf = "TYPE I\r\n";
    logger_printf(g_logger, "%s", send_buf);
    write(thiz->command_socket, send_buf, strlen(send_buf));
    if (ftp_uploader_read_response(thiz) != RET_OK)
    {
        return RET_FAIL;
    }

    return ret;
}

Ret ftp_uploader_put(FtpUploader *thiz, int upload_fd, const char *file_name)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(file_name != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    char send_buf[SEND_BUFFER_SIZE];

    /* entering passive mode */
    sprintf(send_buf, "PASV\r\n");
    logger_printf(g_logger, "%s", send_buf);
    write(thiz->command_socket, send_buf, strlen(send_buf));
    if (ftp_uploader_read_response(thiz) != RET_OK)
    {
        return RET_FAIL;
    }

    /* parse ftp server's data port in passive mode */
    char ip_address[IP_MAX_LENGTH];
    int port;
    if ((ret = ftp_uploader_parse_passive_port(thiz->response, ip_address, &port)) != RET_OK)
    {
        return ret;
    }

    thiz->data_socket = socket(AF_INET, SOCK_STREAM, 0);

    /* connect to ftp server's data port */
    struct sockaddr_in ftp_data_addr;
    bzero(&ftp_data_addr, sizeof(ftp_data_addr));
    ftp_data_addr.sin_family = AF_INET;
    ftp_data_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_address, &ftp_data_addr.sin_addr);

    if (connect(thiz->data_socket, (struct sockaddr *)&ftp_data_addr, sizeof(ftp_data_addr)) == -1)
    {
        fprintf(stderr, "connect to %s ftp data port failed: %s\n", ip_address, strerror(errno));
        
        return RET_FAIL;
    }

    thiz->is_transfered = TRUE;

    /* send command: "STOR filename\r\n" */
    snprintf(send_buf, SEND_BUFFER_SIZE, "STOR %s\r\n", file_name);
    logger_printf(g_logger, "%s", send_buf);
    write(thiz->command_socket, send_buf, strlen(send_buf));
    if (ftp_uploader_read_response(thiz) != RET_OK)
    {
        return RET_FAIL;
    }

    /* send file content */
    int bytes_read;
    char data_buf[DATA_BUFFER_SIZE];
    for (;;)
    {
        if ((bytes_read = read(upload_fd, data_buf, DATA_BUFFER_SIZE)) == 0)
        {
            break;
        }

        if (write(thiz->data_socket, data_buf, bytes_read) == -1)
        {
            break;
        }
    }

    /* close data port socket and wait for 2xx successful reply code */
    close(thiz->data_socket);
    thiz->is_transfered = FALSE;
    if (ftp_uploader_read_response(thiz) != RET_OK)
    {
        return RET_FAIL;
    }

    return ret;
}

Ret ftp_uploader_close(FtpUploader *thiz)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    Ret ret = RET_OK;

    const char *send_buf = "QUIT\r\n";
    logger_printf(g_logger, "%s", send_buf);
    write(thiz->command_socket, send_buf, strlen(send_buf));
    if (ftp_uploader_read_response(thiz) != RET_OK)
    {
    }

    if (thiz->is_transfered == TRUE)
    {
        close(thiz->data_socket);
        thiz->is_connected = FALSE;
    }
    if (thiz->is_connected == TRUE)
    {
        close(thiz->command_socket);
        thiz->is_connected = FALSE;
    }

    bzero(thiz->response, RESP_BUFFER_SIZE);

    return ret;
}
