#pragma once

#include <stdint.h>
#include <cassert>

namespace dtacan {

inline char nibbleToChar(uint8_t nibble)
{
    switch (nibble) {
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    case 10:
        return 'A';
    case 11:
        return 'B';
    case 12:
        return 'C';
    case 13:
        return 'D';
    case 14:
        return 'E';
    case 15:
        return 'F';
    }
    assert(false && "invalid nibble");
}

inline uint8_t charToNibble(char c)
{
    switch (c) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'A':
        return 10;
    case 'B':
        return 11;
    case 'C':
        return 12;
    case 'D':
        return 13;
    case 'E':
        return 14;
    case 'F':
        return 15;
    }
    return 0xff;
}

inline void encodeHexByte(uint8_t byte, char* dest)
{
    dest[0] = nibbleToChar((byte & 0xf0) >> 4);
    dest[1] = nibbleToChar((byte & 0x0f));
}

inline void encodeAddress(uint32_t address, char* dest)
{
    dest[0] = nibbleToChar((address & 0x00000f00) >> 8);
    dest[1] = nibbleToChar((address & 0x000000f0) >> 4);
    dest[2] = nibbleToChar((address & 0x0000000f));
}

inline void encodeExtendedAddress(uint32_t address, char* dest)
{
    dest[0] = nibbleToChar((address & 0xf0000000) >> 28);
    dest[1] = nibbleToChar((address & 0x0f000000) >> 24);
    dest[2] = nibbleToChar((address & 0x00f00000) >> 20);
    dest[3] = nibbleToChar((address & 0x000f0000) >> 16);
    dest[4] = nibbleToChar((address & 0x0000f000) >> 12);
    dest[5] = nibbleToChar((address & 0x00000f00) >> 8);
    dest[6] = nibbleToChar((address & 0x000000f0) >> 4);
    dest[7] = nibbleToChar((address & 0x0000000f));
}

inline void encodeHexStream(const uint8_t* data, char* dest, std::size_t size)
{
    for (std::size_t i = 0; i < size; i++) {
        encodeHexByte(data[i], dest + i * 2);
    }
}
}
