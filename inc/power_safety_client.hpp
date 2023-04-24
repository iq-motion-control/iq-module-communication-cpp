/*
  Copyright 2023 IQinetics Technologies, Inc support@iq-control.com

  This file is part of the IQ C++ API.

  This code is licensed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT for details)
*/

/*
  Name: power_safety_client.hpp
  Last update: 2023/04/12 by Ben Quan
  Author: Ben Quan
  Contributors:
*/

#ifndef POWER_SAFETY_CLIENT_HPP_
#define POWER_SAFETY_CLIENT_HPP_

#include "client_communication.hpp"

const uint8_t kTypePowerSafety = 84;

class PowerSafetyClient : public ClientAbstract {
   public:
    PowerSafetyClient(uint8_t obj_idn)
        : ClientAbstract(kTypePowerSafety, obj_idn),
          fault_now_(kTypePowerSafety, obj_idn, kSubFaultNow),
          fault_ever_(kTypePowerSafety, obj_idn, kSubFaultEver),
          fault_latching_(kTypePowerSafety, obj_idn, kSubFaultLatching),
          low1_(kTypePowerSafety, obj_idn, kSubVoltInputLow),
          high1_(kTypePowerSafety, obj_idn, kSubVoltInputHigh),
          low2_(kTypePowerSafety, obj_idn, kSubVrefIntLow),
          high2_(kTypePowerSafety, obj_idn, kSubVrefIntHigh),
          low3_(kTypePowerSafety, obj_idn, kSubCurrentInputLow),
          high3_(kTypePowerSafety, obj_idn, kSubCurrentInputHigh),
          low4_(kTypePowerSafety, obj_idn, kSubMotorCurrentLow),
          high4_(kTypePowerSafety, obj_idn, kSubMotorCurrentHigh),
          low5_(kTypePowerSafety, obj_idn, kSubTemperatureUcLow),
          high5_(kTypePowerSafety, obj_idn, kSubTemperatureUcHigh),
          low6_(kTypePowerSafety, obj_idn, kSubTemperatureCoilLow),
          high6_(kTypePowerSafety, obj_idn, kSubTemperatureCoilHigh){};

    // Client Entries
    ClientEntry<float> fault_now_;
    ClientEntry<float> fault_ever_;
    ClientEntry<float> fault_latching_;
    ClientEntry<float> low1_;
    ClientEntry<float> high1_;
    ClientEntry<float> low2_;
    ClientEntry<float> high2_;
    ClientEntry<float> low3_;
    ClientEntry<float> high3_;
    ClientEntry<float> low4_;
    ClientEntry<float> high4_;
    ClientEntry<float> low5_;
    ClientEntry<float> high5_;
    ClientEntry<float> low6_;
    ClientEntry<float> high6_;

    void ReadMsg(uint8_t* rx_data, uint8_t rx_length) {
        static const uint8_t kEntryLength              = kSubTemperatureCoilHigh + 1;
        ClientEntryAbstract* entry_array[kEntryLength] = {
            &fault_now_,       // 0
            &fault_ever_,      // 1
            &fault_latching_,  // 2
            &low1_,            // 3
            &high1_,           // 4
            &low2_,            // 5
            &high2_,           // 6
            &low3_,            // 7
            &high3_,           // 8
            &low4_,            // 9
            &high4_,           // 10
            &low5_,            // 11
            &high5_,           // 12
            &low6_,            // 13
            &high6_,           // 14
        };
        ParseMsg(rx_data, rx_length, entry_array, kEntryLength);
    }

   private:
    static const uint8_t kSubFaultNow            = 0;
    static const uint8_t kSubFaultEver           = 1;
    static const uint8_t kSubFaultLatching       = 2;
    static const uint8_t kSubVoltInputLow        = 3;
    static const uint8_t kSubVoltInputHigh       = 4;
    static const uint8_t kSubVrefIntLow          = 5;
    static const uint8_t kSubVrefIntHigh         = 6;
    static const uint8_t kSubCurrentInputLow     = 7;
    static const uint8_t kSubCurrentInputHigh    = 8;
    static const uint8_t kSubMotorCurrentLow     = 9;
    static const uint8_t kSubMotorCurrentHigh    = 10;
    static const uint8_t kSubTemperatureUcLow    = 11;
    static const uint8_t kSubTemperatureUcHigh   = 12;
    static const uint8_t kSubTemperatureCoilLow  = 13;
    static const uint8_t kSubTemperatureCoilHigh = 14;
};

#endif /* POWER_SAFETY_CLIENT_HPP_ */