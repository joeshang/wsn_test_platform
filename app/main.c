/**
 * @file main.c
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-04
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "../common/common.h"
#include "packet_transfer.h"

#define BACKLOG         5
#define CMD_DATA_PORT   9160
#define REPROGRAM_PORT  9163

static int init_server_listen_at(int port)
{
    int sock;
    struct sockaddr_in addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        return -1;
    }

    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt failed");
        return -1;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind failed");
        return -1;
    }

    if (listen(sock, BACKLOG) == -1)
    {
        perror("listen failed");
        return -1;
    }

    return sock;
}

int main(int argc, char *argv[])
{
    int connect_socket = -1;
    int cmd_data_socket = -1; 
    int reprogram_socket = -1; 
    int max_fd;

    PacketTransfer *packet_transfer = NULL;
    if ((packet_transfer = packet_transfer_create()) == NULL)
    {
        fprintf(stderr, "create PacketTransfer failed\n");
        goto fail;
    }
    
    /* 初始化命令数据端口 */
    if ((cmd_data_socket = init_server_listen_at(CMD_DATA_PORT)) == -1)
    {
        goto fail;
    }
    debug("[Init]: command_data socket is listening at port: %d\n", CMD_DATA_PORT);
    
    /* 初始化重编程端口 */
    if ((reprogram_socket = init_server_listen_at(REPROGRAM_PORT)) == -1)
    {
        goto fail;
    }
    debug("[Init]: reprogram socket is listening at port: %d\n", REPROGRAM_PORT);

    /* 等待上位机连接 */
    fd_set listen_set;
    max_fd = (cmd_data_socket >= reprogram_socket) ? cmd_data_socket : reprogram_socket;
    for (;;)
    {
        debug("[Listen]: wait for connection\n");

        FD_ZERO(&listen_set);
        FD_SET(cmd_data_socket, &listen_set);
        FD_SET(reprogram_socket, &listen_set);
    
        if (select(max_fd + 1, &listen_set, NULL, NULL, NULL) == -1)
        {
            perror("select error");
            goto fail;
        }

        char client_ip_buf[INET_ADDRSTRLEN];
        struct sockaddr_in client_addr;
        socklen_t client_addr_len;
        bzero(&client_addr, sizeof(client_addr));

        /* 上位机连接命令数据端口 */
        if (FD_ISSET(cmd_data_socket, &listen_set))
        {
            if ((connect_socket = accept(cmd_data_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1)
            {
                perror("command data accept error");
                goto fail;
            }

            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip_buf, INET_ADDRSTRLEN);
            debug("[Listen]: client %s is connect command_data port\n", client_ip_buf);

            /* 进入命令数据处理流程，会一直循环等待上位机发送命令，直到上位机关闭连接 */
            packet_transfer_process(packet_transfer, connect_socket);

            debug("[Listen]: exit command_data port process\n");
        }
        /* 上位机连接重编程端口 */
        else if (FD_ISSET(reprogram_socket, &listen_set))
        {
            if ((connect_socket = accept(reprogram_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1)
            {
                perror("reprogram accept error");
                goto fail;
            }

            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip_buf, INET_ADDRSTRLEN);
            debug("[Listen]: client %s is connect reprogram port\n", client_ip_buf);
        }
    }

    close(cmd_data_socket);
    close(reprogram_socket);

    packet_transfer_destroy(packet_transfer);

    return 0;

fail:
    /* 非正常退出时需要释放已经申请的资源 */
    packet_transfer_destroy(packet_transfer);

    /* exit会关闭所有打开的文件描述符 */ 
    exit(EXIT_FAILURE); 
}
