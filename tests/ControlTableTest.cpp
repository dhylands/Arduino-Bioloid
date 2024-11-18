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
 *   @file   ControlTableTest.cpp
 *
 *   @brief  Tests the packet parser.
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "ControlTable.h"
#include "FileStorage.h"
#include "Util.h"

//! @brief Test port for testing the control table.
class TestPort : public bioloid::IPort {
    uint8_t available() override { return 0; }

    uint8_t readByte() { return 0xff; }

    void writePacket(bioloid::Packet const& pkt) { (void)pkt; }
};

//! @brief Test control table class.
class TestControlTable : public bioloid::IControlTable {
 public:
    //! @brief Total number of bytes in the control table.
    static constexpr uint8_t NUM_CTL_BYTES = 0x20;

    //! @brief Number of bytes which are persisted.
    static constexpr uint8_t NUM_PERSISTENT_BYTES = 0x10;

    //! @brief Default value for field in TestControlTable
    //! @{
    static constexpr uint32_t FIELD1_DEFAULT = 0x11223344;
    static constexpr uint16_t FIELD2_DEFAULT = 0x5566;
    static constexpr uint8_t FIELD3_DEFAULT = 0x77;
    //! @}

    //! @brief Offsets for fields in the TestControlTable
    struct Offset : public IControlTable::Offset {
        // Persistent fields
        static constexpr Type FIELD1 = 0x06;  //!< a uint32_t field
        static constexpr Type FIELD2 = 0x0A;  //!< a uint16_t field
        static constexpr Type FIELD3 = 0x0C;  //!< a uint8_t field

        // Non-persistent fields
        static constexpr Type FIELD4 = 0x1B;  //!< a volatile field
    };

    TestControlTable()
        : IControlTable(
              NUM_CTL_BYTES,
              NUM_PERSISTENT_BYTES,
              this->m_ctlBytes,
              this->m_storage,
              &this->m_port),
          m_storage("ControlTableTest.ctl") {}

    TestControlTable(uint8_t* ctlBytes)
        : IControlTable(
              NUM_CTL_BYTES,
              NUM_PERSISTENT_BYTES,
              ctlBytes,
              this->m_storage,
              &this->m_port),
          m_storage("ControlTableTest.ctl") {}

    void setToInitialValues() override {
        this->IControlTable::setToInitialValues();
        this->set(Offset::FIELD1, FIELD1_DEFAULT);
        this->set(Offset::FIELD2, FIELD2_DEFAULT);
        this->set(Offset::FIELD3, FIELD3_DEFAULT);
    }

    //! @brief Returns the name of the file storage.
    //! @return a pointer to a C string containing the storage filename.
    char const* fileName() { return this->m_storage.fileName(); }

 protected:
    void populateEntry(Offset::Type offset) const override {
        this->IControlTable::populateEntry(offset);
    }

    void entryModified(Offset::Type offset) override { this->IControlTable::entryModified(offset); }

 private:
    uint8_t m_ctlBytes[NUM_CTL_BYTES];
    bioloid::FileStorage m_storage;
    TestPort m_port;
};

using Offset = TestControlTable::Offset;  //!< Convenience alias

TEST(ControlTableTest, InitialValue) {
    TestControlTable test;

    test.setToInitialValues();

    EXPECT_EQ(test.get_u8(Offset::ID), TestControlTable::DEFAULT_DEVICE_ID);
    EXPECT_EQ(test.get_u8(Offset::BAUD), TestControlTable::DEFAULT_BAUD);
    EXPECT_EQ(test.get_u8(Offset::RDT), TestControlTable::DEFAULT_RDT);

    EXPECT_EQ(test.get_u32(Offset::FIELD1), TestControlTable::FIELD1_DEFAULT);
    EXPECT_EQ(test.get_u16(Offset::FIELD2), TestControlTable::FIELD2_DEFAULT);
    EXPECT_EQ(test.get_u8(Offset::FIELD3), TestControlTable::FIELD3_DEFAULT);

    EXPECT_EQ(test.get_u8(Offset::FIELD1), 0x44);  // LSB of FIELD1_DEFAULT
    EXPECT_EQ(test.get_u8(Offset::FIELD1 + 1), 0x33);
    EXPECT_EQ(test.get_u8(Offset::FIELD1 + 2), 0x22);
    EXPECT_EQ(test.get_u8(Offset::FIELD1 + 3), 0x11);

    EXPECT_EQ(test.get_u8(Offset::FIELD2), 0x66);
    EXPECT_EQ(test.get_u8(Offset::FIELD2 + 1), 0x55);
}

