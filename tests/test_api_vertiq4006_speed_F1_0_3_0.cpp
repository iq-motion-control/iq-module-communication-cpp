/*
 * The serial port setup for windows is based on the example provided by microsoft:
 *      https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
 *
 * Name: test_api_vertiq4006_speed_F1_0_3_0.cpp
 * Last update: 2026/03/13 by Ben Quan
 * Author: Ben Quan
 */

#include <tchar.h>
#include <windows.h>

#include <iostream>

#include "../inc/brushless_drive_client.hpp"
#include "../inc/client_communication.cpp"
#include "../inc/generic_interface.hpp"
#include "../inc/multi_turn_angle_control_client.hpp"
#include "../inc/uavcan_node_client.hpp"
#include "../inc/throttle_source_manager_client.hpp"

using namespace std;

HANDLE comPort;                    // Handler for COM port
const TCHAR *pcCommPort = "COM4";  // Change COM4 to whichever port your motor is connected to
GenericInterface com;              // Interface used by com port to communicate with motor

BrushlessDriveClient brushlessDrive(0);                      // Initialize Brushless Drive Client
MultiTurnAngleControlClient multiTurnAngleControl(0);        // Initialize Multi Turn Angle Control Client
UavcanNodeClient uavcanNode(0);                              // Initialize UAVCAN Node Client
ThrottleSourceManagerClient throttleSourceManager(0);        // Initialize Throttle Source Manager Client

// Initialize clientList to make it easier to call ReadMsg for each client
ClientAbstract *clientList[4] = {&brushlessDrive,
                                 &multiTurnAngleControl,
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

void setWarningEnableBitmask(uint32_t newValue) {
    uavcanNode.warning_enable_bitmask_.set(com, newValue);
    handleComTx();
}

uint32_t getErrorEnableBitmask() {
    uavcanNode.error_enable_bitmask_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.error_enable_bitmask_.get_reply();
}

void setErrorEnableBitmask(uint32_t newValue) {
    uavcanNode.error_enable_bitmask_.set(com, newValue);
    handleComTx();
}

uint32_t getCriticalEnableBitmask() {
    uavcanNode.critical_enable_bitmask_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.critical_enable_bitmask_.get_reply();
}

void setCriticalEnableBitmask(uint32_t newValue) {
    uavcanNode.critical_enable_bitmask_.set(com, newValue);
    handleComTx();
}

uint32_t getErrorCountConfiguration() {
    uavcanNode.error_count_configuration_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.error_count_configuration_.get_reply();
}

void setErrorCountConfiguration(uint32_t newValue) {
    uavcanNode.error_count_configuration_.set(com, newValue);
    handleComTx();
}

uint8_t getCurrentActiveThrottleSource() {
    throttleSourceManager.current_active_throttle_source_.get(com);
    sendMessageAndProcessReply();
    return throttleSourceManager.current_active_throttle_source_.get_reply();
}

uint8_t getTrajectoryQueueLength(){
    multiTurnAngleControl.trajectory_queue_length_.get(com);
    sendMessageAndProcessReply();
    return multiTurnAngleControl.trajectory_queue_length_.get_reply();
}

uint32_t getFF(){
    multiTurnAngleControl.ff_.get(com);
    sendMessageAndProcessReply();
    return multiTurnAngleControl.ff_.get_reply();
}
void setFF(uint32_t newValue) {
    multiTurnAngleControl.ff_.set(com, newValue);
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

    cout << "--------Testing uavcan_node--------" << endl;
    uint32_t warningEnableBitmask = getWarningEnableBitmask();
    cout << "default value warning_enable_bitmask: " << to_string(warningEnableBitmask) << endl;
    uint32_t newWarningEnableBitmaskValue = 6;
    cout << "setting warning_enable_bitmask: " << to_string(newWarningEnableBitmaskValue) << endl;
    setWarningEnableBitmask(newWarningEnableBitmaskValue);
    uint32_t newWarningEnableBitmask = getWarningEnableBitmask();
    cout << "after setting warning_enable_bitmask: " << to_string(newWarningEnableBitmask) << endl;
    cout << "\n" << endl;

    uint32_t errorEnableBitmask = getErrorEnableBitmask();
    cout << "default value error_enable_bitmask: " << to_string(errorEnableBitmask) << endl;
    uint32_t newErrorEnableBitmaskValue = 2;
    cout << "setting error_enable_bitmask: " << to_string(newErrorEnableBitmaskValue) << endl;
    setErrorEnableBitmask(newErrorEnableBitmaskValue);
    uint32_t newErrorEnableBitmask = getErrorEnableBitmask();
    cout << "after setting error_enable_bitmask: " << to_string(newErrorEnableBitmask) << endl;
    cout << "\n" << endl;

    uint32_t criticalEnableBitmask = getCriticalEnableBitmask();
    cout << "default value critical_enable_bitmask: " << to_string(criticalEnableBitmask) << endl;
    uint32_t newCriticalEnableBitmaskValue = 2;
    cout << "setting critical_enable_bitmask: " << to_string(newCriticalEnableBitmaskValue) << endl;
    setCriticalEnableBitmask(newCriticalEnableBitmaskValue);
    uint32_t newCriticalEnableBitmask = getCriticalEnableBitmask();
    cout << "after setting critical_enable_bitmask: " << to_string(newCriticalEnableBitmask) << endl;
    cout << "\n" << endl;


    uint32_t errorCountConfiguration = getErrorCountConfiguration();
    cout << "default value error_count_configuration: " << to_string(errorCountConfiguration) << endl;
    uint32_t newErrorCountConfigurationValue = 2;
    cout << "setting error_count_configuration: " << to_string(newErrorCountConfigurationValue) << endl;
    setErrorCountConfiguration(newErrorCountConfigurationValue);
    uint32_t newErrorCountConfiguration = getErrorCountConfiguration();
    cout << "after setting error_count_configuration: " << to_string(newErrorCountConfiguration) << endl;
    cout << "\n" << endl;

    cout << "-----Testing throttle_source_manager-----" << endl;
    uint8_t currentActiveThrottleSource = getCurrentActiveThrottleSource();
    cout << "current_active_throttle_source: " << to_string(currentActiveThrottleSource) << endl;
    cout << "\n" << endl;

    cout << "-----Testing multi_turn_angle_control-----" << endl;
    uint32_t ff = getFF();
    cout << "default value ff: " << to_string(ff) << endl;
    uint32_t newFFValue = 2;
    cout << "setting ff: " << to_string(newFFValue) << endl;
    setFF(newFFValue);
    uint32_t newFF = getFF();
    cout << "after setting ff: " << to_string(newFF) << endl;
    cout << "\n" << endl;

    uint8_t trajectoryQueueLength = getTrajectoryQueueLength();
    cout << "default value trajectory_queue_length: " << to_string(trajectoryQueueLength) << endl;
    cout << "\n" << endl;

    cout << "\n" << endl;
    cout << "--------Testing Complete--------" << endl;

    return 0;
}
