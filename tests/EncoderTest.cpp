#include "dtacan/Encoder.h"
#include "dtacan/StringEncoder.h"

#include <gtest/gtest.h>

using namespace dtacan;

class EncoderTest : public ::testing::Test {
protected:
    void SetUp()
    {
        clear();
    }

    void clear()
    {
        _encoder.result().clear();
    }

    template <std::size_t n>
    void transmitData(uint32_t address, uint8_t (&array)[n])
    {
        ASSERT_TRUE(_encoder.transmitData(address, array, n));
    }

    template <std::size_t n>
    void transmitStdFrame(uint32_t address, uint8_t (&array)[n])
    {
        ASSERT_TRUE(_encoder.transmitStdFrame(address, array, n));
    }

    template <std::size_t n>
    void transmitExtFrame(uint32_t address, uint8_t (&array)[n])
    {
        ASSERT_TRUE(_encoder.transmitExtFrame(address, array, n));
    }

    void transmitStdEmpty(uint32_t address)
    {
        ASSERT_TRUE(_encoder.transmitStdFrame(address, nullptr, 0));
    }

    void transmitExtEmpty(uint32_t address)
    {
        ASSERT_TRUE(_encoder.transmitExtFrame(address, nullptr, 0));
    }

    void setBaudRate(BaudRate baud)
    {
        _encoder.setBaudrate(baud);
    }

    void expectData(const char* data)
    {
        EXPECT_EQ(data, _encoder.result());
    }

    StringEncoder _encoder;
};

TEST_F(EncoderTest, stdFrameEmpty)
{
    transmitStdEmpty(0x7ff);
    expectData("t7FF0\r");
}

TEST_F(EncoderTest, stdFrameOneMsg)
{
    uint8_t data[] = {0xaa, 0xbb, 0xcc};
    transmitStdFrame(0x0f0, data);
    expectData("t0F03AABBCC\r");
}

TEST_F(EncoderTest, stdFrameOneFullMsg)
{
    uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22};
    transmitStdFrame(0x111, data);
    expectData("t1118AABBCCDDEEFF1122\r");
}

TEST_F(EncoderTest, stdFrameTwoMsgs)
{
    uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22, 0x24, 0x56};
    transmitData(0x033, data);
    expectData("t0338AABBCCDDEEFF1122\rt03322456\r");
}

TEST_F(EncoderTest, stdFrameTwoFullMsgs)
{
    uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22, 0x22, 0x11, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa};
    transmitData(0x056, data);
    expectData("t0568AABBCCDDEEFF1122\rt05682211FFEEDDCCBBAA\r");
}

TEST_F(EncoderTest, extFrameEmpty)
{
    transmitExtEmpty(0x1fffffff);
    expectData("T1FFFFFFF0\r");
}

TEST_F(EncoderTest, extFrameOneMsg)
{
    uint8_t data[] = {0xf6, 0x26, 0x91};
    transmitExtFrame(0x000008ff, data);
    expectData("T000008FF3F62691\r");
}

TEST_F(EncoderTest, extFrameOneFullMsg)
{
    uint8_t data[] = {0x22, 0x11, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa};
    transmitExtFrame(0x10203040, data);
    expectData("T1020304082211FFEEDDCCBBAA\r");
}

TEST_F(EncoderTest, extFrameTwoMsgs)
{
    uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22, 0x24, 0x56};
    transmitData(0x02020202, data);
    expectData("T020202028AABBCCDDEEFF1122\rT0202020222456\r");
}

TEST_F(EncoderTest, extFrameTwoFullMsgs)
{
    uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22, 0x22, 0x11, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa};
    transmitData(0x10000000, data);
    expectData("T100000008AABBCCDDEEFF1122\rT1000000082211FFEEDDCCBBAA\r");
}

TEST_F(EncoderTest, baud10k)
{
    setBaudRate(BaudRate::Baud10k);
    expectData("S0\r");
}

TEST_F(EncoderTest, baud20k)
{
    setBaudRate(BaudRate::Baud20k);
    expectData("S1\r");
}

TEST_F(EncoderTest, baud50k)
{
    setBaudRate(BaudRate::Baud50k);
    expectData("S2\r");
}

TEST_F(EncoderTest, baud100k)
{
    setBaudRate(BaudRate::Baud100k);
    expectData("S3\r");
}

TEST_F(EncoderTest, baud125k)
{
    setBaudRate(BaudRate::Baud125k);
    expectData("S4\r");
}

TEST_F(EncoderTest, baud250k)
{
    setBaudRate(BaudRate::Baud250k);
    expectData("S5\r");
}

TEST_F(EncoderTest, baud500k)
{
    setBaudRate(BaudRate::Baud500k);
    expectData("S6\r");
}

TEST_F(EncoderTest, baud800k)
{
    setBaudRate(BaudRate::Baud800k);
    expectData("S7\r");
}

TEST_F(EncoderTest, baud1M)
{
    setBaudRate(BaudRate::Baud1M);
    expectData("S8\r");
}

TEST_F(EncoderTest, open)
{
    _encoder.openCanChannel();
    expectData("O\r");
}

TEST_F(EncoderTest, close)
{
    _encoder.closeCanChannel();
    expectData("C\r");
}
