/*
 * This example C++ program uses the Power Monitor client to retrieve the Voltage of a motor.
 * The serial port setup for windows is based on the example provided by microsoft:
 *      https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
 *
 * Name: fortiq_test.cpp
 * Last update: 2023/04/18 by Ben Quan
 * Author: Ben Quan and Jordan Leiber
 */

#include <tchar.h>
#include <windows.h>

#include <iostream>

#include "../inc/adc_interface_client.hpp"
#include "../inc/client_communication.cpp"
#include "../inc/client_communication.hpp"
#include "../inc/generic_interface.hpp"
#include "../inc/gpio_controller_client.hpp"
#include "../inc/power_monitor_client.hpp"
#include "../inc/pwm_interface_client.hpp"

using namespace std;

HANDLE com_port;                   // Handler for COM port
TCHAR *pcCommPort = TEXT("COM3");  // Change COM4 to whichever port your motor is connected to
GenericInterface com;              // Interface used by com port to communicate with motor

PowerMonitorClient power(0);  // Client endpoint for voltage reading
GpioControllerClient gpio(0);
PwmInterfaceClient pwm(0);
AdcInterfaceClient adc(0);

//  Send out any message data we have over the serial interface
int handle_com_tx() {
    uint8_t packet_buf[64];
    uint8_t length = 0;
    DWORD bytes_written;

    // Get the packet from the com interface and place it into the packet buffer
    if (com.GetTxBytes(packet_buf, length)) {
        WriteFile(com_port, packet_buf, length, &bytes_written, NULL);
    }

    return bytes_written;
}

// Grab any received data on the serial interface
int handle_com_rx() {
    uint8_t recv_bytes[64];
    DWORD dwBytesReceived;

    ReadFile(com_port, &recv_bytes, 64, &dwBytesReceived, 0);
    com.SetRxBytes(recv_bytes, dwBytesReceived);

    return dwBytesReceived;
}

// Hand off any received data to each module so they can handle it
void update_modules() {
    // Temporary Pointer to the packet data location
    uint8_t *packet_data;
    uint8_t packet_length;

    // Loads the packet data buffer with data receieved from the motor
    while (com.PeekPacket(&packet_data, &packet_length)) {
        power.ReadMsg(packet_data, packet_length);
        gpio.ReadMsg(packet_data, packet_length);
        pwm.ReadMsg(packet_data, packet_length);
        adc.ReadMsg(packet_data, packet_length);
        com.DropPacket();
    }
}

// Handles communication with motor
void send_message_and_process_reply() {
    handle_com_tx();
    handle_com_rx();
    update_modules();
}

// Sends the command to motor to get Voltage
float get_voltage() {
    power.volts_.get(com);
    send_message_and_process_reply();
    return power.volts_.get_reply();
}

void set_gpio_outputs() {}

int main() {
    com_port = CreateFile(pcCommPort, GENERIC_READ | GENERIC_WRITE,
                          0,              //  must be opened with exclusive-access
                          NULL,           //  default security attributes
                          OPEN_EXISTING,  //  must use OPEN_EXISTING
                          0,              //  not overlapped I/O
                          NULL);          //  hTemplate must be NULL for comm devices

    cout << com_port << endl;

    if (com_port == INVALID_HANDLE_VALUE)
        cout << "Error in opening serial port" << endl;
    else
        cout << "opening serial port successful" << endl;

    DCB dcb       = {0};  // Device-control block used to configure serial communications
    dcb.DCBlength = sizeof(DCB);
    GetCommState(com_port, &dcb);
    dcb.BaudRate = CBR_115200;  // Set baud rate to 115200
    dcb.ByteSize = 8;
    SetCommState(com_port, &dcb);

    // Set up a read timeout
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(com_port, &timeouts);
    timeouts.ReadIntervalTimeout = 5;
    SetCommTimeouts(com_port, &timeouts);

    // Get and print the current Voltage reading of the motor
    float current_voltage = get_voltage();
    cout << "voltage: " << to_string(current_voltage) << endl;

    gpio.outputs_register_.set(com, 2);
    gpio.outputs_register_.get(com);
    send_message_and_process_reply();
    cout << "gpio outputs: " << to_string(gpio.outputs_register_.get_reply()) << endl;

    pwm.duty_cycle_.set(com, 50);
    pwm.duty_cycle_.get(com);
    send_message_and_process_reply();
    cout << "duty cycle: " << to_string(pwm.duty_cycle_.get_reply()) << endl;

    gpio.addressable_outputs_.set(com, 131);

    adc.adc_voltage_.get(com);
    send_message_and_process_reply();
    cout << "adc volts: " << to_string(adc.adc_voltage_.get_reply()) << endl;

    return 0;
}