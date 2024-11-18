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
 *   @file   SerialPort.h
 *
 *   @brief  Class for implementing a socket port.
 *
 ****************************************************************************/

#pragma once

#include <cassert>

#include "Port.h"

//! @addtogroup bioloid
//! @{

namespace bioloid {

//! @brief Abstract base class for a port used to talk to bioloid devices.
class SocketPort : public IPort {
 public:
    SocketPort(int socket) : m_socket{socket} { assert(this->m_socket >= 0); }

    //! @brief Returns the number of bytes which can be read without blocking.
    //! @returns The number of bytes that can be read without blocking.
    uint8_t available() override;

    //! @brief Reads a byte from the port (or other virtual device).
    //! @details This function blocks until a byte is available.
    //! @returns The byte that was read.
    uint8_t readByte() override;

    //! @brief Reads a byte
    uint8_t readByteBlocking();

    //! @brief Write an entire packet to the  port.
    void writePacket(Packet const& pkt  //!< [in] Packet to write.
                     ) override;

 private:
    int m_socket;  //!< Socket to use for I/O
};

}  // namespace bioloid

//! @}
