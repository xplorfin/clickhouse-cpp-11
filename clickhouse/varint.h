#pragma once

#include <stdexcept>
#include <string>

#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace clickhouse {

#define DEFAULT_MAX_STRING_SIZE 0x00FFFFFFULL

inline void readVarUInt(int s, uint64_t& x) {
    x = 0;
    for (size_t i = 0; i < 9; ++i)
    {
        uint8_t byte;
        int ret = recv(s, &byte, sizeof(byte), 0);

        if (ret != 1) {
            throw std::runtime_error("fail");
        }

        x |= (byte & 0x7F) << (7 * i);

        if (!(byte & 0x80))
            return;
    }
}

inline void readStringBinary(int s, std::string& str, size_t max_size = DEFAULT_MAX_STRING_SIZE)
{
    uint64_t size = 0;
    readVarUInt(s, size);

    if (size > max_size)
        throw std::runtime_error("Too large string size.");

    str.resize(size);

    int ret = recv(s, &str[0], size, 0);
    if (ret != size) {
        throw std::runtime_error("can't receive string data");
    }
}

template <typename T>
inline void readBinary(int s, T& x) {
    int ret = recv(s, &x, sizeof(x), 0);
    if (ret != sizeof(x)) {
        throw std::runtime_error("can't receive binary data");
    }
}

inline char* writeVarUInt(uint64_t x, char* ostr) {
    for (size_t i = 0; i < 9; ++i) {
        uint8_t byte = x & 0x7F;
        if (x > 0x7F)
            byte |= 0x80;

        *ostr = byte;
        ++ostr;

        x >>= 7;
        if (!x)
            return ostr;
    }

    return ostr;
}

inline char* writeStringBinary(const std::string& s, char* ostr) {
    ostr = writeVarUInt(s.size(), ostr);
    memcpy(ostr, s.data(), s.size());
    ostr += s.size();
    return ostr;
}

}