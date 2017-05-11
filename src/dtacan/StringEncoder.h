#pragma once

#include "dtacan/Encoder.h"

#include <string>

namespace dtacan {

class StringEncoder : public Encoder<StringEncoder> {
public:
    void handleEncodedData(const char* str, std::size_t size)
    {
        _result.append(str, size);
    }

    void clear()
    {
        _result.clear();
    }

    const std::string& result() const
    {
        return _result;
    }

    std::string& result()
    {
        return _result;
    }

private:
    std::string _result;
};
}
