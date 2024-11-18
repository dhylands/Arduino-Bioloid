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
 *   @file   ControlTable.cpp
 *
 *   @brief  Implements the control table for a bioloid gadget.
 *
 ****************************************************************************/

#include "ControlTable.h"

bioloid::IControlTable::IControlTable(
    uint8_t numCtlBytes,
    uint8_t numPersistentBytes,
    uint8_t* ctlBytes,
    IControlTableStorage& storage,
    IPort* port)
    : m_numCtlBytes{numCtlBytes},
      m_numPersistentBytes{numPersistentBytes},
      m_ctlBytes{ctlBytes},
      m_storage{storage},
      m_port{port} {
    assert(this->m_ctlBytes != nullptr);
}

void bioloid::IControlTable::load() {
    memset(this->m_ctlBytes, 0, this->m_numCtlBytes);
    auto rc = this->m_storage.load(0, this->m_numPersistentBytes, &this->m_ctlBytes[0]);
    if (rc == IControlTableStorage::Error::NONE) {
        return;
    }

    this->setToInitialValues();
}

bioloid::IControlTableStorage::Error bioloid::IControlTable::save() {
    return this->m_storage.save(0, this->m_numPersistentBytes, &this->m_ctlBytes[0]);
}

void bioloid::IControlTable::setToInitialValues() {
    memset(this->m_ctlBytes, 0, this->m_numCtlBytes);

    this->set(Offset::ID, DEFAULT_DEVICE_ID);
    this->set(Offset::BAUD, DEFAULT_BAUD);
    this->set(Offset::RDT, DEFAULT_RDT);
}

void bioloid::IControlTable::populateEntry(Offset::Type offset) const {
    // Currently nothing to do
    (void)offset;
}

void bioloid::IControlTable::entryModified(Offset::Type offset) {
    if (offset == Offset::BAUD) {
        uint32_t val = this->get_u8(Offset::BAUD) + 1;
        uint32_t baudRate = 2'000'000 / val;
        this->m_port->setBaudRate(baudRate);
    }
}
