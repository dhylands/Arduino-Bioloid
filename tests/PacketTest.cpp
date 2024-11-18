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
 *   @file   PacketTest.cpp
 *
 *   @brief  Tests the packet parser.
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "Packet.h"
#include "Util.h"

//! Convenience aliases
//! @{
using ByteBuffer = std::vector<uint8_t>;
using Command = bioloid::Command;
using Error = bioloid::Error;
using ID = bioloid::ID;
//! @}

//! @brief Converts an ASCII string containing hexadecimal digits into binary data.
//! @details Ignnores spaces.
//! @example
//!     @code
//!     auto bytes = AsciiHexToBinary("12 34");
//!     @endcode
//      `bytes[0] will contain 0x12 and bytes[1] will contain 0x34.
//! @returns a uint8_t vector containing the bytes parsed from the string.
ByteBuffer AsciiHexToBinary(const char* str  //!< [in] String to convert.
) {
    ByteBuffer result;
    uint8_t byte;
    bool high_nibble = true;
    while (*str != '\0') {
        if (*str == ' ') {
            str++;
            continue;
        }
        uint8_t nibble = 0;
        if (std::isdigit(*str)) {
            nibble = *str - '0';
        } else if (std::isxdigit(*str)) {
            nibble = toupper(*str) - 'A' + 10;
        } else {
            assert(!"Non-hex digit found");
        }
        if (high_nibble) {
            byte = nibble << 4;
        } else {
            byte |= nibble;
            result.push_back(byte);
        }
        high_nibble = !high_nibble;
        str++;
    }
    return result;
}

//! @brief A class which makes testing packets easier.
//! @details It includes storage for a packet and has a constructor which allows
//!          packets to be constructed from ASCII strings. For example:
//!          @code
//!          auto test = PacketTest("ff ff fe 04 03 03 01 f6");
//!          EXPECT_EQ(test.parseData(), Error::NONE);
//!          @endcode
class PacketTest {
 public:
    //! Default Constructor.
    PacketTest() {}

    //! Construct the packet with a maximum number of parameters.
    explicit PacketTest(
        size_t maxParams  //!< [in] Max number of parameters that the packet can store.
        )
        : m_packet(maxParams, this->m_params) {
        assert(maxParams < LEN(this->m_params));
    }

    //! Constructor which populates m_dataStream from the ASCII hex string.
    explicit PacketTest(const char* str  //!< [in] ASCII Hex string (spaces are ignored).
                        )
        : m_dataStream{AsciiHexToBinary(str)}, m_packet{LEN(this->m_params), this->m_params} {}

    //! Constructor which populates m_dataStream from the ASCII hex string and also limits
    //! the number of parameter bytes.
    PacketTest(
        size_t maxParams,  //!< [in] Max number of parameters that the packet can store.
        const char* str    //!< [in] ASCII Hex string (spaces are ignored).
        )
        : m_dataStream{AsciiHexToBinary(str)}, m_packet{maxParams, this->m_params} {}

    //! Parses all of the bytes from m_dataStream using the packet parser.
    //! @returns Error::NONE if a packet was parsed successfully.
    //! @returns Error::NOT_DONE if more bytes are needed to complete parsing the packet.
    //! @returns Error::TOO_MUCH_DATA if the packet has more parameters than we have storage for.
    //! @returns Error::CHECKSUM if a checksum error was encountered while parsing.
    Error parseData() {
        for (uint8_t byte : this->m_dataStream) {
            if (auto err = this->m_packet.processByte(byte); err != Error::NOT_DONE) {
                return err;
            }
        }
        return Error::NOT_DONE;
    }

    ByteBuffer m_dataStream;   //!< Binary data converted from an ASCII string.
    bioloid::Packet m_packet;  //!< The packet being parsed.
    uint8_t m_params[32];      //!< Storage for the parameter data.
};

