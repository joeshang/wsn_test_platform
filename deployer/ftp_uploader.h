/**
 * @file ftp_uploader.h
 * @brief The ftp uploader module: upload deploying files into ARM board.
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-18
 */

#ifndef __FTP_UPLOADER_H__
#define __FTP_UPLOADER_H__

#include "common.h"

DECLS_BEGIN

struct _FtpUploader;
typedef struct _FtpUploader FtpUploader;

FtpUploader *ftp_uploader_create();
void ftp_uploader_destroy(FtpUploader *thiz);

Ret ftp_uploader_connect(FtpUploader *thiz, const char *ip_address);
Ret ftp_uploader_login(FtpUploader *thiz, const char *user_name, const char *password);
Ret ftp_uploader_set_binary_mode(FtpUploader *thiz);
Ret ftp_uploader_put(FtpUploader *thiz, int upload_fd, const char *file_name);
Ret ftp_uploader_close(FtpUploader *thiz);

DECLS_END

#endif
