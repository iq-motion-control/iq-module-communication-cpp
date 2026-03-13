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

#include "../inc/client_communication.cpp"
#include "../inc/generic_interface.hpp"

#include "../inc/esc_propeller_input_parser_client.hpp"
#include "../inc/propeller_motor_control_client.hpp"

using namespace std;

HANDLE comPort;                    // Handler for COM port
const TCHAR *pcCommPort = "COM4";  // Change COM4 to whichever port your motor is connected to
GenericInterface com;              // Interface used by com port to communicate with motor

EscPropellerInputParserClient escPropellerInputParser(0);    // Initialize ESC Propeller Input Parser Client
PropellerMotorControlClient propellerMotorControl(0);        // Initialize Propeller Motor Control Client

// Initialize clientList to make it easier to call ReadMsg for each client
ClientAbstract *clientList[2] = {&escPropellerInputParser,
                                 &propellerMotorControl,
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

float getTorqueMax() {
    escPropellerInputParser.torque_max_.get(com);
    sendMessageAndProcessReply();
    return escPropellerInputParser.torque_max_.get_reply();
}
void setTorqueMax(float newValue) {
    escPropellerInputParser.torque_max_.set(com, newValue);
    handleComTx();
}

float getCtrlTorque() {
    propellerMotorControl.ctrl_torque_.get(com);
    sendMessageAndProcessReply();
    return propellerMotorControl.ctrl_torque_.get_reply();
}
void setCtrlTorque(float newValue) {
    propellerMotorControl.ctrl_torque_.set(com, newValue);
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

    cout << "--------Testing esc_propeller_input_parser--------" << endl;
    float torqueMax = getTorqueMax();
    cout << "default value torque_max: " << to_string(torqueMax) << endl;
    float newTorqueMaxValue = 0.34;
    cout << "setting torque_max: " << to_string(newTorqueMaxValue) << endl;
    setTorqueMax(newTorqueMaxValue);
    float newTorqueMax = getTorqueMax();
    cout << "after setting torque_max: " << to_string(newTorqueMax) << endl;
    cout << "\n" << endl;

    cout << "-----Testing propeller_motor_control-----" << endl;
    float ctrlTorque = getCtrlTorque();
    cout << "default value ctrl_torque: " << to_string(ctrlTorque) << endl;
    float newCtrlTorqueValue = 0.10;
    cout << "setting ctrl_torque: " << to_string(newCtrlTorqueValue) << endl;
    setCtrlTorque(newCtrlTorqueValue);
    float newCtrlTorque = getCtrlTorque();
    cout << "after setting ctrl_torque: " << to_string(newCtrlTorque) << endl;
    cout << "\n" << endl;

    cout << "\n" << endl;
    cout << "--------Testing Complete--------" << endl;

    return 0;
}
