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
 *   @file   Bioloid.h
 *
 *   @brief  Contains some constants which are used by various bioloid classes.
 *
 ****************************************************************************/

#pragma once

#include <stdint.h>

#include "Str.h"
#include "Util.h"

//! @addtogroup bioloid
//! @{

namespace bioloid {

//! ID of a device.
struct ID : Bits<uint8_t> {
    static constexpr Type DEFAULT = 0x00;    //!< Default ID.
    static constexpr Type BROADCAST = 0xFE;  //!< Broadcast to all devices on the bus.
    static constexpr Type INVALID = 0xFF;    //!< An invalid ID.
};

//! @brief Predefined commands.
//! @details We use a struct rather than an enum so that a device can derive their own commands.
struct Command : Bits<uint8_t> {
    static constexpr Type PING = 0x01;        //!< Used to obatin a status packet
    static constexpr Type READ = 0x02;        //!< Read values from the control table
    static constexpr Type WRITE = 0x03;       //!< Write values to control table
    static constexpr Type REG_WRITE = 0x04;   //!< Prime values to write when ACTION sent
    static constexpr Type ACTION = 0x05;      //!< Triggers REG_WRITE
    static constexpr Type RESET = 0x06;       //!< Changes control values back to factory defaults
    static constexpr Type SYNC_WRITE = 0x83;  //!< Writes values to many devices

    //! @brief Returns the string version of a command.
    //! @details By making this virtual, derived classes can add custom commands and also
    //!          return the string equivalents.
    //! @returns A pointer to a C string containing the string equivalent of the command.
    virtual const char* as_str() {
        switch (this->value) {
            case PING:
                return "PING";
            case READ:
                return "READ";
            case WRITE:
                return "WRITE";
            case REG_WRITE:
                return "REG_WRITE";
            case ACTION:
                return "ACTION";
            case RESET:
                return "RESET";
            case SYNC_WRITE:
                return "SYNC_WRITE";
        }
        return "???";
    }
};

//! @brief Error codes.
//! @note that the error codes <= 0xff are bit masks and multiple bits may be set.
struct Error : public Bits<uint16_t> {
    static constexpr Type RESERVED = 0x80;       //!< Reserved - set to zero
    static constexpr Type INSTRUCTION = 0x40;    //!< Undefined instruction
    static constexpr Type OVERLOAD = 0x20;       //!< Max torque can't control applied load
    static constexpr Type CHECKSUM = 0x10;       //!< Checksum of instruction packet incorrect
    static constexpr Type RANGE = 0x08;          //!< Instruction is out of range
    static constexpr Type OVERHEATING = 0x04;    //!< Internal temperature is too high
    static constexpr Type ANGLE_LIMIT = 0x02;    //!< Goal position is outside of limit range
    static constexpr Type INPUT_VOLTAGE = 0x01;  //!< Input voltage out of range
    static constexpr Type NONE = 0x00;           //!< No Error

    //! Special error code used by Packet::processByte()
    static constexpr Type NOT_DONE = 0x100;

    //! Indicates that a timeout occurred whilw waiting for a reply
    static constexpr Type TIMEOUT = 0x101;

    //! Packet storage isn't big enough
    static constexpr Type TOO_MUCH_DATA = 0x102;

    //! Construct an Error from an ErrorType.
    Error(Type value  //!< [in] uint16_t value to initialize error from.
          )
        : Bits{value} {}

    //! @brief Converts the error code into a uint8_t.
    //! @note This is only suitable for the error codes which are returned in a status reply packet.
    //! @returns the uint8_t version of the error code.
    uint8_t as_uint8_t() { return static_cast<uint8_t>(this->value); }

    //! Converts the error code into its string equivalent.
    //! @returns a pointer to outStr.
    const char* as_str(
        size_t outLen,  //!< [in] Lenght of output string
        char* outStr    //!< [out] Place
    ) {
        const char* s = "";

        // These are error codes which don't use bit maaks
        switch (this->value) {
            case Error::NONE:
                s = "None";
                break;
            case Error::NOT_DONE:
                s = "NotDone";
                break;
            case Error::TIMEOUT:
                s = "Timeout";
                break;
            case Error::TOO_MUCH_DATA:
                s = "TooMuchdata";
                break;
        }
        StrMaxCpy(outStr, s, outLen);
        if (outStr[0] != '\0') {
            return outStr;
        }

        static const char* errStr[] = {
            // clang-format off
            "InputVoltage",
            "AngleLimit",
            "Overheating",
            "Range",
            "Checksum",
            "Overload",
            "Instruction",
            "Reserved",
            // clang-format on
        };

        for (uint_fast8_t i = 0; i < 8; i++) {
            if (this->isSet(1 << i)) {
                if (outStr[0] != '\0') {
                    StrMaxCat(outStr, " ", outLen);
                }
                StrMaxCat(outStr, errStr[i], outLen);
            }
        }
        return outStr;
    }
};

}  // namespace bioloid

//! @}  bioloid group
