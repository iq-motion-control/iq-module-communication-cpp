/*
 * This example C++ program uses the Power Monitor client to retrieve the Voltage of a motor.
 * The serial port setup for windows is based on the example provided by microsoft:
 *      https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
 *
 * Name: 4006_speed_v0_3_0_test.cpp
 * Last update: 2026/03/13 by Ben Quan
 * Author: Ben Quan
 */

#include <tchar.h>
#include <windows.h>

#include <iostream>
// #include <list>

#include "../inc/brushless_drive_client.hpp"
#include "../inc/client_communication.cpp"
#include "../inc/generic_interface.hpp"
#include "../inc/uavcan_node_client.hpp"
#include "../inc/throttle_source_manager_client.hpp"

using namespace std;

HANDLE comPort;                    // Handler for COM port
TCHAR *pcCommPort = TEXT("COM4");  // Change COM4 to whichever port your motor is connected to
GenericInterface com;              // Interface used by com port to communicate with motor

BrushlessDriveClient brushlessDrive(0);                      // Initialize Brushless Drive Client
UavcanNodeClient uavcanNode(0);                              // Initialize UAVCAN Node Client
ThrottleSourceManagerClient throttleSourceManager(0);        // Initialize Throttle Source Manager Client

// Initialize clientList to make it easier to call ReadMsg for each client
ClientAbstract *clientList[3] = {&brushlessDrive,
                                 &uavcanNode,
                                 &throttleSourceManager
                                 };

//  Send out any message data we have over the serial interface
int handleComTx() {
    uint8_t packetBuffer[64];
    uint8_t length = 0;
    DWORD bytesWritten;

    // Get the packet from the com interface and place it into the packet buffer
    if (com.GetTxBytes(packetBuffer, length)) {
        WriteFile(comPort, packetBuffer, length, &bytesWritten, NULL);
    }

    return bytesWritten;
}

// Grab any received data on the serial interface
int handleComRx() {
    uint8_t recvBytes[64];
    DWORD dwBytesReceived;

    ReadFile(comPort, &recvBytes, 64, &dwBytesReceived, 0);
    com.SetRxBytes(recvBytes, dwBytesReceived);

    return dwBytesReceived;
}

// Hand off any received data to each module so they can handle it
void updateModules() {
    // Temporary Pointer to the packet data location
    uint8_t *packetData;
    uint8_t packetLength;

    // Loads the packet data buffer with data receieved from the motor
    while (com.PeekPacket(&packetData, &packetLength)) {
        for (auto &client : clientList) {
            client->ReadMsg(packetData, packetLength);
        };
        com.DropPacket();
    }
}

// Handles communication with motor
void sendMessageAndProcessReply() {
    handleComTx();
    handleComRx();
    updateModules();
}

uint32_t getWarningEnableBitmask() {
    uavcanNode.warning_enable_bitmask_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.warning_enable_bitmask_.get_reply();
}

uint32_t getErrorEnableBitmask() {
    uavcanNode.error_enable_bitmask_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.error_enable_bitmask_.get_reply();
}

uint32_t getCriticalEnableBitmask() {
    uavcanNode.critical_enable_bitmask_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.critical_enable_bitmask_.get_reply();
}

uint32_t getErrorCountConfiguration() {
    uavcanNode.error_count_configuration_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.error_count_configuration_.get_reply();
}


uint8_t getCurrentActiveThrottleSource() {
    throttleSourceManager.current_active_throttle_source_.get(com);
    sendMessageAndProcessReply();
    throttleSourceManager.current_active_throttle_source_.get_reply();
}

int main() {
    comPort = CreateFile(pcCommPort, GENERIC_READ | GENERIC_WRITE,
                         0,              //  must be opened with exclusive-access
                         NULL,           //  default security attributes
                         OPEN_EXISTING,  //  must use OPEN_EXISTING
                         0,              //  not overlapped I/O
                         NULL);          //  hTemplate must be NULL for comm devices

    cout << comPort << endl;

    if (comPort == INVALID_HANDLE_VALUE)
        cout << "Error in opening serial port" << endl;
    else
        cout << "opening serial port successful" << endl;

    DCB dcb       = {0};  // Device-control block used to configure serial communications
    dcb.DCBlength = sizeof(DCB);
    GetCommState(comPort, &dcb);
    dcb.BaudRate = CBR_115200;  // Set baud rate to 115200
    dcb.ByteSize = 8;
    SetCommState(comPort, &dcb);

    // Set up a read timeout
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(comPort, &timeouts);
    timeouts.ReadIntervalTimeout = 5;
    SetCommTimeouts(comPort, &timeouts);

    cout << "Testing uavcan_node" << endl;
    uint32_t warningEnableBitmask = getWarningEnableBitmask();
    cout << "warning_enable_bitmask: " << to_string(warningEnableBitmask) << endl;

    uint32_t errorEnableBitmask = getErrorEnableBitmask();
    cout << "error_enable_bitmask: " << to_string(errorEnableBitmask) << endl;

    uint32_t criticalEnableBitmask = getCriticalEnableBitmask();
    cout << "critical_enable_bitmask: " << to_string(criticalEnableBitmask) << endl;

    uint32_t errorCountConfiguration = getErrorCountConfiguration();
    cout << "error_count_configuration: " << to_string(errorCountConfiguration) << endl;

    cout << "Testing throttle_source_manager" << endl;
    uint8_t currentActiveThrottleSource = getCurrentActiveThrottleSource();
    cout << "current_active_throttle_source: " << to_string(currentActiveThrottleSource) << endl;


    return 0;
}
