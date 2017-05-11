#pragma once

#include "dtacan/Util.h"
#include "dtacan/BaudRate.h"

#include <cstddef>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <stdint.h>

namespace dtacan {

template <typename B>
class Encoder {
public:
    void handleEncodedData(const char* str, std::size_t size);

    void openCanChannel();
    void closeCanChannel();
    void setBaudrate(BaudRate rate);
    bool transmitData(uint32_t address, const void* data, std::size_t size);
    bool transmitStdFrame(uint32_t address, const void* data, std::size_t size);
    bool transmitExtFrame(uint32_t address, const void* data, std::size_t size);

private:
    void encodeStdFrame(uint32_t address, const void* data, std::size_t size);
    void encodeExtFrame(uint32_t address, const void* data, std::size_t size);
    char baudRateToChar(BaudRate baud);

    B& base();
};

template <typename B>
inline B& Encoder<B>::base()
{
    return *static_cast<B*>(this);
}

template <typename B>
inline void Encoder<B>::handleEncodedData(const char* str, std::size_t size)
{
    (void)str;
    (void)size;
}

template <typename B>
char Encoder<B>::baudRateToChar(BaudRate rate)
{
    switch (rate) {
    case BaudRate::Baud10k:
        return '0';
    case BaudRate::Baud20k:
        return '1';
    case BaudRate::Baud50k:
        return '2';
    case BaudRate::Baud100k:
        return '3';
    case BaudRate::Baud125k:
        return '4';
    case BaudRate::Baud250k:
        return '5';
    case BaudRate::Baud500k:
        return '6';
    case BaudRate::Baud800k:
        return '7';
    case BaudRate::Baud1M:
        return '8';
    }
    assert(false && "invalid baudrate");
}

template <typename B>
void Encoder<B>::setBaudrate(BaudRate rate)
{
    char data[3];
    data[0] = 'S';
    data[1] = baudRateToChar(rate);
    data[2] = '\r';
    base().handleEncodedData(data, 3);
}

template <typename B>
void Encoder<B>::openCanChannel()
{
    base().handleEncodedData("O\r", 2);
}

template <typename B>
void Encoder<B>::closeCanChannel()
{
    base().handleEncodedData("C\r", 2);
}

template <typename B>
void Encoder<B>::encodeStdFrame(uint32_t address, const void* data, std::size_t size)
{
    char msg[22];
    msg[0] = 't';
    encodeAddress(address, msg + 1);
    msg[4] = '0' + size;
    encodeHexStream((const uint8_t*)data, msg + 5, size);
    msg[5 + size * 2] = '\r';
    base().handleEncodedData(msg, 5 + size * 2 + 1);
}

template <typename B>
void Encoder<B>::encodeExtFrame(uint32_t address, const void* data, std::size_t size)
{
    char msg[27];
    msg[0] = 'T';
    encodeExtendedAddress(address, msg + 1);
    msg[9] = '0' + size;
    encodeHexStream((const uint8_t*)data, msg + 10, size);
    msg[10 + size * 2] = '\r';
    base().handleEncodedData(msg, 10 + size * 2 + 1);
}

template <typename B>
bool Encoder<B>::transmitStdFrame(uint32_t address, const void* data, std::size_t size)
{
    if (size > 8 || address > 0x7ff) {
        return false;
    }
    encodeStdFrame(address, data, size);
    return true;
}

template <typename B>
bool Encoder<B>::transmitExtFrame(uint32_t address, const void* data, std::size_t size)
{
    if (size > 8 || address > 0x1fffffff) {
        return false;
    }
    encodeExtFrame(address, data, size);
    return true;
}

template <typename B>
bool Encoder<B>::transmitData(uint32_t address, const void* data, std::size_t size)
{
    //TODO: refact
    if (size <= 8) {
        if (address > 0x1fffffff) {
            return false;
        } else if (address > 0x7ff) {
            encodeExtFrame(address, data, size);
            return true;
        }
        encodeStdFrame(address, data, size);
        return true;
    }

    char prefix;
    std::size_t streamSize;
    std::size_t fullMsgNum = size / 8;
    std::size_t lastMsgDataSize = size % 8;
    std::size_t addressSize;
    char hexAddress[8];
    if (address > 0x1fffffff) {
        return false;
    } else if (address > 0x7ff) {
        prefix = 'T';
        addressSize = 8;
        encodeExtendedAddress(address, hexAddress);
        streamSize = fullMsgNum * 27;
        if (lastMsgDataSize) {
            streamSize += 11 + lastMsgDataSize * 2;
        }
    } else {
        prefix = 't';
        addressSize = 3;
        encodeAddress(address, hexAddress);
        streamSize = fullMsgNum * 22;
        if (lastMsgDataSize) {
            streamSize += 6 + lastMsgDataSize * 2;
        }
    }

    char* msg = (char*)std::malloc(streamSize);
    char* cur = msg;
    const uint8_t* ptr = (const uint8_t*)data;

    for (std::size_t i = 0; i < fullMsgNum; i++) {
        cur[0] = prefix;
        cur += 1;

        std::memcpy(cur, hexAddress, addressSize);
        cur += addressSize;

        *cur = '8';
        cur++;

        encodeHexStream(ptr, cur, 8);
        cur[16] = '\r';

        cur += 17;
        ptr += 8;
    }

    if (lastMsgDataSize) {
        cur[0] = prefix;
        cur += 1;

        std::memcpy(cur, hexAddress, addressSize);
        cur += addressSize;

        *cur = '0' + lastMsgDataSize;
        cur++;

        encodeHexStream(ptr, cur, lastMsgDataSize);
        cur[lastMsgDataSize * 2] = '\r';
    }

    base().handleEncodedData(msg, streamSize);
    std::free(msg);
    return true;
}
}
