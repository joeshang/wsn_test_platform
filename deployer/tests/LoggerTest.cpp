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
    g_logger = logger_create(LOGGER_OUTPUT_TYPE_FILE, "../log");
    POINTERS_EQUAL(NULL, !g_logger);

    logger_printf(g_logger, "test logger module.\n");
    logger_printf(g_logger, "test output with arg(int): %d\n", 1);
    logger_printf(g_logger, "test output with arg(char *): %s\n", "Hello Logger");

    logger_destroy(g_logger);
}

TEST(Logger, PrintfToStdout)
{
    g_logger = logger_create(LOGGER_OUTPUT_TYPE_STDOUT, NULL);
    POINTERS_EQUAL(NULL, !g_logger);

    logger_printf(g_logger, "test logger module.\n");
    logger_printf(g_logger, "test output with arg(int): %d\n", 1);
    logger_printf(g_logger, "test output with arg(char *): %s\n", "Hello Logger");

    logger_destroy(g_logger);
}
