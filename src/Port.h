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
 *   @file   Port.h
 *
 *   @brief  Base class for implementing a port.
 *
 ****************************************************************************/

#pragma once

#include "Packet.h"

//! @addtogroup bioloid
//! @{

namespace bioloid {

//! @brief Abstract base class for a port used to talk to bioloid devices.
class IPort {
 public:
    //! @brief Destructor.
    virtual ~IPort() = default;

    //! @brief Returns the number of bytes which can be read without blocking.
    //! @returns The number of bytes that can be read without blocking.
    virtual uint8_t available() = 0;

    //! @brief Sets the baud rate (if applicablle).
    //! @param baudRate
    virtual void setBaudRate(uint32_t baudRate  //! [in] baud rate (in bits/second)
    ) {
        (void)baudRate;
    }

    //! @brief Reads a byte from the port (or other virtual device).
    //! @details This function blocks until a byte is available.
    //! @returns The byte that was read.
    virtual uint8_t readByte() = 0;

    //! @brief Write an entire packet to the  port.
    virtual void writePacket(Packet const& pkt  //!< [in] Packet to write.
                             ) = 0;
};

}  // namespace bioloid

//! @}
