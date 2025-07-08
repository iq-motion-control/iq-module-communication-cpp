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
 
 #include "../inc/client_communication.cpp"
 #include "../inc/generic_interface.hpp"

 #include "../inc/arming_handler_client.hpp"
 #include "../inc/coil_temperature_estimator_client.hpp"
 #include "../inc/esc_propeller_input_parser_client.hpp"
 #include "../inc/propeller_motor_control_client.hpp"
 #include "../inc/stopping_handler_client.hpp"
 #include "../inc/power_monitor_client.hpp"
 #include "../inc/throttle_source_manager_client.hpp"

 #include "../inc/current_safeties_client.hpp"
 #include "../inc/drive_control_interface_client.hpp"
 #include "../inc/motor_driver_client.hpp"
 #include "../inc/motor_model_client.hpp"
 #include "../inc/rotor_angle_generator_client.hpp"
 #include "../inc/voltage_target_generator_client.hpp"
 
 
 using namespace std;
 
 HANDLE comPort;                    // Handler for COM port
 TCHAR *pcCommPort = TEXT("COM3");  // Change COM4 to whichever port your motor is connected to
 GenericInterface com;              // Interface used by com port to communicate with motor
 
 ArmingHandlerClient armingHandler(0);                        // Initialize Arming Handler Client
 StoppingHandlerClient stoppingHandler(0);                    // Initialize Stopping Handler Client
 PropellerMotorControlClient propellerMotorControl(0);        // Initialize Propeller Motor Control Client
 EscPropellerInputParserClient escPropellerInputParser(0);    // Initialize Esc Propeller Input Parser Client
 CoilTemperatureEstimatorClient coilTemperatureEstimator(0);  // Initialize Coil Temperature Estimator Client
 PowerMonitorClient powerMonitorClient(0);                    // Initialize Power Monitor Client
 ThrottleSourceManagerClient throttleSourceManager(0);        // Initialize Throttle Source Manager Client

 // Hyperdrive clients
 CurrentSafetiesClient currentSafeties(0);
 DriveControlInterfaceClient driveControlInterface(0);
 MotorDriverClient motorDriver(0);
 MotorModelClient motorModel(0);
 RotorAngleGeneratorClient rotorAngleGenerator(0);
 VoltageTargetGeneratorClient voltageTargetGenerator(0);
 
 
 // Initialize clientList to make it easier to call ReadMsg for each client
 ClientAbstract *clientList[13] = {&armingHandler,
                                  &stoppingHandler,
                                  &propellerMotorControl,
                                  &escPropellerInputParser,
                                  &coilTemperatureEstimator,
                                  &powerMonitorClient,
                                  &throttleSourceManager,
                                  &currentSafeties,
                                  &driveControlInterface,
                                  &motorDriver,
                                  &motorModel,
                                  &rotorAngleGenerator,
                                  &voltageTargetGenerator
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
 
 float getTCoil() {
     coilTemperatureEstimator.t_coil_.get(com);
     sendMessageAndProcessReply();
     return coilTemperatureEstimator.t_coil_.get_reply();
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

// Hyperdrive functions
float getFinalMaxCurrentDerate() {
    currentSafeties.final_max_current_derate_.get(com);
    sendMessageAndProcessReply();
    return currentSafeties.final_max_current_derate_.get_reply();
}

float getVoltageTarget() {
    driveControlInterface.voltage_target_.get(com);
    sendMessageAndProcessReply();
    return driveControlInterface.voltage_target_.get_reply();
}

float getRotorMagnitude() {
    motorDriver.rotor_magnitude_.get(com);
    sendMessageAndProcessReply();
    return motorDriver.rotor_magnitude_.get_reply();
}

float getMechanicalInductance() {
    motorModel.mechanical_inductance_.get(com);
    sendMessageAndProcessReply();
    return motorModel.mechanical_inductance_.get_reply();
}

float getRotorAngle() {
    rotorAngleGenerator.rotor_angle_.get(com);
    sendMessageAndProcessReply();
    return rotorAngleGenerator.rotor_angle_.get_reply();
}

float getQCurrent() {
    voltageTargetGenerator.q_current_.get(com);
    sendMessageAndProcessReply();
    return voltageTargetGenerator.q_current_.get_reply();
}

 
 int main() {
     comPort = CreateFile(pcCommPort, GENERIC_READ | GENERIC_WRITE,
                          0,              //  must be opened with exclusive-access
                          NULL,           //  default security attributes
                          OPEN_EXISTING,  //  must use OPEN_EXISTING
                          0,              //  not overlapped I/O
                          NULL);          //  hTemplate must be NULL for comm devices
 
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

     cout << "Starting tests\n" << endl;
 
 
     float armThrottleUpperLimit = getArmThrottleUpperLimit();
     cout << "arm throttle upper limit: " << to_string(armThrottleUpperLimit) << endl;
 
     uint32_t consecutiveDisarmingThrottlesToDisarm = getConsecutiveDisarmingThrottlesToDisarm();
     cout << "consecutive disarming throttles to disarm: " << to_string(consecutiveDisarmingThrottlesToDisarm) << endl;
 
     float stoppedSpeed = getStoppedSpeed();
     cout << "stopped speed: " << to_string(stoppedSpeed) << endl;
 
     float stoppedTime = getStoppedTime();
     cout << "stopped time: " << to_string(stoppedTime) << endl;
 
     uint8_t timeoutSongOption = getTimeoutSongOption();
     cout << "timeout song option: " << to_string(timeoutSongOption) << endl;
 
     float zeroSpinThrottle = getZeroSpinThrottle();
     cout << "zero spin throttle: " << to_string(zeroSpinThrottle) << endl;
 
     float tCoil = getTCoil();
     cout << "t_coil: " << to_string(tCoil) << endl;
 
     float volts_cascaded = getVoltsCascaded();
     cout << "volts cascaded: " << to_string(volts_cascaded) << endl;
 
     uint32_t volts_cascaded_filter_fc = getVoltsCascadedFilterFc();
     cout << "volts cascaded filter fc: " << to_string(volts_cascaded_filter_fc) << endl;
 
     float throttle_timeout = getThrottleTimeout();
     cout << "throttle_timeout: " << throttle_timeout << endl;
 
     float dronecan_priority = getDronecanPriority();
     cout << "dronecan_priority: " << to_string(dronecan_priority) << endl;


     cout << "\nTesting Hyperdrive Clients\n" << endl;

     float final_max_current_derate = getFinalMaxCurrentDerate();
     cout << "final_max_current_derate:" << to_string(final_max_current_derate) << endl;

     float voltage_target = getVoltageTarget();
     cout << "voltage_target:" << to_string(voltage_target) << endl;

     float rotor_magnitude = getRotorMagnitude();
     cout << "rotor_magnitude:" << to_string(rotor_magnitude) << endl;

     float mechanical_inductance = getMechanicalInductance();
     cout << "mechanical_inductance:" << to_string(mechanical_inductance) << endl;

     float rotor_angle = getRotorAngle();
     cout << "rotor_angle:" << to_string(rotor_angle) << endl;

     float q_current = getQCurrent();
     cout << "q_current:" << to_string(q_current) << endl;
 
     cout << "\nTesting finished" << endl;
     return 0;
 }