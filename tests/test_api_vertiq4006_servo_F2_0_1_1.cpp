/*
 * The serial port setup for windows is based on the example provided by microsoft:
 *      https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
 *
 * Name: test_api_vertiq4006_servo_F2_0_1_1.cpp
 * Last update: 2026/03/13 by Ben Quan
 * Author: Ben Quan
 */

#include <tchar.h>
#include <windows.h>

#include <iostream>

#include "../inc/brushless_drive_client.hpp"
#include "../inc/client_communication.cpp"
#include "../inc/generic_interface.hpp"
#include "../inc/iquart_flight_controller_interface_client.hpp"
#include "../inc/uavcan_node_client.hpp"

using namespace std;

HANDLE comPort;                    // Handler for COM port
const TCHAR *pcCommPort = "COM4";  // Change COM4 to whichever port your motor is connected to
GenericInterface com;              // Interface used by com port to communicate with motor

BrushlessDriveClient brushlessDrive(0);         // Initialize Brushless Drive Client with Module ID 0
IQUartFlightControllerInterfaceClient ifci(0);  // Initialize IFCI Client with Module ID 0
UavcanNodeClient uavcanNode(0);                 // Initialize UAVCAN Node Client with Module ID 0

// Initialize clientList to make it easier to call ReadMsg for each client
ClientAbstract *clientList[3] = {&brushlessDrive,
                                 &uavcanNode,
                                 &ifci
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

uint32_t getActuatorId() {
    uavcanNode.actuator_id_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.actuator_id_.get_reply();
}

void setActuatorId(uint32_t newValue) {
    uavcanNode.actuator_id_.set(com, newValue);
    handleComTx();
}

uint8_t getServoCvi() {
    ifci.servo_cvi_.get(com);
    sendMessageAndProcessReply();
    return ifci.servo_cvi_.get_reply();
}

void setServoCvi(uint32_t newValue) {
    ifci.servo_cvi_.set(com, newValue);
    handleComTx();
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
    cout << "\n" << endl;

    cout << "-----Testing iquart_flight_controller_interface-----" << endl;
    uint8_t servoCvi = getServoCvi();
    cout << "default value servo_cvi: " << to_string(servoCvi) << endl;
    uint8_t newServoCviValue = 6;
    cout << "setting servo_cvi: " << to_string(newServoCviValue) << endl;
    setServoCvi(newServoCviValue);
    uint8_t newServoCvi = getServoCvi();
    cout << "after setting servo_cvi: " << to_string(newServoCvi) << endl;
    cout << "\n" << endl;

    cout << "--------Testing uavcan_node--------" << endl;
    uint32_t actuatorId = getActuatorId();
    cout << "default value actuator_id: " << to_string(actuatorId) << endl;
    uint32_t newActuatorIdValue = 6;
    cout << "setting actuator_id: " << to_string(newActuatorIdValue) << endl;
    setActuatorId(newActuatorIdValue);
    uint32_t newActuatorId= getActuatorId();
    cout << "after setting actuator_id: " << to_string(newActuatorId) << endl;
    cout << "\n" << endl;

    cout << "\n" << endl;
    cout << "--------Testing Complete--------" << endl;

    return 0;
}
