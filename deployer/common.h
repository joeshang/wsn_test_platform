/**
 * @file common.h
 * @brief The common header for project.
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-18
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
#define DECLS_BEGIN     extern "C" {
#define DECLS_END       }
#else
#define DECLS_BEGIN
#define DECLS_END
#endif

#define TRUE    1
#define FALSE   0

typedef enum _Ret
{
    RET_OK,
    RET_OOM,
    RET_STOP,
    RET_INVALID_PARAMS,
    RET_FAIL
}Ret;

#define return_val_if_fail(p, ret) if (!(p)) { return ret; }
#define SAFE_FREE(p) if ((p) != NULL) { free(p); (p) = NULL; }

#define PATH_MAX_LENGTH     4096
#define ERROR_MSG_LENGTH    256

#endif
