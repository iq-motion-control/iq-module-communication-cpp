/*
 * This C++ script tests the updated client entries of various clients for firmware v1.0.2.0
 * The serial port setup for windows is based on the example provided by microsoft:
 *      https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
 *
 * Name: test_api_vertiq8108_speed_F1_0_2_0.cpp
 * Last update: 2025/08/26 by Ben Quan
 * Author: Ben Quan
 */

#include <tchar.h>
#include <windows.h>
#include <iostream>

#include "../inc/arming_handler_client.hpp"
#include "../inc/brushless_drive_client.hpp"
#include "../inc/client_communication.cpp"
#include "../inc/esc_propeller_input_parser_client.hpp"
#include "../inc/generic_interface.hpp"
#include "../inc/hobby_input_client.hpp"
#include "../inc/multi_turn_angle_control_client.hpp"
#include "../inc/propeller_motor_control_client.hpp"

using namespace std;

HANDLE comPort;                    // Handler for COM port
TCHAR *pcCommPort = TEXT("COM4");  // Change COM4 to whichever port your motor is connected to
GenericInterface com;              // Interface used by com port to communicate with motor

ArmingHandlerClient armingHandler(0);                        // Initialize Arming Handler Client
BrushlessDriveClient brushlessDrive(0);                      // Initialize Brushless Drive Client
EscPropellerInputParserClient escPropellerInputParser(0);    // Initialize Esc Propeller Input Parser Client
HobbyInputClient hobbyInput(0);
MultiTurnAngleControlClient multiTurnAngleControl(0);        // Initialize Mult Turn Angle Control Client
PropellerMotorControlClient propellerMotorControl(0);        // Initialize Propeller Motor Control Client

// Initialize clientList to make it easier to call ReadMsg for each client
ClientAbstract *clientList[6] = {  &armingHandler,
                                    &brushlessDrive,
                                    &escPropellerInputParser,
                                    &hobbyInput,
                                    &multiTurnAngleControl,
                                    &propellerMotorControl
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
void sendMessageAndProcessReply(){
    handleComTx();
    handleComRx();
    updateModules();
}

uint8_t getPlayArmingSongOnArm(){
    armingHandler.play_arming_song_on_arm_.get(com);
    sendMessageAndProcessReply();
    return armingHandler.play_arming_song_on_arm_.get_reply();
}

uint32_t getScalingCommutationCyclesPerErev(){
    brushlessDrive.scaling_commutation_cycles_per_erev_.get(com);
    sendMessageAndProcessReply();
    return brushlessDrive.scaling_commutation_cycles_per_erev_.get_reply();
}

uint32_t getScalingCommutationHzFloor(){
    brushlessDrive.scaling_commutation_hz_floor_.get(com);
    sendMessageAndProcessReply();
    return brushlessDrive.scaling_commutation_hz_floor_.get_reply();
}

uint32_t getScalingCommutationHzCeiling(){
    brushlessDrive.scaling_commutation_hz_ceiling_.get(com);
    sendMessageAndProcessReply();
    return brushlessDrive.scaling_commutation_hz_ceiling_.get_reply();
}

uint16_t getHobbyTelemetryFrequency(){
    hobbyInput.hobby_telemetry_frequency_.get(com);
    sendMessageAndProcessReply();
    return hobbyInput.hobby_telemetry_frequency_.get_reply();
}

uint8_t getHobbyTelemetrySpeedStyle(){
    hobbyInput.hobby_telemetry_speed_style_.get(com);
    sendMessageAndProcessReply();
    return hobbyInput.hobby_telemetry_speed_style_.get_reply();
}

uint8_t getAllowDshotDisarmingMessage(){
    hobbyInput.allow_dshot_disarming_message_.get(com);
    sendMessageAndProcessReply();
    return hobbyInput.allow_dshot_disarming_message_.get_reply();
}

float getLowPowerHoldAllowedTargetError(){
    multiTurnAngleControl.low_power_hold_allowed_target_error_.get(com);
    sendMessageAndProcessReply();
    return multiTurnAngleControl.low_power_hold_allowed_target_error_.get_reply();
}

float getLowPowerHoldMaxBrakeError(){
    multiTurnAngleControl.low_power_hold_max_brake_error_.get(com);
    sendMessageAndProcessReply();
    return multiTurnAngleControl.low_power_hold_max_brake_error_.get_reply();
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

    cout << "starting tests..." << endl;

    uint8_t playArmingSongOnArm = getPlayArmingSongOnArm();
    cout << "play_arming_song_on_arm: " << to_string(playArmingSongOnArm) << endl;

    uint16_t hobby_telemetry_frequency = getHobbyTelemetryFrequency();
    cout << "hobby_telemetry_frequency: " << to_string(hobby_telemetry_frequency) << endl;

    uint8_t hobby_telemetry_speed_style = getHobbyTelemetrySpeedStyle();
    cout << "hobby_telemetry_speed_style: " << to_string(hobby_telemetry_speed_style) << endl;

    uint8_t allow_dshot_disarming_message = getAllowDshotDisarmingMessage();
    cout << "allow_dshot_disarming_message: " << to_string(allow_dshot_disarming_message) << endl;

    float low_power_hold_allowed_target_error = getLowPowerHoldAllowedTargetError();
    cout << "low_power_hold_allowed_target_error: " << to_string(low_power_hold_allowed_target_error) << endl;

    float low_power_hold_max_brake_error = getLowPowerHoldMaxBrakeError();
    cout << "low_power_hold_max_brake_error: " << to_string(low_power_hold_max_brake_error) << endl;

    return 0;
}