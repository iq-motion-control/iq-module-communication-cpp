/*
 * This example C++ program uses the Power Monitor client to retrieve the Voltage of a motor.
 * The serial port setup for windows is based on the example provided by microsoft:
 *      https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
 *
 * Name: 8108_speed_v0_0_7_test.cpp
 * Last update: 2023/04/18 by Ben Quan
 * Author: Ben Quan
 */

#include <tchar.h>
#include <windows.h>

#include <iostream>
// #include <list>

#include "../inc/arming_handler_client.hpp"
#include "../inc/brushless_drive_client.hpp"
#include "../inc/client_communication.cpp"
#include "../inc/coil_temperature_estimator_client.hpp"
#include "../inc/esc_propeller_input_parser_client.hpp"
#include "../inc/generic_interface.hpp"
#include "../inc/multi_turn_angle_control_client.hpp"
#include "../inc/propeller_motor_control_client.hpp"
#include "../inc/stopping_handler_client.hpp"
#include "../inc/stow_user_interface_client.hpp"
#include "../inc/uavcan_node_client.hpp"
#include "../inc/power_monitor_client.hpp"
#include "../inc/throttle_source_manager_client.hpp"

using namespace std;

HANDLE comPort;                    // Handler for COM port
TCHAR *pcCommPort = TEXT("COM3");  // Change COM4 to whichever port your motor is connected to
GenericInterface com;              // Interface used by com port to communicate with motor

BrushlessDriveClient brushlessDrive(0);                      // Initialize Brushless Drive Client
ArmingHandlerClient armingHandler(0);                        // Initialize Arming Handler Client
StoppingHandlerClient stoppingHandler(0);                    // Initialize Stopping Handler Client
StowUserInterfaceClient stowUserInterface(0);                // Initialize Stow User Interface Client
MultiTurnAngleControlClient multiTurnAngleControl(0);        // Initialize Mult Turn Angle Control Client
PropellerMotorControlClient propellerMotorControl(0);        // Initialize Propeller Motor Control Client
EscPropellerInputParserClient escPropellerInputParser(0);    // Initialize Esc Propeller Input Parser Client
CoilTemperatureEstimatorClient coilTemperatureEstimator(0);  // Initialize Coil Temperature Estimator Client
UavcanNodeClient uavcanNode(0);                              // Initialize UAVCAN Node Client
PowerMonitorClient powerMonitorClient(0);                    // Initialize Power Monitor Client
ThrottleSourceManagerClient throttleSourceManager(0);        // Initialize Throttle Source Manager Client

