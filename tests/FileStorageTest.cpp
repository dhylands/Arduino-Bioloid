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
 *   @file   FileStorageTest.cpp
 *
 *   @brief  Tests the packet parser.
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdint>
#include <vector>

#include "ControlTable.h"
#include "FileStorage.h"
#include "Util.h"

static constexpr const char* fileName = "FileStorageTest.ctl";

using FileStorage = bioloid::FileStorage;

TEST(FileStorageTest, LoadTest) {
    FileStorage test(fileName);

    remove(fileName);

    uint8_t buf[32];

    // Loading a non-existant file should fail.
    EXPECT_EQ(test.load(0, LEN(buf), buf), FileStorage::Error::FAILED);

    for (uint_fast8_t i = 0; i < LEN(buf); i++) {
        buf[i] = i;
    }

    // Create a file
    FILE* fs = fopen(fileName, "wb");
    EXPECT_NE(fs, nullptr);
    EXPECT_EQ(fwrite(buf, 1, LEN(buf), fs), LEN(buf));
    fclose(fs);

    // Loading partway thru the files should fail
    EXPECT_EQ(test.load(10, LEN(buf), buf), FileStorage::Error::FAILED);

    // Loading the file from offset zero should succeed
    EXPECT_EQ(test.load(0, LEN(buf), buf), FileStorage::Error::NONE);

    for (uint_fast8_t i = 0; i < LEN(buf); i++) {
        EXPECT_EQ(buf[i], i);
    }
}

TEST(FileStorageDeathTest, NullFileName) {
    EXPECT_DEATH(FileStorage test(nullptr), "Assertion `this->m_fileName != nullptr' failed.");
}

TEST(FileStorageTest, SaveFailTest) {
    FileStorage test("/fail.txt");

    uint8_t buf[32];

    // Writing to a non-permissible location should fail
    EXPECT_EQ(test.save(0, LEN(buf), buf), FileStorage::Error::FAILED);
}

TEST(FileStorageTest, SaveFullTest) {
    FileStorage test("/dev/full");

    uint8_t buf[32] = {};

    // Writing to a non-permissible location should fail
    EXPECT_EQ(test.save(0, LEN(buf), buf), FileStorage::Error::FAILED);
}

TEST(FileStorageTest, SaveSeekFail) {
    FileStorage test("/dev/tty");

    uint8_t buf[32] = {};

    // Seeking on /dev/tty will fail
    EXPECT_EQ(test.load((255), LEN(buf), buf), FileStorage::Error::FAILED);
    EXPECT_EQ(test.save((255), LEN(buf), buf), FileStorage::Error::FAILED);
}
