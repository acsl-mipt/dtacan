#include "dtacan/Parser.h"

#include <gtest/gtest.h>

#include <cstring>
#include <deque>

using namespace dtacan;

struct Data {
    Data(uint32_t address, const uint8_t* data, std::size_t size)
        : address(address)
        , size(size)
    {
        this->data = new uint8_t[size];
        std::memcpy(this->data, data, size);
    }

    ~Data()
    {
        if (data) {
            delete [] data;
        }
    }

    Data& operator=(const Data& other) = delete;
    Data& operator=(Data&& other)
    {
        this->address = other.address;
        this->data = other.data;
        this->size = other.size;
        other.data = nullptr;
        return *this;
    }

    uint32_t address;
    uint8_t* data;
    std::size_t size;
};

class ParserTest : public ::testing::Test, public Parser<ParserTest> {
public:
    void SetUp() override
    {
        _data.clear();
        _junk.clear();
    }

    void handleData(uint32_t address, const uint8_t* data, std::size_t size)
    {
        _data.emplace_back(address, data, size);
    }

    void handleJunk(const uint8_t* junk, std::size_t size)
    {
        _junk.append((const char*)junk, size);
    }

    void expectJunk(const char* junk)
    {
        EXPECT_EQ(junk, _junk);
        _junk.clear();
    }

    void acceptString(const char* str)
    {
        acceptData(str, std::strlen(str));
    }

    void expectEmptyData(uint32_t address)
    {
        ASSERT_FALSE(_data.empty());
        const Data& d = _data.front();
        EXPECT_EQ(address, d.address);
        EXPECT_EQ(0u, d.size);
    }

    template <std::size_t n>
    void expectData(uint32_t address, uint8_t (&data)[n])
    {
        ASSERT_FALSE(_data.empty());
        const Data& d = _data.front();
        EXPECT_EQ(address, d.address);
        ASSERT_EQ(n, d.size);
        EXPECT_EQ(0, std::memcmp(data, d.data, n));
        _data.pop_front();
    }

protected:
    std::deque<Data> _data;
    std::string _junk;
};

TEST_F(ParserTest, stdFrameEmpty)
{
    acceptString("t0010\r");
    expectEmptyData(0x001);
}

TEST_F(ParserTest, stdFrameEmptyMaxAddress)
{
    acceptString("t7FF0\r");
    expectEmptyData(0x7ff);
}

TEST_F(ParserTest, stdFrameWrongAddress)
{
    acceptString("t8FF0\r");
    expectJunk("t8FF0");
}

TEST_F(ParserTest, stdFrameWrongDataSize)
{
    acceptString("t7FF9123\r");
    expectJunk("t7FF9123");
}

TEST_F(ParserTest, stdFrameMissingCrlf)
{
    acceptString("t7FF21233");
    expectJunk("t7FF21233");
}

TEST_F(ParserTest, stdFrame)
{
    acceptString("t7FA2AABB\r");
    uint8_t data[] = {0xaa, 0xbb};
    expectData(0x7fa, data);
}

TEST_F(ParserTest, stdFrameMaxSize)
{
    acceptString("t12381234567890ABCDEF\r");
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef};
    expectData(0x123, data);
}

TEST_F(ParserTest, stdFrameSeveral)
{
    acceptString("t1111AA\rt00021234\rt7775AABBCCDDEE\r");
    uint8_t data1[] = {0xaa};
    uint8_t data2[] = {0x12, 0x34};
    uint8_t data3[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee};
    expectData(0x111, data1);
    expectData(0x000, data2);
    expectData(0x777, data3);
}

TEST_F(ParserTest, stdFrameJunkPlusOne)
{
    acceptString("asd\rt00021234\r"); //TODO: remove \r after asd
    uint8_t data2[] = {0x12, 0x34};
    expectJunk("asd");
    expectData(0x000, data2);
}
