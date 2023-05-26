/*
 * The serial port setup for windows is based on the example provided by microsoft:
 *      https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
 *
 * Name: 2306_pulsing_test.cpp
 * Last update: 2023/04/19 by Ben Quan
 * Author: Ben Quan
 */

#include <tchar.h>
#include <windows.h>

#include <iostream>
// #include <list>

#include "../inc/brushless_drive_client.hpp"
#include "../inc/client_communication.cpp"
#include "../inc/coil_temperature_estimator_client.hpp"
#include "../inc/esc_propeller_input_parser_client.hpp"
#include "../inc/generic_interface.hpp"
#include "../inc/iquart_flight_controller_interface_client.hpp"
#include "../inc/persistent_memory_client.hpp"
#include "../inc/power_monitor_client.hpp"
#include "../inc/power_safety_client.hpp"
#include "../inc/propeller_motor_control_client.hpp"
#include "../inc/pulsing_rectangular_input_parser_client.hpp"
#include "../inc/serial_interface_client.hpp"
#include "../inc/system_control_client.hpp"
#include "../inc/temperature_estimator_client.hpp"
#include "../inc/temperature_monitor_uc_client.hpp"
#include "../inc/voltage_superposition_client.hpp"

using namespace std;

HANDLE comPort;                    // Handler for COM port
TCHAR *pcCommPort = TEXT("COM4");  // Change COM4 to whichever port your motor is connected to
GenericInterface com;              // Interface used by com port to communicate with motor

BrushlessDriveClient brushlessDrive(0);                      // Initialize Brushless Drive Client
CoilTemperatureEstimatorClient coilTemperatureEstimator(0);  // Initialize Coil Temperature Estimator Client
EscPropellerInputParserClient escPropellerInputParser(0);    // Initialize Esc Propeller Input Parser Client
IQUartFlightControllerInterfaceClient iquartFlightControllerInterface(
    0);                                                // Initialize IQUart Flight Controller Interface Client
PersistentMemoryClient persistentMemory(0);            // Initialize Persistent Memory Client
PowerMonitorClient powerMonitor(0);                    // Initialize Power Monitor Client
PowerSafetyClient powerSafety(0);                      // Initialize Power Safety Client
PropellerMotorControlClient propellerMotorControl(0);  // Initialize Propeller Motor Control Client
PulsingRectangularInputParserClient pulsingRectangularInputParser(
    0);                                              // Initialize Pulsing Rectangulat Input Parser Client
SerialInterfaceClient serialInterface(0);            // Initialize Serial Interface Client
SystemControlClient systemControl(0);                // Initialize System Control Client
TemperatureEstimatorClient temperatureEstimator(0);  // Initialize Temperature Estimator Client
TemperatureMonitorUcClient temperatureMonitorUc(0);  // Initialize Temperature Monitor Uc Client
VoltageSuperPositionClient voltageSuperposition(0);  // Initialize Voltage Superposition Client

// Initialize clientList to make it easier to call ReadMsg for each client
ClientAbstract *clientList[14] = {&brushlessDrive,
                                  &coilTemperatureEstimator,
                                  &escPropellerInputParser,
                                  &iquartFlightControllerInterface,
                                  &persistentMemory,
                                  &powerMonitor,
                                  &powerSafety,
                                  &propellerMotorControl,
                                  &pulsingRectangularInputParser,
                                  &serialInterface,
                                  &systemControl,
                                  &temperatureEstimator,
                                  &temperatureMonitorUc,
                                  &voltageSuperposition};

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

float getTCoil() {
    coilTemperatureEstimator.t_coil_.get(com);
    sendMessageAndProcessReply();
    return coilTemperatureEstimator.t_coil_.get_reply();
}

float getZeroSpinThrottle() {
    escPropellerInputParser.zero_spin_throttle_.get(com);
    sendMessageAndProcessReply();
    return escPropellerInputParser.zero_spin_throttle_.get_reply();
}

IFCITelemetryData getTelemetry() {
    iquartFlightControllerInterface.telemetry_.get(com);
    sendMessageAndProcessReply();
    IFCITelemetryData tele = iquartFlightControllerInterface.telemetry_.get_reply();
    return tele;
}

