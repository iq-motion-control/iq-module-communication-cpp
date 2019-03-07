/*
  Copyright 2019 IQinetics Technologies, Inc support@iq-control.com

  This file is part of the IQ C++ API.

  IQ C++ API is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  IQ C++ API is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/*
  Name: buffered_voltage_monitor_client.hpp
  Last update: 3/7/2019 by Raphael Van Hoffelen
  Author: Matthew Piccoli
  Contributors: Raphael Van Hoffelen
*/

#ifndef BUFFERED_VOLTAGE_MONITOR_CLIENT_H
#define BUFFERED_VOLTAGE_MONITOR_CLIENT_H

#include "client_communication.hpp"

const uint8_t kTypeVoltageMonitor = 42;

class BufferedVoltageMonitorClient: public ClientAbstract{
  public:
    BufferedVoltageMonitorClient(uint8_t obj_idn):
      ClientAbstract( kTypeVoltageMonitor, obj_idn),
      volts_raw_(     kTypeVoltageMonitor, obj_idn, kSubVoltsRaw),
      volts_(         kTypeVoltageMonitor, obj_idn, kSubVolts),
      volts_gain_(    kTypeVoltageMonitor, obj_idn, kSubVoltsGain),
      filter_fs_(     kTypeVoltageMonitor, obj_idn, kSubFilterFs),
      filter_fc_(     kTypeVoltageMonitor, obj_idn, kSubFilterFc)
      {};

    // Client Entries
    ClientEntry<float>    volts_raw_;
    ClientEntry<float>    volts_;
    ClientEntry<float>    volts_gain_;
    ClientEntry<uint32_t> filter_fs_;
    ClientEntry<uint32_t> filter_fc_;

    void ReadMsg(uint8_t* rx_data, uint8_t rx_length)
    {
      static const uint8_t kEntryLength = kSubFilterFc+1;
      ClientEntryAbstract* entry_array[kEntryLength] = {
        &volts_raw_,  // 0
        nullptr,      // 1
        &volts_,      // 2
        nullptr,      // 3
        nullptr,      // 4
        nullptr,      // 5
        nullptr,      // 6
        nullptr,      // 7
        nullptr,      // 8
        nullptr,      // 9
        nullptr,      // 10
        &volts_gain_, // 11
        nullptr,      // 12
        nullptr,      // 13
        nullptr,      // 14
        nullptr,      // 15
        &filter_fs_,  // 16
        &filter_fc_   // 17
      };

      ParseMsg(rx_data, rx_length, entry_array, kEntryLength);
    }

  private:
    static const uint8_t kSubVoltsRaw  = 0;
    static const uint8_t kSubVolts     = 2;
    static const uint8_t kSubVoltsGain = 11;
    static const uint8_t kSubFilterFs  = 16;
    static const uint8_t kSubFilterFc  = 17;
};

#endif // BUFFERED_VOLTAGE_MONITOR_CLIENT_H
