/*
  Copyright 2019 IQinetics Technologies, Inc support@iq-control.com

  This file is part of the IQ C++ API.

  This code is licensed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT for details)
*/

/*
  Name: hobby_input_client.hpp
  Last update: 3/7/2019 by Raphael Van Hoffelen
  Author: Matthew Piccoli
  Contributors: Raphael Van Hoffelen
*/

#ifndef HOBBY_INPUT_CLIENT_HPP_
#define HOBBY_INPUT_CLIENT_HPP_

#include "client_communication.hpp"

const uint8_t kTypeHobbyInput = 76;

class HobbyInputClient: public ClientAbstract{
  public:
    HobbyInputClient(uint8_t obj_idn):
      ClientAbstract(     kTypeHobbyInput, obj_idn),
      allowed_protocols_( kTypeHobbyInput, obj_idn, kSubAllowedProtocols),
      protocol_(          kTypeHobbyInput, obj_idn, kSubProtocol)
      {};

    // Client Entries
    // Control commands
    ClientEntry<uint8_t>    allowed_protocols_;
    ClientEntry<uint8_t>    protocol_;

    void ReadMsg(uint8_t* rx_data, uint8_t rx_length)
    {
      static const uint8_t kEntryLength = kSubProtocol+1;
      ClientEntryAbstract* entry_array[kEntryLength] = {
        &allowed_protocols_,  // 0
        &protocol_            // 1
      };

      ParseMsg(rx_data, rx_length, entry_array, kEntryLength);
    }

  private:
    static const uint8_t kSubAllowedProtocols =  0;
    static const uint8_t kSubProtocol         =  1;
};

#endif /* HOBBY_INPUT_CLIENT_HPP_ */