TEST(ControlTableTest, TestSave) {
    TestControlTable test;

    // Remove the file, if it exists
    remove(test.fileName());

    // Loading a non-existent file should set to intitial values.
    test.load();

    EXPECT_EQ(test.get_u8(Offset::ID), TestControlTable::DEFAULT_DEVICE_ID);
    EXPECT_EQ(test.get_u8(Offset::BAUD), TestControlTable::DEFAULT_BAUD);
    EXPECT_EQ(test.get_u8(Offset::RDT), TestControlTable::DEFAULT_RDT);

    EXPECT_EQ(test.get_u32(Offset::FIELD1), TestControlTable::FIELD1_DEFAULT);
    EXPECT_EQ(test.get_u16(Offset::FIELD2), TestControlTable::FIELD2_DEFAULT);
    EXPECT_EQ(test.get_u8(Offset::FIELD3), TestControlTable::FIELD3_DEFAULT);

    // Modify a values
    test.set(Offset::FIELD1, 0x01020304);

    EXPECT_EQ(test.save(), bioloid::IControlTableStorage::Error::NONE);

    //! Read the file back indepdendently
    uint8_t buf[TestControlTable::NUM_PERSISTENT_BYTES];
    FILE* fs = fopen(test.fileName(), "rb");
    EXPECT_NE(fs, nullptr);

    EXPECT_EQ(fread(buf, 1, sizeof(buf), fs), sizeof(buf));
    EXPECT_EQ(memcmp(buf, test.ctlBytes(), LEN(buf)), 0);

    fclose(fs);

    test.setToInitialValues();

    // Verify that loading the files loads the modified values
    test.load();

    EXPECT_EQ(memcmp(buf, test.ctlBytes(), LEN(buf)), 0);

    EXPECT_EQ(test.get_u32(Offset::FIELD1), 0x01020304);
}

TEST(ControlTableDeathTest, NullFileName) {
    EXPECT_DEATH(TestControlTable(nullptr), "Assertion `this->m_ctlBytes != nullptr' failed.");
}

TEST(ControlTableDeathTest, BadOffset_get_u8) {
    TestControlTable test;
    test.setToInitialValues();
    EXPECT_DEATH(
        test.get_u8(TestControlTable::NUM_CTL_BYTES),
        "Assertion `offset \\+ sizeof\\(val\\) <= this->m_numCtlBytes' failed.");
}

TEST(ControlTableDeathTest, BadOffset_get_u16) {
    TestControlTable test;
    test.setToInitialValues();
    EXPECT_DEATH(
        test.get_u16(TestControlTable::NUM_CTL_BYTES),
        "Assertion `offset \\+ sizeof\\(val\\) <= this->m_numCtlBytes' failed.");
}

TEST(ControlTableDeathTest, BadOffset_get_u32) {
    TestControlTable test;
    test.setToInitialValues();
    EXPECT_DEATH(
        test.get_u32(TestControlTable::NUM_CTL_BYTES),
        "Assertion `offset \\+ sizeof\\(val\\) <= this->m_numCtlBytes' failed.");
}

TEST(ControlTableDeathTest, BadOffset_get) {
    TestControlTable test;
    test.setToInitialValues();
    uint32_t val;
    EXPECT_DEATH(
        test.get(TestControlTable::NUM_CTL_BYTES, &val),
        "Assertion `offset \\+ sizeof\\(T\\) <= this->m_numCtlBytes' failed.");
}

TEST(ControlTableDeathTest, BadOffset_set) {
    TestControlTable test;
    test.setToInitialValues();
    uint32_t val = 1;
    EXPECT_DEATH(
        test.set(TestControlTable::NUM_CTL_BYTES, val),
        "Assertion `offset \\+ sizeof\\(T\\) <= this->m_numCtlBytes' failed.");
}
