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
 *   @file   FileStorage.h
 *
 *   @brief  Implements the control table for a bioloid gadget.
 *
 ****************************************************************************/

#pragma once

#include "ControlTable.h"

//! @addtogroup bioloid
//! @{

namespace bioloid {

//! @brief Class which implements control table storage using a file.
class FileStorage : public IControlTableStorage {
 public:
    //! @brief Constructor.
    FileStorage(const char* fileName  //!< [in] Name of file to store control table in.
    );

    //! @brief Returns the filename that was passed to the construcor.
    //! @return const char* C string containing the filename.
    const char* fileName() const { return this->m_fileName; }

    Error load(OffsetType offset, uint8_t numBytes, void* data) override;
    Error save(OffsetType offset, uint8_t numBytes, const void* data) override;

 private:
    char const* m_fileName;
};

}  // namespace bioloid

//! @}
