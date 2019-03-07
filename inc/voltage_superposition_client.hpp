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
  Name: voltage_superposition_client.hpp
  Last update: 3/7/2019 by Raphael Van Hoffelen
  Author: Matthew Piccoli
  Contributors: Raphael Van Hoffelen
*/

#ifndef VOLTAGE_SUPERPOSITION_CLIENT_HPP_
#define VOLTAGE_SUPERPOSITION_CLIENT_HPP_

#include "client_communication.hpp"

const uint8_t kTypeVoltageSuperposition = 74;

class VoltageSuperPositionClient: public ClientAbstract{
	public:
		VoltageSuperPositionClient(uint8_t obj_idn):
			ClientAbstract(  kTypeVoltageSuperposition, obj_idn),
			zero_angle_(     kTypeVoltageSuperposition, obj_idn, kSubZeroAngle),
			frequency_(      kTypeVoltageSuperposition, obj_idn, kSubFrequency),
			phase_(          kTypeVoltageSuperposition, obj_idn, kSubPhase),
			amplitude_(      kTypeVoltageSuperposition, obj_idn, kSubAmplitude),
			voltage_(        kTypeVoltageSuperposition, obj_idn, kSubVoltage)
			{};

		// Client Entries
		ClientEntry<float>    zero_angle_;
		ClientEntry<uint8_t>  frequency_;
		ClientEntry<float>    phase_;
		ClientEntry<float>    amplitude_;
		ClientEntry<float>    voltage_;

		void ReadMsg(uint8_t* rx_data, uint8_t rx_length)
    {
      static const uint8_t kEntryLength = kSubVoltage+1;
      ClientEntryAbstract* entry_array[kEntryLength] = {
        &zero_angle_,        // 0
        &frequency_,       	 // 1
        &phase_,       			 // 2
        &amplitude_,         // 3
        &voltage_,      		 // 4
      };

      ParseMsg(rx_data, rx_length, entry_array, kEntryLength);
    }

 private:
		static const uint8_t kSubZeroAngle = 0;
		static const uint8_t kSubFrequency = 1;
		static const uint8_t kSubPhase     = 2;
		static const uint8_t kSubAmplitude = 3;
		static const uint8_t kSubVoltage   = 4;
};

#endif // VOLTAGE_SUPERPOSITION_CLIENT_HPP_