TEST(PacketTest, ConstructPacketNoParams) {
    auto test = PacketTest{};

    test.m_packet.id(1);
    test.m_packet.command(Command::PING);
    test.m_packet.params(0);
    test.m_packet.update_checksum();

    EXPECT_EQ(test.m_packet.id(), 1);
    EXPECT_EQ(test.m_packet.length(), 2);
    EXPECT_EQ(test.m_packet.command(), Command::PING);
    EXPECT_EQ(test.m_packet.numParams(), 0);
    EXPECT_EQ(test.m_packet.checksum(), 0xfb);

    uint8_t data[20];
    EXPECT_EQ(test.m_packet.data(LEN(data), data), 6);
    EXPECT_EQ(data[0], 0xff);
    EXPECT_EQ(data[1], 0xff);
    EXPECT_EQ(data[2], 0x01);
    EXPECT_EQ(data[3], 0x02);
    EXPECT_EQ(data[4], 0x01);
    EXPECT_EQ(data[5], 0xfb);
}

TEST(PacketTest, ConstructPacketParamsData) {
    auto test = PacketTest{8};

    debug();
    uint8_t params[] = {0x03, 0x01};
    test.m_packet.id(ID::BROADCAST);
    test.m_packet.command(Command::WRITE);
    test.m_packet.params(LEN(params), params);
    test.m_packet.update_checksum();

    EXPECT_EQ(test.m_packet.id(), ID::BROADCAST);
    EXPECT_EQ(test.m_packet.length(), 4);
    EXPECT_EQ(test.m_packet.command(), Command::WRITE);
    EXPECT_EQ(test.m_packet.numParams(), 2);
    EXPECT_EQ(test.m_params[0], 0x03);
    EXPECT_EQ(test.m_params[1], 0x01);
    EXPECT_EQ(test.m_packet.checksum(), 0xf6);

    uint8_t data[20];
    EXPECT_EQ(test.m_packet.data(LEN(data), data), 8u);
    EXPECT_EQ(data[0], 0xff);
    EXPECT_EQ(data[1], 0xff);
    EXPECT_EQ(data[2], 0xfe);
    EXPECT_EQ(data[3], 0x04);
    EXPECT_EQ(data[4], 0x03);
    EXPECT_EQ(data[5], 0x03);
    EXPECT_EQ(data[6], 0x01);
    EXPECT_EQ(data[7], 0xf6);
}

TEST(PacketTest, ConstructPacketParamsInitializerList) {
    auto test = PacketTest{2};

    test.m_packet.id(ID::BROADCAST);
    test.m_packet.command(Command::WRITE);
    test.m_packet.params({0x03, 0x01});
    test.m_packet.update_checksum();

    EXPECT_EQ(test.m_packet.id(), ID::BROADCAST);
    EXPECT_EQ(test.m_packet.length(), 4);
    EXPECT_EQ(test.m_packet.command(), Command::WRITE);
    EXPECT_EQ(test.m_packet.numParams(), 2);
    EXPECT_EQ(test.m_params[0], 0x03);
    EXPECT_EQ(test.m_params[1], 0x01);
    EXPECT_EQ(test.m_packet.checksum(), 0xf6);

    uint8_t data[20];
    EXPECT_EQ(test.m_packet.data(LEN(data), data), 8u);
    EXPECT_EQ(data[0], 0xff);
    EXPECT_EQ(data[1], 0xff);
    EXPECT_EQ(data[2], 0xfe);
    EXPECT_EQ(data[3], 0x04);
    EXPECT_EQ(data[4], 0x03);
    EXPECT_EQ(data[5], 0x03);
    EXPECT_EQ(data[6], 0x01);
    EXPECT_EQ(data[7], 0xf6);
}

