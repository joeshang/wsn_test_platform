/**
 * @file FtpUploaderTest.cpp
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-19
 */

#include "CppUTest/TestHarness.h"

#include <stdio.h>
#include "ftp_uploader.h"
#include "logger.h"

TEST_GROUP(FtpUploader)
{
    FtpUploader *ftp_uploader;

    void setup()
    {
        ftp_uploader = ftp_uploader_create();
        g_logger = logger_create(LOGGER_OUTPUT_TYPE_STDOUT, NULL);
    }

    void teardown()
    {
        ftp_uploader_destroy(ftp_uploader);
        logger_destroy(g_logger);
    }
};

//TEST(FtpUploader, FtpConnectFailed)
//{
    //printf("==============================\n");
    //printf("FtpLoginConnectFailded:\n");

    //CHECK_EQUAL(RET_OK, ftp_uploader_connect(ftp_uploader, "127.0.0.11"));

    //printf("==============================\n");
    //printf("\n\n");
//}

//TEST(FtpUploader, FtpLoginInvalidUsername)
//{
    //printf("==============================\n");
    //printf("FtpLoginInvalidUsername:\n");

    //LONGS_EQUAL(RET_OK, ftp_uploader_connect(ftp_uploader, "127.0.0.1"));
    //LONGS_EQUAL(RET_FAIL, ftp_uploader_login(ftp_uploader, "invalid", "invalid"));
    //ftp_uploader_close(ftp_uploader);

    //printf("==============================\n");
    //printf("\n\n");
//}

//TEST(FtpUploader, FtpLoginInvalidPassword)
//{
    //printf("==============================\n");
    //printf("FtpLoginInvalidPassword:\n");

    //LONGS_EQUAL(RET_OK, ftp_uploader_connect(ftp_uploader, "127.0.0.1"));
    //LONGS_EQUAL(RET_FAIL, ftp_uploader_login(ftp_uploader, "joe", "invalid"));
    //ftp_uploader_close(ftp_uploader);

    //printf("==============================\n");
    //printf("\n\n");
//}

TEST(FtpUploader, FtpLoginSuccess)
{
    int fd;

    printf("==============================\n");
    printf("FtpPutFileSuccess:\n");
    LONGS_EQUAL(RET_OK, ftp_uploader_connect(ftp_uploader, "127.0.0.1"));
    LONGS_EQUAL(RET_OK, ftp_uploader_login(ftp_uploader, "joe", "lovejie1108"));
    LONGS_EQUAL(RET_OK, ftp_uploader_set_binary_mode(ftp_uploader));
    fd = fileno(fopen("AllTests.cpp", "r"));
    LONGS_EQUAL(RET_OK, ftp_uploader_put(ftp_uploader, fd, "AllTests.cpp"));
    ftp_uploader_close(ftp_uploader);
    printf("==============================\n");
    printf("\n\n");

    printf("==============================\n");
    printf("FtpLoginWithAnonymousSuccess:\n");
    LONGS_EQUAL(RET_OK, ftp_uploader_connect(ftp_uploader, "127.0.0.1"));
    LONGS_EQUAL(RET_OK, ftp_uploader_login(ftp_uploader, "anonymous", ""));
    ftp_uploader_close(ftp_uploader);
    printf("==============================\n");
    printf("\n\n");

    printf("==============================\n");
    printf("FtpLoginConnectFailded:\n");
    CHECK_EQUAL(RET_OK, ftp_uploader_connect(ftp_uploader, "127.0.0.11"));
    ftp_uploader_close(ftp_uploader);
    printf("==============================\n");
    printf("\n\n");

    printf("==============================\n");
    printf("FtpLoginInvalidUsername:\n");
    LONGS_EQUAL(RET_OK, ftp_uploader_connect(ftp_uploader, "127.0.0.1"));
    LONGS_EQUAL(RET_FAIL, ftp_uploader_login(ftp_uploader, "invalid", "invalid"));
    ftp_uploader_close(ftp_uploader);
    printf("==============================\n");
    printf("\n\n");

    printf("==============================\n");
    printf("FtpLoginInvalidPassword:\n");
    LONGS_EQUAL(RET_OK, ftp_uploader_connect(ftp_uploader, "127.0.0.1"));
    LONGS_EQUAL(RET_FAIL, ftp_uploader_login(ftp_uploader, "joe", "invalid"));
    ftp_uploader_close(ftp_uploader);
    printf("==============================\n");
    printf("\n\n");
}
