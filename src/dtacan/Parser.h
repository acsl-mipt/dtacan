#pragma once

#include "dtacan/Util.h"
#include "dtacan/BaudRate.h"

#include <algorithm>
#include <string>

#include <cstddef>
#include <stdint.h>

namespace dtacan {

template <typename B>
class Parser {
public:
    void handleData(uint32_t address, const uint8_t* data, std::size_t size);
    void handleJunk(const uint8_t* junk, std::size_t size);

    void acceptData(const void* data, std::size_t size);

private:
    B& base();
    const char* skipJunk(const char* start, const char* it, const char* end);
    uint32_t parseAddress(const char* it, std::size_t size);

    std::string _buffer;
};

template <typename B>
inline B& Parser<B>::base()
{
    return *static_cast<B*>(this);
}

template <typename B>
inline void Parser<B>::handleData(uint32_t address, const uint8_t* data, std::size_t size)
{
    (void)address;
    (void)data;
    (void)size;
}


template <typename B>
inline void Parser<B>::handleJunk(const uint8_t* junk, std::size_t size)
{
    (void)junk;
    (void)size;
}

template <typename B>
inline const char* Parser<B>::skipJunk(const char* start, const char* it, const char* end)
{
    it = std::find_if(it, end, [](char c) {
        return c == '\r';
    });
    base().handleJunk((const uint8_t*)start, it - start);
    return it;
}

template <typename B>
uint32_t Parser<B>::parseAddress(const char* it, std::size_t size)
{
    uint32_t address = 0;
    unsigned shift = size * 4;
    while (shift != 0) {
        shift -= 4;
        uint8_t n = charToNibble(*it);
        if (n == 0xff) {
            return 0xffffffff;
        }
        address |= n << shift;
        it++;
    }
    return address;
}

template <typename B>
void Parser<B>::acceptData(const void* data, std::size_t size)
{
    if (size == 0) {
        return;
    }

    _buffer.append((const char*)data, size);

    const char* it = _buffer.data();
    const char* end = it + _buffer.size();
    std::size_t addrSize;
    uint32_t maxAddress;

    while (true) {
        const char* currentMsg = it;
        switch (*it) {
        case '\r':
            it++;
            break;
        case 'z':
            it++;
            if (it == end) {
                _buffer.resize(1);
                _buffer[0] = 'z';
                return;
            }
            if (*it != '\r') {
                it = skipJunk(currentMsg, it, end);
            } else {
                it++;
            }
            break;
        case 't':
            addrSize = 3;
            maxAddress = 0x7ff;
            goto parseFrame;
        case 'T':
            addrSize = 8;
            maxAddress = 0x1fffffff;
parseFrame: {
            assert(end >= it);
            if (std::size_t(end - it) < addrSize + 2) {
                _buffer.erase(0, currentMsg - _buffer.data());
                return;
            }
            it++;
            uint32_t address = parseAddress(it, addrSize);
            if (address > maxAddress) {
                it = skipJunk(currentMsg, it, end);
                break;
            }
            it += addrSize;
            uint8_t dataSize = charToNibble(*it);
            if (dataSize > 8) {
                it = skipJunk(currentMsg, it, end);
                break;
            }
            it++;
            if ((end - it) < (dataSize * 2 + 1)) {
                _buffer.erase(0, currentMsg - _buffer.data());
                return;
            }
            uint8_t data[8];
            for (std::size_t i = 0; i < dataSize; i++) {
                uint8_t l = charToNibble(it[0]);
                if (l == 0xff) {
                    it = skipJunk(currentMsg, it, end);
                    goto checkEos;
                }
                uint8_t r = charToNibble(it[1]);
                if (r == 0xff) {
                    it = skipJunk(currentMsg, it, end);
                    goto checkEos;
                }
                data[i] = (l << 4) | r;
                it += 2;
            }
            if (*it != '\r') {
                it = skipJunk(currentMsg, it, end);
                break;
            } else {
                it++;
            }
            base().handleData(address, data, dataSize);
            break;
        }
        default:
            it = skipJunk(currentMsg, it, end);
        }
checkEos:
        if (it == end) {
            _buffer.clear();
            return;
        }
    }
}
}