uint32_t getFormatKey1() {
    persistentMemory.format_key_1_.get(com);
    sendMessageAndProcessReply();
    return persistentMemory.format_key_1_.get_reply();
}

float getVolts() {
    powerMonitor.volts_.get(com);
    sendMessageAndProcessReply();
    return powerMonitor.volts_.get_reply();
}

float getVoltInputLow() {
    powerSafety.volt_input_low_.get(com);
    sendMessageAndProcessReply();
    return powerSafety.volt_input_low_.get_reply();
}

uint8_t getTimeoutSongOption() {
    propellerMotorControl.timeout_song_option_.get(com);
    sendMessageAndProcessReply();
    return propellerMotorControl.timeout_song_option_.get_reply();
}

uint8_t getPulsingVoltageMode() {
    pulsingRectangularInputParser.pulsing_voltage_mode_.get(com);
    sendMessageAndProcessReply();
    return pulsingRectangularInputParser.pulsing_voltage_mode_.get_reply();
}

uint32_t getBaudRate() {
    serialInterface.baud_rate_.get(com);
    sendMessageAndProcessReply();
    return serialInterface.baud_rate_.get_reply();
}

uint32_t getUid1() {
    systemControl.uid1_.get(com);
    sendMessageAndProcessReply();
    return systemControl.uid1_.get_reply();
}

uint32_t getControlFlags() {
    systemControl.control_flags_.get(com);
    sendMessageAndProcessReply();
    return systemControl.control_flags_.get_reply();
}

float getTemp() {
    temperatureEstimator.temp_.get(com);
    sendMessageAndProcessReply();
    return temperatureEstimator.temp_.get_reply();
}

float getUcTemp() {
    temperatureMonitorUc.uc_temp_.get(com);
    sendMessageAndProcessReply();
    return temperatureMonitorUc.uc_temp_.get_reply();
}

uint8_t getFrequency() {
    voltageSuperposition.frequency_.get(com);
    sendMessageAndProcessReply();
    return voltageSuperposition.frequency_.get_reply();
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

    float tCoil = getTCoil();
    cout << "t_coil: " << to_string(tCoil) << endl;

    float zeroSpinThrottle = getZeroSpinThrottle();
    cout << "zero spin throttle: " << to_string(zeroSpinThrottle) << endl;

    IFCITelemetryData telemetry = getTelemetry();
    cout << "telemetry coil_temp: " << telemetry.coil_temp << endl;
    cout << "telemetry consumption: " << telemetry.consumption << endl;
    cout << "telemetry current: " << telemetry.current << endl;
    cout << "telemetry mcu temp: " << telemetry.mcu_temp << endl;
    cout << "telemetry speed: " << telemetry.speed << endl;
    cout << "telemetry uptime: " << telemetry.uptime << endl;
    cout << "telemetry voltage: " << telemetry.voltage << endl;

    uint32_t formatKey1 = getFormatKey1();
    cout << "format key 1: " << to_string(formatKey1) << endl;

    float volts = getVolts();
    cout << "volts: " << to_string(volts) << endl;

    float voltInputLow = getVoltInputLow();
    cout << "volt input low: " << to_string(voltInputLow) << endl;

    uint8_t timeoutSongOption = getTimeoutSongOption();
    cout << "timeout song option: " << to_string(timeoutSongOption) << endl;

    uint8_t pulsingVoltageMode = getPulsingVoltageMode();
    cout << "pulsing voltage mode: " << to_string(pulsingVoltageMode) << endl;

    uint32_t baudRate = getBaudRate();
    cout << "baud rate: " << to_string(baudRate) << endl;

    uint32_t uid1 = getUid1();
    cout << "uid1: " << to_string(uid1) << endl;

    uint32_t controlFlags = getControlFlags();
    cout << "control flags: " << to_string(controlFlags) << endl;

    // float temp = getTemp();
    // cout << "temp: " << to_string(temp) << endl;

    float ucTemp = getUcTemp();
    cout << "uc temp: " << to_string(ucTemp) << endl;

    uint8_t frequency = getFrequency();
    cout << "frequency: " << to_string(frequency) << endl;

    return 0;
}