// Initialize clientList to make it easier to call ReadMsg for each client
ClientAbstract *clientList[11] = {&brushlessDrive,
                                 &armingHandler,
                                 &stoppingHandler,
                                 &stowUserInterface,
                                 &multiTurnAngleControl,
                                 &propellerMotorControl,
                                 &escPropellerInputParser,
                                 &coilTemperatureEstimator,
                                 &uavcanNode,
                                 &powerMonitorClient,
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

uint8_t getDriveMode() {
    brushlessDrive.drive_mode_.get(com);
    sendMessageAndProcessReply();
    return brushlessDrive.drive_mode_.get_reply();
}

float getVMaxStart() {
    brushlessDrive.v_max_start_.get(com);
    sendMessageAndProcessReply();
    return brushlessDrive.v_max_start_.get_reply();
}

uint32_t getDerateLowPassFilterFc() {
    brushlessDrive.derate_low_pass_filter_fc_.get(com);
    sendMessageAndProcessReply();
    return brushlessDrive.derate_low_pass_filter_fc_.get_reply();
}

uint32_t getDerateLowPassFilterFs() {
    brushlessDrive.derate_low_pass_filter_fs_.get(com);
    sendMessageAndProcessReply();
    return brushlessDrive.derate_low_pass_filter_fs_.get_reply();
}

// Sends the command to motor to get Voltage
float getArmThrottleUpperLimit() {
    armingHandler.arm_throttle_upper_limit_.get(com);
    sendMessageAndProcessReply();
    return armingHandler.arm_throttle_upper_limit_.get_reply();
}

uint32_t getConsecutiveDisarmingThrottlesToDisarm() {
    armingHandler.consecutive_disarming_throttles_to_disarm_.get(com);
    sendMessageAndProcessReply();
    return armingHandler.consecutive_disarming_throttles_to_disarm_.get_reply();
}

float getStoppedSpeed() {
    stoppingHandler.stopped_speed_.get(com);
    sendMessageAndProcessReply();
    return stoppingHandler.stopped_speed_.get_reply();
}

float getStoppedTime() {
    stoppingHandler.stopped_time_.get(com);
    sendMessageAndProcessReply();
    return stoppingHandler.stopped_time_.get_reply();
}

float getStowKp() {
    stowUserInterface.stow_kp_.get(com);
    sendMessageAndProcessReply();
    return stowUserInterface.stow_kp_.get_reply();
}

void setStow() {
    stowUserInterface.stow_.set(com);
    sendMessageAndProcessReply();
}

float getAngleKd() {
    multiTurnAngleControl.angle_Kd_.get(com);
    sendMessageAndProcessReply();
    return multiTurnAngleControl.angle_Kd_.get_reply();
}

uint8_t getTimeoutSongOption() {
    propellerMotorControl.timeout_song_option_.get(com);
    sendMessageAndProcessReply();
    return propellerMotorControl.timeout_song_option_.get_reply();
}

float getZeroSpinThrottle() {
    escPropellerInputParser.zero_spin_throttle_.get(com);
    sendMessageAndProcessReply();
    return escPropellerInputParser.zero_spin_throttle_.get_reply();
}

uint8_t getReportTelemetryAsSpeed() {
    escPropellerInputParser.report_telemetry_as_speed_.get(com);
    sendMessageAndProcessReply();
    return escPropellerInputParser.report_telemetry_as_speed_.get_reply();
}

float getTCoil() {
    coilTemperatureEstimator.t_coil_.get(com);
    sendMessageAndProcessReply();
    return coilTemperatureEstimator.t_coil_.get_reply();
}

uint32_t getBitRate() {
    uavcanNode.bit_rate_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.bit_rate_.get_reply();
}

uint8_t getArmingByArmingStatus() {
    uavcanNode.arming_by_arming_status_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.arming_by_arming_status_.get_reply();
}

float getVoltsCascaded() {
    powerMonitorClient.volts_cascaded_.get(com);
    sendMessageAndProcessReply();
    return powerMonitorClient.volts_cascaded_.get_reply();
}

uint32_t getVoltsCascadedFilterFc() {
    powerMonitorClient.volts_cascaded_filter_fc_.get(com);
    sendMessageAndProcessReply();
    return powerMonitorClient.volts_cascaded_filter_fc_.get_reply();
}

float getThrottleTimeout() {
    throttleSourceManager.throttle_timeout_.get(com);
    sendMessageAndProcessReply();
    return throttleSourceManager.throttle_timeout_.get_reply();
}

uint8_t getDronecanPriority() {
    throttleSourceManager.dronecan_priority_.get(com);
    sendMessageAndProcessReply();
    return throttleSourceManager.dronecan_priority_.get_reply();
}

uint8_t getTelemetryStyle() {
    uavcanNode.telemetry_style_.get(com);
    sendMessageAndProcessReply();
    return uavcanNode.telemetry_style_.get_reply();
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

    uint8_t driveMode = getDriveMode();
    cout << "drive mode: " << to_string(driveMode) << endl;

    float vMaxStart = getVMaxStart();
    cout << "v max start: " << to_string(vMaxStart) << endl;

    // uint32_t derateLowPassFilterFc = getDerateLowPassFilterFc();
    // cout << "derate low pass filter fc: " << to_string(derateLowPassFilterFc) << endl;

    // uint32_t derateLowPassFilterFs = getDerateLowPassFilterFs();
    // cout << "derate low pass filter fs: " << to_string(derateLowPassFilterFs) << endl;

    float armThrottleUpperLimit = getArmThrottleUpperLimit();
    cout << "arm throttle upper limit: " << to_string(armThrottleUpperLimit) << endl;

    // uint32_t consecutiveDisarmingThrottlesToDisarm = getConsecutiveDisarmingThrottlesToDisarm();
    // cout << "consecutive disarming throttles to disarm: " << to_string(consecutiveDisarmingThrottlesToDisarm) << endl;

    float stoppedSpeed = getStoppedSpeed();
    cout << "stopped speed: " << to_string(stoppedSpeed) << endl;

    float stoppedTime = getStoppedTime();
    cout << "stopped time: " << to_string(stoppedTime) << endl;

    float stowKp = getStowKp();
    cout << "stow kp: " << to_string(stowKp) << endl;

    float angleKd = getAngleKd();
    cout << "angle kd: " << to_string(angleKd) << endl;

    uint8_t timeoutSongOption = getTimeoutSongOption();
    cout << "timeout song option: " << to_string(timeoutSongOption) << endl;

    float zeroSpinThrottle = getZeroSpinThrottle();
    cout << "zero spin throttle: " << to_string(zeroSpinThrottle) << endl;

    uint8_t reportTelemetryAsSpeed = getReportTelemetryAsSpeed();
    cout << "report telemetry as speed: " << to_string(reportTelemetryAsSpeed) << endl;

    float tCoil = getTCoil();
    cout << "t_coil: " << to_string(tCoil) << endl;

    uint32_t bitRate = getBitRate();
    cout << "bit rate: " << to_string(bitRate) << endl;

    uint8_t armingByArmingStatus = getArmingByArmingStatus();
    cout << "arming by arming status: " << to_string(armingByArmingStatus) << endl;

    float volts_cascaded = getVoltsCascaded();
    cout << "volts cascaded: " << to_string(volts_cascaded) << endl;

    uint32_t volts_cascaded_filter_fc = getVoltsCascadedFilterFc();
    cout << "volts cascaded filter fc: " << to_string(volts_cascaded_filter_fc) << endl;

    float throttle_timeout = getThrottleTimeout();
    cout << "throttle_timeout: " << throttle_timeout << endl;

    float dronecan_priority = getDronecanPriority();
    cout << "dronecan_priority: " << to_string(dronecan_priority) << endl;

    uint8_t telemetry_style = getTelemetryStyle();
    cout << "telemetry_style: " << to_string(telemetry_style) << endl;

    cout << "setting Stow now" << endl;
    setStow();

    return 0;
}