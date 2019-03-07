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
  Name: encoder_client.hpp
  Last update: 3/7/2019 by Raphael Van Hoffelen
  Author: Matthew Piccoli
  Contributors: Raphael Van Hoffelen
*/

#ifndef ENCODER_CLIENT_H
#define ENCODER_CLIENT_H

#include "client_communication.hpp"

const uint8_t kTypeEncoder = 53;

class EncoderClient: public ClientAbstract{
  public:
    EncoderClient(uint8_t obj_idn):
      ClientAbstract(     kTypeEncoder, obj_idn),
      zero_angle_(        kTypeEncoder, obj_idn, kSubZeroAngle),
      velocity_filter_fs_(kTypeEncoder, obj_idn, kSubVelocityFilterFs),
      velocity_filter_fc_(kTypeEncoder, obj_idn, kSubVelocityFilterFc),
      rev_(                kTypeEncoder, obj_idn, kSubRev),
      absolute_rev_(      kTypeEncoder, obj_idn, kSubAbsoluteRev),
      rad_(               kTypeEncoder, obj_idn, kSubRad),
      absolute_rad_(      kTypeEncoder, obj_idn, kSubAbsoluteRad),
      velocity_(          kTypeEncoder, obj_idn, kSubVelocity)
      {};

    // Client Entries
    ClientEntry<float>    zero_angle_;
    ClientEntry<uint32_t> velocity_filter_fs_;
    ClientEntry<uint32_t> velocity_filter_fc_;
    ClientEntry<uint32_t> rev_;
    ClientEntry<uint32_t> absolute_rev_;
    ClientEntry<float>    rad_;
    ClientEntry<float>    absolute_rad_;
    ClientEntry<float>    velocity_;

    void ReadMsg(uint8_t* rx_data, uint8_t rx_length)
    {
      static const uint8_t kEntryLength = kSubVelocity+1;
      ClientEntryAbstract* entry_array[kEntryLength] = {
        &zero_angle_,         // 0
        &velocity_filter_fs_, // 1
        &velocity_filter_fc_, // 2
        &rev_,                // 3
        &absolute_rev_,       // 4
        &rad_,                // 5
        &absolute_rad_,       // 6
        &velocity_            // 7
      };

      ParseMsg(rx_data, rx_length, entry_array, kEntryLength);
    }

  private:
    static const uint8_t kSubZeroAngle        = 0;
    static const uint8_t kSubVelocityFilterFs = 1;
    static const uint8_t kSubVelocityFilterFc = 2;
    static const uint8_t kSubRev              = 3;
    static const uint8_t kSubAbsoluteRev      = 4;
    static const uint8_t kSubRad              = 5;
    static const uint8_t kSubAbsoluteRad      = 6;
    static const uint8_t kSubVelocity         = 7;
};

#endif // ENCODER_CLIENT_H
