/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   main.cpp
 *
 */

#include <cstdlib>
#include <unistd.h>

#include "isotp_socket.h"

using namespace std;

/*
 *
 */
int main(int argc, char** argv)
{

    string device = "vcan0";
    if (argc > 1)
    {
        device = argv[1];
    }

    // test ecu
    IsoTpSocket tester(0x200,0x100, device.c_str());
    tester.openSender();

    constexpr array<uint8_t, 3> ReadDataByIdentifier01 = { 0x22, 0xf1, 0x90 };
    constexpr array<uint8_t, 3> ReadDataByIdentifier02 = { 0x22, 0xf1, 0x24 };
    constexpr array<uint8_t, 3> ReadDataByIdentifier03 = { 0x22, 0x1e, 0x23 };
    constexpr array<uint8_t, 3> ReadDataByIdentifier04 = { 0x22, 0x00, 0x00 };
    constexpr array<uint8_t, 2> security_access01 = { 0x27, 0x01 };
    constexpr array<uint8_t, 4> security_access02 = { 0x27, 0x02, 0x00, 0x00 };
    constexpr array<uint8_t, 2> diagnostic_session01 = { 0x10, 0x01 };

    tester.sendData(ReadDataByIdentifier01.data(), ReadDataByIdentifier01.size());
    usleep(2000);
    tester.sendData(ReadDataByIdentifier02.data(), ReadDataByIdentifier02.size());
    usleep(2000);
    tester.sendData(ReadDataByIdentifier03.data(), ReadDataByIdentifier03.size());
    usleep(2000);
    tester.sendData(ReadDataByIdentifier04.data(), ReadDataByIdentifier04.size());
    usleep(2000);
    tester.sendData(security_access01.data(), security_access01.size());
    usleep(2000);
    tester.sendData(security_access02.data(), security_access01.size());
    usleep(2000);
    tester.sendData(diagnostic_session01.data(), diagnostic_session01.size());
    usleep(2000);

    tester.closeSender();
    return 0;
}

