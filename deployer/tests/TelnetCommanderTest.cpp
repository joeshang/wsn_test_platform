/**
 * @file TelnetCommanderTest.cpp
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-22
 */

#include "CppUTest/TestHarness.h"

#include "telnet_commander.h"

TEST_GROUP(TelnetCommander)
{
    TelnetCommander *telnet_commander;

    void setup()
    {
        telnet_commander = telnet_commander_create();
    }

    void teardown()
    {
        telnet_commander_destroy(telnet_commander);
    }
};

TEST(TelnetCommander, TelnetTest)
{
    LONGS_EQUAL(RET_OK, telnet_commander_connect(telnet_commander, "192.168.1.230"));
    LONGS_EQUAL(RET_OK, telnet_commander_login(telnet_commander, "root", ""));
    LONGS_EQUAL(RET_OK, telnet_commander_send_one_line(telnet_commander, "cp /etc/init.d/rcS .\r\n"));
    LONGS_EQUAL(RET_OK, telnet_commander_send_one_line(telnet_commander, "chmod 777 rcS\r\n"));
    LONGS_EQUAL(RET_OK, telnet_commander_send_one_line(telnet_commander, "ls > ls.log\r\n"));
    LONGS_EQUAL(RET_OK, telnet_commander_close(telnet_commander));
}
