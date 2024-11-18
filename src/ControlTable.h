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
 *   @file   ControlTable.h
 *
 *   @brief  Implements the control table for a bioloid gadget.
 *
 ****************************************************************************/

#pragma once

#include <cassert>
#include <cstdint>
#include <type_traits>

#include "Port.h"
#include "Util.h"

//! @addtogroup bioloid
//! @{

namespace bioloid {

class IControlTable;  // forward declartion.

//! @brief Abstracts the storage method used for storing the control table data.
//! @details Derived class could use EEPROM, flash, or evan a file to store the control data.
class IControlTableStorage {
 public:
    //! @brief Tyoe used to store offests.
    //! @details We can't use Offset::Type because it hasn't been declared yet, so we
    //!          create another typename with the same type, and static assert that
    //!           OffsetType and Offset::Type are the same.
    using OffsetType = uint8_t;

    //! @brief Error code which indicates whether a storage operation was successful or not.
    enum class Error {
        NONE = 0,  //!< Load/save was successful.
        FAILED,    //!< Load/save failed.
    };

    //! @brief Destructor.
    //! @details This class contains virtual methods, so a virtual destructor is declared.
    virtual ~IControlTableStorage() = default;

    //! @brief Loads the persistent portion of the control table from storage.
    //! @returns Error::NONE if the control table was loaded successfully.
    //! @returns Error::FAILED if an erroor occurred while loading the control table.
    virtual Error load(
        OffsetType offset,  //!< [in] offset of the first byte to load.
        uint8_t numBytes,   //!< [in] Number of bytes to load.
        void* data          //!< [out] Place to store data loaded.
        ) = 0;

    //! @brief Saves the persistent portion of the control table to storage.
    //! @returns Error::NONE if the control table was saved successfully.
    //! @returns Error::FAILED if an error occurred while saving the control table.
    virtual Error save(
        OffsetType offset,  //!< [in] offset of the first byte to save.
        uint8_t numBytes,   //!< [in] Number of bytes to save.
        const void* data    //!< [in] Data to save.
        ) = 0;
};

//! @brief The ControlTable contains informaton in the status and opeatation of the device.
//! @tparam NUM_CTL_BYTES - total number of bytes in the control tables.
//! @tparam NUM_PERSISTENT_BYTES - number of bytes that are persisted.
class IControlTable {
 public:
    //! @brief Offsets for common fields within control table.
    //! @details Devices can derive from this to add their own offsets.
    struct Offset : public Bits<uint8_t> {
        static constexpr Type MODEL = 0x00;    //!< 2-byte Model Number LSB, MSB in 0x01
        static constexpr Type VERSION = 0x02;  //!< Firmware Version
        static constexpr Type ID = 0x03;       //!< Device ID
        static constexpr Type BAUD = 0x04;     //!< Baud Rate (2,000,000 / (val + 1))
        static constexpr Type RDT = 0x05;      //!< Return Delay Time: (val * 2) usecs
        static constexpr Type LED = 0x19;      //!< Status LED
    };

    static constexpr uint8_t DEFAULT_DEVICE_ID = 0x00;  //!< Initial device ID
    static constexpr uint8_t DEFAULT_BAUD = 0x01;       //!< Corresponds to 1 Mbit/sec
    static constexpr uint8_t DEFAULT_RDT = 250;         //!< Corresponds to 500 usec

    //! @brief Constructor.
    IControlTable(
        uint8_t numCtlBytes,            //!< [in] Number of bytes in the control table.
        uint8_t numPersistentBytes,     //!< [in] Number of persistent bytes.
        uint8_t* ctlBytes,              //!< [in] Underlying memory used to store the control bytes.
        IControlTableStorage& storage,  //!< [in] Class which actually persists the data.
        IPort* port                     //!< [in] Port associated with the device.

    );

    //! @brief Destructor.
    //! @details This class contains virtual methods, so a virtual destructor is declared.
    virtual ~IControlTable() = default;

    //! @brief Returns a uint8_t from the control table.
    //! @returns uint8_t from the control table at the indicated offset.
    uint8_t get_u8(
        Offset::Type offset  //!< [in] Offset within control table to retrieve value from.
    ) const {
        uint8_t val;
        assert(offset + sizeof(val) <= this->m_numCtlBytes);
        this->get(offset, &val);
        return val;
    }

    //! @brief Returns a uint16_t from the control table.
    //! @details the data is assumed to be stored in little endian byte order.
    //! @returns uint16_t from the control table at the indicated offset.
    uint16_t get_u16(
        Offset::Type offset  //!< [in] Offset within control table to retrieve value from.
    ) const {
        uint16_t val;
        assert(offset + sizeof(val) <= this->m_numCtlBytes);
        this->get(offset, &val);
        return val;
    }

    //! @brief Returns a uint32_t from the control table.
    //! @details the data is assumed to be stored in little endian byte order.
    //! @returns uint32_t from the control table at the indicated offset.
    uint32_t get_u32(
        Offset::Type offset  //!< [in] Offset within control table to retrieve value from.
    ) const {
        uint32_t val;
        assert(offset + sizeof(val) <= this->m_numCtlBytes);
        this->get(offset, &val);
        return val;
    }

