/**
 * @file telnet_commander.c
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-22
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "telnet_commander.h"
#include "libtelnet.h"

#define TELNET_PORT         23
#define RECV_BUFFER_SIZE    1024
#define CRLF                "\r\n"
#define PROMPT              "]#"
#define LOGIN               "login:"
#define PASSWORD            "password:"
#define LOGIN_FAIL          "incorrect"
#define QUIT_COMMAND        "quit"

static const telnet_telopt_t telopts[] = {
	{ TELNET_TELOPT_ECHO,		TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_TTYPE,		TELNET_WILL, TELNET_DONT },
	{ TELNET_TELOPT_COMPRESS2,	TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_MSSP,		TELNET_WONT, TELNET_DO   },
	{ -1, 0, 0 }
};

struct _TelnetCommander
{
    telnet_t *telnet;
    int socket;
};

typedef int (* Match)(const char *string, void *user_data);

static void telnet_commander_send_with_crlf(int sock, const char *buffer, size_t size);
static void event_handler(telnet_t *telnet, telnet_event_t *ev, void *user_data);
static int match(const char *comparer, const char **desire_list, int *match_index);
static Ret telnet_commander_recv_until_match(TelnetCommander *thiz, 
        const char **desire_list,
        int *match_index);

static void telnet_commander_send_with_crlf(int sock, const char *buffer, size_t size)
{
	int rs;

	/* send data */
	while (size > 0) 
    {
		if ((rs = send(sock, buffer, size, 0)) == -1) 
        {
			fprintf(stderr, "send() failed: %s\n", strerror(errno));
			exit(1);
		} 
        else if (rs == 0) 
        {
			fprintf(stderr, "send() unexpectedly returned 0\n");
			exit(1);
		}

		/* update pointer and size to see if we've got more to send */
		buffer += rs;
		size -= rs;
	}

    /* send "\r\n" after the end of data */
    send(sock, CRLF, sizeof(CRLF), 0);
}

static int match(const char *comparer, const char **desire_list, int *match_index)
{
    int index = 0;
    const char **p_desire_item= desire_list;

    while (*p_desire_item != NULL)
    {
        if (strstr(comparer, *p_desire_item) != NULL)   /* matched! */
        {
            if (match_index != NULL)
            {
                *match_index = index;
            }
            return TRUE;
        }

        p_desire_item++;
        index++;
    }

    return FALSE;
}

static Ret telnet_commander_recv_until_match(TelnetCommander *thiz, 
        const char **desire_list,
        int *match_index)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    int rs;
    char recv_buffer[RECV_BUFFER_SIZE];

    do
    {
        rs = recv(thiz->socket, recv_buffer, RECV_BUFFER_SIZE, 0);
        if (rs > 0)
        {
            recv_buffer[rs] = '\0';
            debug("%s", recv_buffer);
        }
        else
        {
            fprintf(stderr, "telnet commander recv failed\n");
            return RET_FAIL;
        }

    }while(!match(recv_buffer, desire_list, match_index));

    return ret;
}

static void event_handler(telnet_t *telnet, telnet_event_t *ev, void *user_data) 
{
	int sock = *(int*)user_data;

	switch (ev->type) 
    {
	/* data received */
	case TELNET_EV_DATA:
		break;
	/* data must be sent */
	case TELNET_EV_SEND:
        debug("event handler send(size: %d)\n", ev->data.size);
		telnet_commander_send_with_crlf(sock, ev->data.buffer, ev->data.size);
		break;
	/* request to enable remote feature (or receipt) */
	case TELNET_EV_WILL:
		/* we'll agree to turn off our echo if server wants us to stop */
		/*if (ev->neg.telopt == TELNET_TELOPT_ECHO)*/
		break;
	/* notification of disabling remote feature (or receipt) */
	case TELNET_EV_WONT:
		/*if (ev->neg.telopt == TELNET_TELOPT_ECHO)*/
		break;
	/* request to enable local feature (or receipt) */
	case TELNET_EV_DO:
		break;
	/* demand to disable local feature (or receipt) */
	case TELNET_EV_DONT:
		break;
	/* respond to TTYPE commands */
	case TELNET_EV_TTYPE:
		/* respond with our terminal type, if requested */
		if (ev->ttype.cmd == TELNET_TTYPE_SEND) 
        {
			telnet_ttype_is(telnet, getenv("TERM"));
		}
		break;
	/* respond to particular subnegotiations */
	case TELNET_EV_SUBNEGOTIATION:
		break;
	/* error */
	case TELNET_EV_ERROR:
		fprintf(stderr, "ERROR: %s\n", ev->error.msg);
		exit(1);
	default:
		/* ignore */
		break;
	}
}

