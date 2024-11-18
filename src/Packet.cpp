/****************************************************************************
 *
 *   @copyright Copyright (c) 2024 Dave Hylands     <dhylands@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the MIT License version as described in the
 *   LICENSE file in the root of this repository.
 *
 ****************************************************************************/
/**
 *   @file   Packet.cpp
 *
 *   @brief  Parses bioloid packets.
 *
 ****************************************************************************/

#include "Packet.h"

#include <algorithm>
#include <cassert>
#include <cinttypes>

#include "Log.h"

//! @addtogroup bioloid
//! @{

namespace bioloid {

//! The maximum number of bytes allowed by the protocol.
static constexpr uint8_t MAX_PARAMS = 0xffu - 2u;

Packet::Packet() : m_state{State::IDLE}, m_maxParams{0}, m_params{nullptr} {}

Packet::Packet(size_t maxParams, void* params)
    : m_state{State::IDLE},
      m_maxParams(static_cast<uint8_t>(maxParams)),
      m_params{reinterpret_cast<uint8_t*>(params)} {
    assert(maxParams <= MAX_PARAMS);
}

Packet::~Packet() {}

void Packet::params(size_t numParams) {
    assert(numParams <= this->m_maxParams);
    this->m_length = 2u + std::min(static_cast<uint8_t>(numParams), this->m_maxParams);
}

void Packet::params(size_t numParams, const void* params) {
    assert(numParams <= this->m_maxParams);
    if (numParams >= this->m_maxParams) {
        numParams = this->m_maxParams;
    }
    memcpy(this->m_params, params, numParams);
    this->m_length = 2u + std::min(static_cast<uint8_t>(numParams), this->m_maxParams);
}

void Packet::update_checksum() {
    this->m_checksum = this->id();
    this->m_checksum += this->length();
    this->m_checksum += this->command();
    for (uint_fast8_t i = 0; i < this->numParams(); i++) {
        this->m_checksum += this->m_params[i];
    }
    this->m_checksum = ~this->m_checksum;
}

size_t Packet::data(size_t maxLen, void* void_data) {
    uint8_t* data = reinterpret_cast<uint8_t*>(void_data);
    size_t len = 0;
    if (len < maxLen) {
        data[len++] = 0xff;
    }
    if (len < maxLen) {
        data[len++] = 0xff;
    }
    if (len < maxLen) {
        data[len++] = this->id();
    }
    if (len < maxLen) {
        data[len++] = this->length();
    }
    if (len < maxLen) {
        data[len++] = this->command();
    }
    for (size_t idx = 0; idx < this->numParams(); idx++) {
        if (idx >= this->m_maxParams) {
            // This happens if we get the TOO_MUCH_DATA error.
            return len;
        }
        if (len < maxLen) {
            data[len++] = this->m_params[idx];
        } else {
            break;
        }
    }
    if (len < maxLen) {
        data[len++] = this->m_checksum;
    }
    return len;
}

Error::Type Packet::processByte(uint8_t byte) {
    State nextState = this->m_state;
    Error::Type err = Error::NOT_DONE;

    switch (nextState) {
        case State::IDLE: {  // We're waiting for the beginning of the packet (0xFF)
            if (byte == 0xFF) {
                nextState = State::FF_1ST_RCVD;
            }
            break;
        }

        case State::FF_1ST_RCVD: {  // We've received the 1st 0xFF
            if (byte == 0xFF) {
                nextState = State::FF_2ND_RCVD;
            } else {
                nextState = State::IDLE;
            }
            break;
        }

        case State::FF_2ND_RCVD: {  // We've received the 2nd 0xFF, ch is the ID
            if (byte == 0xFF) {
                // 0xFF is invalid as an ID, so just stay in this state until we receive
                // a non-0xFF
                nextState = State::FF_2ND_RCVD;
                break;
            }
            this->m_id = byte;
            this->m_checksum = byte;
            nextState = State::ID_RCVD;
            break;
        }

        case State::ID_RCVD: {  // We've received the ID, ch is the length
            this->m_length = byte;
            this->m_checksum += byte;
            nextState = State::LENGTH_RCVD;
            break;
        }

        case State::LENGTH_RCVD: {  // We've received the length, ch is the command
            this->m_cmd = byte;
            this->m_checksum += byte;
            this->m_paramIdx = 0;
            nextState = State::COMMAND_RCVD;
            break;
        }

            // NOTE: In the future, we should decode the SYNC_WRITE
            //       packet so that we only need to keep the portion that
            //       belongs to our ID

        case State::COMMAND_RCVD: {  // We've received the command, ch is a param byte or
                                     // checksum
            if (this->m_paramIdx >= this->numParams()) {
                // ch is the Checksum

                this->m_checksum = ~this->m_checksum;

                if (this->m_checksum == byte) {
                    if (this->m_paramIdx <= this->m_maxParams) {
                        err = Error::NONE;
                    } else {
                        err = Error::TOO_MUCH_DATA;
                    }
                } else {
                    // CRC failed
                    err = Error::CHECKSUM;
                    Log::debug(
                        "Rcvd Checksum: 0x%02" PRIx8 " Expecting: 0x%02" PRIx8 "\n", byte,
                        this->m_checksum);
                    this->m_checksum = byte;
                }
                nextState = State::IDLE;
                break;
            }

            this->m_checksum += byte;
            if (this->m_paramIdx < this->m_maxParams) {
                this->m_params[this->m_paramIdx] = byte;
            }
            this->m_paramIdx++;
            break;
        }
    }

    this->m_state = nextState;

    return err;
}

}  // namespace bioloid

//! @}  bioloid group