TEST(PacketTest, SetIdTo1) {
    // Command to Set the ID of a connected Dynamixel actuator to 1
    auto test = PacketTest("ff ff fe 04 03 03 01 f6");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.id(), ID::BROADCAST);
    EXPECT_EQ(test.m_packet.length(), 4);
    EXPECT_EQ(test.m_packet.command(), Command::WRITE);
    EXPECT_EQ(test.m_packet.numParams(), 2);
    EXPECT_EQ(test.m_params[0], 0x03);
    EXPECT_EQ(test.m_params[1], 0x01);
    EXPECT_EQ(test.m_packet.checksum(), 0xf6);

    uint8_t data[20];
    EXPECT_EQ(test.m_packet.data(LEN(data), data), 8u);
    EXPECT_EQ(data[0], 0xff);
    EXPECT_EQ(data[1], 0xff);
    EXPECT_EQ(data[2], 0xfe);
    EXPECT_EQ(data[3], 0x04);
    EXPECT_EQ(data[4], 0x03);
    EXPECT_EQ(data[5], 0x03);
    EXPECT_EQ(data[6], 0x01);
    EXPECT_EQ(data[7], 0xf6);
}

TEST(PacketTest, TruncatedData) {
    // Command to Set the ID of a connected Dynamixel actuator to 1
    auto test = PacketTest("ff ff fe 04 03 03 01 f6");

    EXPECT_EQ(test.parseData(), Error::NONE);

    uint8_t data[20];
    uint8_t expectedData[] = {0xff, 0xff, 0xfe, 0x04, 0x03, 0x03, 0x01, 0xf6};

    for (size_t dataLen = 0; dataLen < LEN(expectedData); dataLen++) {
        EXPECT_EQ(test.m_packet.data(dataLen, data), dataLen);

        for (size_t i = 0; i < dataLen; i++) {
            EXPECT_EQ(data[i], expectedData[i]);
        }
    }
}

TEST(PacketTest, ReadInternalTemp) {
    // Read the internal temperature of the Dynamixel actuator with an ID of 1
    auto test = PacketTest("ff ff 01 04 02 2b 01 cc");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.id(), 0x01);
    EXPECT_EQ(test.m_packet.length(), 4);
    EXPECT_EQ(test.m_packet.command(), Command::READ);
    EXPECT_EQ(test.m_packet.numParams(), 2);
    EXPECT_EQ(test.m_params[0], 0x2b);
    EXPECT_EQ(test.m_params[1], 0x01);
    EXPECT_EQ(test.m_packet.checksum(), 0xcc);

    uint8_t data[20];
    EXPECT_EQ(test.m_packet.data(LEN(data), data), 8u);
    EXPECT_EQ(data[0], 0xff);
    EXPECT_EQ(data[1], 0xff);
    EXPECT_EQ(data[2], 0x01);
    EXPECT_EQ(data[3], 0x04);
    EXPECT_EQ(data[4], 0x02);
    EXPECT_EQ(data[5], 0x2b);
    EXPECT_EQ(data[6], 0x01);
    EXPECT_EQ(data[7], 0xcc);
}

TEST(PacketTest, NoSecondFF) {
    // Read the internal temperature of the Dynamixel actuator with an ID of 1
    auto test = PacketTest("ff 00");

    EXPECT_EQ(test.parseData(), Error::NOT_DONE);
}

TEST(PacketTest, Noise) {
    // Read the internal temperature of the Dynamixel actuator with an ID of 1
    auto test = PacketTest("00 ff ff ff 01 04 02 2b 01 cc");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.id(), 0x01);
    EXPECT_EQ(test.m_packet.length(), 4);
    EXPECT_EQ(test.m_packet.command(), Command::READ);
    EXPECT_EQ(test.m_packet.numParams(), 2);
    EXPECT_EQ(test.m_params[0], 0x2b);
    EXPECT_EQ(test.m_params[1], 0x01);
    EXPECT_EQ(test.m_packet.checksum(), 0xcc);
}

TEST(PacketTest, ThreeFFs) {
    // Read the internal temperature of the Dynamixel actuator with an ID of 1
    auto test = PacketTest("ff ff ff 01 04 02 2b 01 cc");

    EXPECT_EQ(test.parseData(), Error::NONE);
    EXPECT_EQ(test.m_packet.id(), 0x01);
    EXPECT_EQ(test.m_packet.length(), 4);
    EXPECT_EQ(test.m_packet.command(), Command::READ);
    EXPECT_EQ(test.m_packet.numParams(), 2);
    EXPECT_EQ(test.m_params[0], 0x2b);
    EXPECT_EQ(test.m_params[1], 0x01);
    EXPECT_EQ(test.m_packet.checksum(), 0xcc);
}

