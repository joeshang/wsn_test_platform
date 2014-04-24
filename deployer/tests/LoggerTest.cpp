/**
 * @file LoggerTest.cpp
 * @brief The unit test for logger module.
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-18
 */

#include "CppUTest/TestHarness.h"

#include "logger.h"

TEST_GROUP(Logger)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(Logger, PrintfToFile)
{
    LONGS_EQUAL(RET_OK, logger_init("logger_test.log"));

    logger_printf("test logger module.\n");
    logger_printf("test output with arg(int): %d\n", 1);
    logger_printf("test output with arg(char *): %s\n", "Hello Logger");

    logger_close();
}

TEST(Logger, PrintfToStdout)
{
    LONGS_EQUAL(RET_OK, logger_init(NULL));

    logger_printf("test logger module.\n");
    logger_printf("test output with arg(int): %d\n", 1);
    logger_printf("test output with arg(char *): %s\n", "Hello Logger");

    logger_close();
}
