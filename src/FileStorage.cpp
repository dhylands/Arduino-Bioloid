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
 *   @file   FileStorage.cpp
 *
 *   @brief  Implements the control table for a bioloid gadget.
 *
 ****************************************************************************/

#include "FileStorage.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

bioloid::FileStorage::FileStorage(const char* fileName) : m_fileName(fileName) {
    assert(this->m_fileName != nullptr);
}

bioloid::IControlTableStorage::Error
bioloid::FileStorage::load(OffsetType offset, uint8_t numBytes, void* data) {
    int fd = open(this->m_fileName, O_RDONLY);
    if (fd < 0) {
        return Error::FAILED;
    }
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        close(fd);
        return Error::FAILED;
    }
    if (read(fd, data, numBytes) != numBytes) {
        close(fd);
        return Error::FAILED;
    }
    close(fd);
    return Error::NONE;
}

bioloid::IControlTableStorage::Error
bioloid::FileStorage::save(OffsetType offset, uint8_t numBytes, const void* data) {
    // Using mode a+ will create the file if it doesn't exist and leave the contents
    // alone if it does. The file pointer is positioned at the end of the file, but
    // we seek to our position anyways, so this doesn't matter.
    int fd = open(this->m_fileName, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        return Error::FAILED;
    }
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        close(fd);
        return Error::FAILED;
    }
    if (write(fd, data, numBytes) != numBytes) {
        close(fd);
        return Error::FAILED;
    }
    close(fd);
    return Error::NONE;
}