TEST(PacketTest, BadState) {
    // Read the internal temperature of the Dynamixel actuator with an ID of 1
    auto test = PacketTest("00");

    test.m_packet.m_state = static_cast<bioloid::Packet::State>(0xff);
    EXPECT_EQ(test.parseData(), Error::NOT_DONE);
    EXPECT_EQ(test.m_packet.m_state, static_cast<bioloid::Packet::State>(0xff));
}

TEST(PacketTest, TooMuchData) {
    // Read the internal temperature of the Dynamixel actuator with an ID of 1
    auto test = PacketTest(1, "ff ff 01 04 02 2b 01 cc");

    EXPECT_EQ(test.parseData(), Error::TOO_MUCH_DATA);
    EXPECT_EQ(test.m_packet.id(), 0x01);
    EXPECT_EQ(test.m_packet.length(), 4);
    EXPECT_EQ(test.m_packet.command(), Command::READ);
    EXPECT_EQ(test.m_packet.numParams(), 2);
    EXPECT_EQ(test.m_params[0], 0x2b);

    uint8_t data[20];
    EXPECT_EQ(test.m_packet.data(LEN(data), data), 6u);
    EXPECT_EQ(data[0], 0xff);
    EXPECT_EQ(data[1], 0xff);
    EXPECT_EQ(data[2], 0x01);
    EXPECT_EQ(data[3], 0x04);
    EXPECT_EQ(data[4], 0x02);
    EXPECT_EQ(data[5], 0x2b);
}

TEST(PacketTest, Checksum) {
    // Read the internal temperature of the Dynamixel actuator with an ID of 1
    auto test = PacketTest("ff ff 01 04 02 2b 01 ee");

    EXPECT_EQ(test.parseData(), Error::CHECKSUM);
    EXPECT_EQ(test.m_packet.id(), 0x01);
    EXPECT_EQ(test.m_packet.length(), 4);
    EXPECT_EQ(test.m_packet.command(), Command::READ);
    EXPECT_EQ(test.m_packet.numParams(), 2);
    EXPECT_EQ(test.m_params[0], 0x2b);
    EXPECT_EQ(test.m_params[1], 0x01);
    EXPECT_EQ(test.m_packet.checksum(), 0xee);

    uint8_t data[20];
    EXPECT_EQ(test.m_packet.data(LEN(data), data), 8u);
    EXPECT_EQ(data[0], 0xff);
    EXPECT_EQ(data[1], 0xff);
    EXPECT_EQ(data[2], 0x01);
    EXPECT_EQ(data[3], 0x04);
    EXPECT_EQ(data[4], 0x02);
    EXPECT_EQ(data[5], 0x2b);
    EXPECT_EQ(data[6], 0x01);
    EXPECT_EQ(data[7], 0xee);
}

TEST(PacketDeathTest, MaxParams1) {
    uint8_t params[256];
    ASSERT_DEATH(
        bioloid::Packet(LEN(params), params), "Assertion `maxParams <= MAX_PARAMS' failed.");
}

TEST(PacketDeathTest, MaxParams2) {
    uint8_t params[8];
    auto packet = bioloid::Packet(LEN(params), params);

    uint8_t debug_params[12];
    ASSERT_DEATH(
        packet.params(LEN(debug_params), debug_params),
        "Assertion `numParams <= this->m_maxParams' failed.");
}

TEST(PacketDeathTest, MaxParams3) {
    uint8_t params[8];
    auto packet = bioloid::Packet(LEN(params), params);

    uint8_t debug_params[12];
    ASSERT_DEATH(
        packet.params(LEN(debug_params)), "Assertion `numParams <= this->m_maxParams' failed.");
}
