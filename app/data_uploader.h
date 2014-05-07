/**
 * @file data_uploader.h
 * @brief 负责接收并解析上位机发送命令，发送给FPGA板，并从FPGA板读取数据，发回上位机
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-05-06
 */

#ifndef __DATA_UPLOADER_H__
#define __DATA_UPLOADER_H__

#include "../common/common.h"

DECLS_BEGIN

struct _DataUploader;
typedef struct _DataUploader DataUploader;

DataUploader *data_uploader_create();
void data_uploader_destroy(DataUploader *thiz);
Ret  data_uploader_process(DataUploader *thiz, int socket);
Ret  data_uploader_reset(DataUploader *thiz);

DECLS_END

#endif