TelnetCommander *telnet_commander_create()
{
    TelnetCommander *thiz = (TelnetCommander *)malloc(sizeof(TelnetCommander));

    if (thiz != NULL)
    {
        thiz->socket = -1; 
    }

    return thiz;
}

Ret telnet_commander_connect(TelnetCommander *thiz, const char *ip_address)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(ip_address != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;

    thiz->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (thiz->socket == -1)
    {
        fprintf(stderr, "telnet command socket failed: %s\n", strerror(errno));
        goto fail;
    }

    thiz->telnet = telnet_init(telopts, event_handler, 0, &thiz->socket);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TELNET_PORT);
    inet_pton(AF_INET, ip_address, &server_addr.sin_addr);
    if (connect(thiz->socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        fprintf(stderr, "telnet command connect failed: %s\n", strerror(errno));
        goto fail;
    }

    return ret;
fail:
    telnet_commander_close(thiz);
    return RET_FAIL;
}

Ret telnet_commander_login(TelnetCommander *thiz, const char *username, const char *password)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(username != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(password != NULL, RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    int match_index;

    debug("telnet login\n");
    /* wait for login: */
    const char *desire_list_after_connect[] = { LOGIN, NULL };
    if (telnet_commander_recv_until_match(thiz, desire_list_after_connect, NULL) != RET_OK)
    {
        return RET_FAIL;
    }


    debug("telnet send username\n");
    telnet_send(thiz->telnet, username, strlen(username));
    /*telnet_send(thiz->telnet, CRLF, strlen(CRLF));*/

    match_index = -1;
    const char *desire_list_after_username[] = { PASSWORD, PROMPT, NULL };
    if (telnet_commander_recv_until_match(thiz, desire_list_after_username, &match_index) != RET_OK)
    {
        return RET_FAIL;
    }

    if (match_index == 0) /* require password */
    {
        debug("telnet send password\n");
        if (strlen(password) == 1)
        {
            fprintf(stderr, "telnet login failed: required password\n");
            return RET_FAIL;
        }

        telnet_send(thiz->telnet, password, strlen(password));
        /*telnet_send(thiz->telnet, CRLF, strlen(CRLF));*/

        match_index = -1;
        const char *desire_list_after_password[] = { LOGIN_FAIL, PROMPT, NULL };
        if (telnet_commander_recv_until_match(thiz, desire_list_after_password, NULL) != RET_OK)
        {
            return RET_FAIL;
        }

        if (match_index == 0) /* incorrect password */
        {
            fprintf(stderr, "telnet login failed: invalid username/password\n");
            return RET_FAIL;
        }
    }

    return ret;
}

Ret telnet_commander_send_one_line(TelnetCommander *thiz, const char *input)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);
    return_val_if_fail(input != NULL, RET_INVALID_PARAMS);

    telnet_send(thiz->telnet, input, strlen(input));
    /*telnet_send(thiz->telnet, CRLF, strlen(CRLF));*/

    const char *desire_list_after_command[] = { PROMPT, NULL };
    if (telnet_commander_recv_until_match(thiz, desire_list_after_command, NULL) != RET_OK)
    {
        return RET_FAIL;
    }

    return RET_OK;
}

Ret telnet_commander_close(TelnetCommander *thiz)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    if (thiz->socket != -1)
    {
        close(thiz->socket);
        thiz->socket = -1;
    }

    if (thiz->telnet != NULL)
    {
        telnet_free(thiz->telnet);
        thiz->telnet = NULL;
    }

    return RET_OK;
}

void telnet_commander_destroy(TelnetCommander *thiz)
{
    if (thiz == NULL)
    {
        return;
    }

    telnet_commander_close(thiz);

    free(thiz);
}

