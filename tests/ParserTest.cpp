#include "dtacan/Parser.h"

#include "DtaCanTest.h"

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

    void acceptChar(char c)
    {
        acceptData(&c, 1);
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
        EXPECT_EQ_MEM(data, d.data, n);
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
    acceptString("t7FF21233t");
    expectJunk("t7FF21233t");
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

TEST_F(ParserTest, stdFrameByteByByte)
{
    for (char c : "t1111AA\rt6665AABBCCDDEE\r") { //TODO: remove \0
        acceptChar(c);
    }
    uint8_t data1[] = {0xaa};
    uint8_t data2[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee};
    expectData(0x111, data1);
    expectData(0x666, data2);
}

TEST_F(ParserTest, stdFrameJunkPlusOne)
{
    acceptString("asd\rt00021234\r"); //TODO: remove \r after asd
    uint8_t data2[] = {0x12, 0x34};
    expectJunk("asd");
    expectData(0x000, data2);
}

TEST_F(ParserTest, extFrameEmpty)
{
    acceptString("T000000010\r");
    expectEmptyData(0x00000001);
}

TEST_F(ParserTest, extFrameEmptyMaxAddress)
{
    acceptString("T1FFFFFFF0\r");
    expectEmptyData(0x1fffffff);
}

TEST_F(ParserTest, extFrameWrongAddress)
{
    acceptString("T2FFFFFFF0\r");
    expectJunk("T2FFFFFFF0");
}

TEST_F(ParserTest, extFrameWrongDataSize)
{
    acceptString("T11111111a123\r");
    expectJunk("T11111111a123");
}

TEST_F(ParserTest, extFrameMissingCrlf)
{
    acceptString("T0101010121233T");
    expectJunk("T0101010121233T");
}

TEST_F(ParserTest, extFrame)
{
    acceptString("T012345673284756\r");
    uint8_t data[] = {0x28, 0x47, 0x56};
    expectData(0x01234567, data);
}

TEST_F(ParserTest, extFrameMaxSize)
{
    acceptString("T0987654381234567890ABCDEF\r");
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef};
    expectData(0x09876543, data);
}

TEST_F(ParserTest, extFrameSeveral)
{
    acceptString("T00756483155\rT000000003098765\rT001111115AABBCCDDEE\r");
    uint8_t data1[] = {0x55};
    uint8_t data2[] = {0x09, 0x87, 0x65};
    uint8_t data3[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee};
    expectData(0x00756483, data1);
    expectData(0x00000000, data2);
    expectData(0x00111111, data3);
}

TEST_F(ParserTest, extFrameByteByByte)
{
    for (char c : "T0000044425678\rT099999996000000000000\r") {
        acceptChar(c);
    }
    uint8_t data1[] = {0x56, 0x78};
    uint8_t data2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    expectData(0x00000444, data1);
    expectData(0x09999999, data2);
}

TEST_F(ParserTest, extFrameJunkPlusOne)
{
    acceptString("xxxxxxxxx\rT00008800157\r"); //TODO: remove \r after asd
    uint8_t data2[] = {0x57};
    expectJunk("xxxxxxxxx");
    expectData(0x00008800, data2);
}