    //! @brief Returns an int8_t from the control table.
    //! @returns int8_t from the control table at the indicated offset.
    int8_t get_i8(Offset::Type offset  //!< [in] Offset within control table to retrieve value from.
    ) const {
        int8_t val;
        assert(offset + sizeof(val) <= this->m_numCtlBytes);
        this->get(offset, &val);
        return val;
    }

    //! @brief Returns an int16_t from the control table.
    //! @details the data is assumed to be stored in little endian byte order.
    //! @returns int16_t from the control table at the indicated offset.
    int16_t get_i16(
        Offset::Type offset  //!< [in] Offset within control table to retrieve value from.
    ) const {
        int16_t val;
        assert(offset + sizeof(val) <= this->m_numCtlBytes);
        this->get(offset, &val);
        return val;
    }

    //! @brief Returns an int32_t from the control table.
    //! @details the data is assumed to be stored in little endian byte order.
    //! @returns int32_t from the control table at the indicated offset.
    int32_t get_i32(
        Offset::Type offset  //!< [in] Offset within control table to retrieve value from.
    ) const {
        int32_t val;
        assert(offset + sizeof(val) <= this->m_numCtlBytes);
        this->get(offset, &val);
        return val;
    }

    //! @brief Retrieves a value from the control table.
    //! @details the data is assumed to be stored in little endian byte order.
    //! @tparam T the type to retrieve from the table.
    template <typename T>
    void get(
        Offset::Type offset,  //!< [in] Offset within control table to retrieve value from.
        T* val                //!< [out] Place to store the value retrieved.
    ) const {
        static_assert(std::is_integral_v<T>);
        assert(offset + sizeof(T) <= this->m_numCtlBytes);

        this->populateEntry(offset);
        if constexpr (sizeof(T) == 1) {
            *val = static_cast<T>(this->m_ctlBytes[offset]);
        } else if constexpr (sizeof(T) == 2) {
            *val = this->m_ctlBytes[offset + 1];
            *val <<= 8;
            *val |= this->m_ctlBytes[offset];
        } else {
            offset += sizeof(T);
            *val = this->m_ctlBytes[--offset];
            for (uint_fast8_t i = 1; i < sizeof(T); i++) {
                *val <<= 8;
                *val |= this->m_ctlBytes[--offset];
            }
        }
    }

    //! @brief Sets a value in the control table.
    //! @details the data is assumed to be stored in little endian byte order.
    //! @tparam T the type to retrieve from the table.
    template <typename T>
    void set(
        Offset::Type offset,  //!< [in] Offset within control table to retrieve value from.
        T val                 //!< [in] Value to store in the control table.
    ) {
        static_assert(std::is_integral_v<T>);
        assert(offset + sizeof(T) <= this->m_numCtlBytes);

        if constexpr (sizeof(T) == 1) {
            this->m_ctlBytes[offset] = static_cast<T>(val);
        } else if constexpr (sizeof(T) == 2) {
            this->m_ctlBytes[offset++] = val & 0xff;
            val >>= 8;
            this->m_ctlBytes[offset++] = val & 0xff;
        } else {
            this->m_ctlBytes[offset++] = val & 0xff;
            for (uint_fast8_t i = 1; i < sizeof(T); i++) {
                val >>= 8;
                this->m_ctlBytes[offset++] = val & 0xff;
            }
        }
        this->entryModified(offset);
    }

    //! @brief Sets the initial values of the control table.
    //! @details This is done anytime
    virtual void setToInitialValues();

    //! @brief Loads the control table from storage.
    //! @details If loading from storage fails, then the control table will be set to
    //!          its initial valie using setToInitialValue()
    void load();

    //! @brief Saves the control table to storage.
    //! @returns IControlTableStorage::Error::NONE if the control table was saved successfully.
    //! @returns IControlTableStorage::Error::FAILED if the control table could not be saved.
    IControlTableStorage::Error save();

    //! @brief Returns a pointer to the underlying control bytes.
    //! @returns a pointer to the underlying control bytes.
    const uint8_t* ctlBytes() const { return this->m_ctlBytes; }

 protected:
    //! @brief Called to populate a control table entry just before retrieving its value.
    virtual void populateEntry(
        Offset::Type offset  //!< [in] Offset of the field that is being retrieved.
    ) const;

    //! @brief Called whenever one of the control table entries is modified.
    virtual void entryModified(Offset::Type offset  //!< [in] Offset of the field that was modified.
    );

    const uint8_t m_numCtlBytes;         //!< Number of bytes in the control table.
    const uint8_t m_numPersistentBytes;  //!< Number of persistent bytes.
    uint8_t* const m_ctlBytes;           //!< Pointer to the actual control bytes.
    IControlTableStorage& m_storage;     //!< Object which actually persists the control table.
    IPort* m_port;                       //!< Port associated with the device.
};

static_assert(std::is_same_v<IControlTableStorage::OffsetType, IControlTable::Offset::Type>);

}  // namespace bioloid

//! @}
