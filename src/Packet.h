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
 *   @file   Packet.h
 *
 *   @brief  Parses bioloid packets.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>
#include <initializer_list>

#include "Bioloid.h"

//! Forward declaration.
class PacketTest_BadState_Test;

//! @addtogroup bioloid
//! @{

namespace bioloid {

//! @brief Encapsulates the packet sent to a devices.
//! @details The over the wire format looks like this:
//! @code
//!     FF FF ID Length Command Param0 Param1 ... CHECKSUM
//! @endcode
//! When Length is the number of parameters + 2
class Packet {
 public:
    //! Default constructor
    Packet();

    //! Constructor where the storage for parameter data is specified.
    Packet(
        size_t maxParams,  //!< [in] Max number of params that can be storeed.
        void* params       //!< [in] Place to store parameters.
    );

    //! Destructor
    ~Packet();

    //! Returns the ID from the packet.
    //! @returns ID::Type containing the ID froom the packet.
    ID::Type id() const { return this->m_id; }

    //! Sets the device ID from an ID object.
    void id(ID id  //!< ID object to set ID to.
    ) {
        this->m_id = id.value;
    }

    //! Sets the device ID from an IDType.
    void id(ID::Type id  //!< ID object to set ID to.
    ) {
        this->m_id = id;
    }

    //! Returns the length of the packet.
    //! @returns uint8_t containing the length of the packet. This is defined as the number
    //!          of parameters + 2.
    uint8_t length() const { return this->m_length; }

    //! Returns the command from the command packet.
    //! @returns Command::Type containg the command found in the packet.
    Command::Type command() const { return this->m_cmd; }

    //! Sets the command for the packet using a Command object.
    void command(Command cmd  //!< [in] Command object to set command from..
    ) {
        this->m_cmd = cmd.value;
    }

    //! Sets the command for the packet using a CommandType.
    void command(Command::Type cmd  //!< [in] Command to set command to.
    ) {
        this->m_cmd = cmd;
    }

    //! Returns the Error code from the response packet.
    //! @note that the command and errCode occupy the same position in the packet.
    //!       Error codes are found in response packets.
    //! @returns Error::Type containing the error code found in the packet.
    Error::Type errorCode() const { return this->m_cmd; }

    //! Sets the error code using an Error object.
    void errorCode(Error err  //!< [in] Error object to get error code from.
    ) {
        this->m_cmd = err.value;
    }

    //! Sets the error code using an ErrorType.
    void errorCode(Error::Type err  //!< [in] Error object to get error code from.
    ) {
        this->m_cmd = err;
    }

    //! Returns the number of parameters included in the packet.
    //! @returns uint8_t containing the number of parameter bytes found in the packet.
    uint8_t numParams() const {
        if (this->m_length <= 2) {
            return 0;
        }
        return this->m_length - 2;
    }

    //! Sets the parameter bytes
    void params(
        size_t numParams,   //!< [in] Number of bytes of parameter data.
        const void* params  //!< [in] Parameter data to set parameters to.
    );

    //! Sets the number of parameter bytes.
    //! @details This function assumes that the caller will write the parameter data
    //!          into the buffer that was passed to the construtor.
    void params(size_t numParams  //!< [in] Number of bytes of parameter data.
    );

    //! Sets the parameters bytes using an initializer list.
    //! @details This allows the following to be used:
    //! @code
    //!     packet.params({0x01, 0x02});
    //! @endcode
    //! and you'll get the same thing as if you did:
    //! @code
    //!     uint8_t data[] = {0x01, 0x02};
    //!     packet.params(LEN(data), data);
    //! @endcode
    void params(std::initializer_list<uint8_t> p  //!< [in] Initializer list to use.
    ) {
        this->params(p.size(), p.begin());
    }

    //! Returns the  checksum parsed with the packet.
    //! @returns uint8_t containing the checksum found in the packet.
    uint8_t checksum() const { return this->m_checksum; }

    //! Updates the checksum based on the packet contents.
    void update_checksum();

    //! Runs a single byte through the packet parser state machine.
    //! @returns Error::NONE if the packet was parsed successfully.
    //! @returns Error::NOT_DONE if the packet is incomplete.
    //! @returns Error::CHECKSUM if a checksum error was encountered.
    Error::Type processByte(uint8_t byte  //!< [in] Byte to parse.
    );

    //! Reconstructs the packet that was received.
    //! @returns the number of bytes stored into the buffer.
    size_t data(
        size_t maxLen,  //!< [in] Size of the output buffer.
        void* data      //!< [out] Place to store the packet data.
    );

 private:
    //! This allows the TEST(PacketTest, BadState) function to access m_state
    friend class ::PacketTest_BadState_Test;

    enum class State {
        IDLE,          //!< We're waiting for the beginning of the packet.
        FF_1ST_RCVD,   //!< We've received the 1st 0xFF.
        FF_2ND_RCVD,   //!< We've received the 2nd 0xFF.
        ID_RCVD,       //!< We've received the ID.
        LENGTH_RCVD,   //!< We've received the length.
        COMMAND_RCVD,  //!< We've received the command.
    };

    State m_state;              //!< State of the parser.
    uint8_t const m_maxParams;  //!< Max number of bytes of parameter data.
    uint8_t* const m_params;    //!< Place to store packet data.

    ID::Type m_id = ID::DEFAULT;          //!< ID asssociated with the packer.
    uint8_t m_length = 2;                 //!< Length of the packet.
    Command::Type m_cmd = Command::PING;  //!< Error code for a status packet.
    uint8_t m_paramIdx = 0;               //!< index of parameter being parsed.
    uint8_t m_checksum = 0;               //!< Checksum parsed frm the packet.
};

}  // namespace bioloid

//! @}
