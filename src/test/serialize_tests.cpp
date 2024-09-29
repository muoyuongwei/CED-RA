// Copyright (c) 2012-2016 The Bitcoin Core developers
// Copyright (c) 2019 Bitcoin Association
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#include "blockencodings.h"
#include "hash.h"
#include "serialize.h"
#include "streams.h"
#include "test/test_bitcoin.h"
#include "protocol.h"

#include <cstdint>
#include <limits>

#include <boost/test/unit_test.hpp>
namespace
{
    // Helper method that checks serialisation/deserialisation works for the
    // largest value of the given unsigned and signed types.
    template<typename Unsigned, typename Signed>
    void TestLimitMax(CDataStream& ss)
    {
        ss.clear();
        ss << VARINT(std::numeric_limits<Unsigned>::max());
        Unsigned j {0};
        ss >> VARINT(j);
        BOOST_CHECK_EQUAL(j, std::numeric_limits<Unsigned>::max());
        ss.clear();
        ss << VARINT(std::numeric_limits<Signed>::max());
        Signed k {0};
        ss >> VARINT(k);
        BOOST_CHECK_EQUAL(k, std::numeric_limits<Signed>::max());
    }
}

BOOST_FIXTURE_TEST_SUITE(serialize_tests, BasicTestingSetup)

class CSerializeMethodsTestSingle {
protected:
    int intval;
    bool boolval;
    std::string stringval;
    const char *charstrval;
    CTransactionRef txval;

public:
    CSerializeMethodsTestSingle() = default;
    CSerializeMethodsTestSingle(int intvalin, bool boolvalin,
                                std::string stringvalin,
                                const char *charstrvalin, CTransaction txvalin)
        : intval(intvalin), boolval(boolvalin),
          stringval(std::move(stringvalin)), charstrval(charstrvalin),
          txval(MakeTransactionRef(txvalin)) {}
    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream &s, Operation ser_action) {
        READWRITE(intval);
        READWRITE(boolval);
        READWRITE(stringval);
        READWRITE(FLATDATA(charstrval));
        READWRITE(txval);
    }

    bool operator==(const CSerializeMethodsTestSingle &rhs) const {
        return intval == rhs.intval && boolval == rhs.boolval &&
               stringval == rhs.stringval &&
               strcmp(charstrval, rhs.charstrval) == 0 && *txval == *rhs.txval;
    }
};

class CSerializeMethodsTestMany : public CSerializeMethodsTestSingle {
public:
    using CSerializeMethodsTestSingle::CSerializeMethodsTestSingle;
    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream &s, Operation ser_action) {
        READWRITEMANY(intval, boolval, stringval, FLATDATA(charstrval), txval);
    }
};

BOOST_AUTO_TEST_CASE(sizes) {
    BOOST_CHECK_EQUAL(sizeof(char), GetSerializeSize(char(0), 0));
    BOOST_CHECK_EQUAL(sizeof(int8_t), GetSerializeSize(int8_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(uint8_t), GetSerializeSize(uint8_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(int16_t), GetSerializeSize(int16_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(uint16_t), GetSerializeSize(uint16_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(int32_t), GetSerializeSize(int32_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(uint32_t), GetSerializeSize(uint32_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(int64_t), GetSerializeSize(int64_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(uint64_t), GetSerializeSize(uint64_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(float), GetSerializeSize(float(0), 0));
    BOOST_CHECK_EQUAL(sizeof(double), GetSerializeSize(double(0), 0));
    // Bool is serialized as char
    BOOST_CHECK_EQUAL(sizeof(char), GetSerializeSize(bool(0), 0));

    // Sanity-check GetSerializeSize and c++ type matching
    BOOST_CHECK_EQUAL(GetSerializeSize(char(0), 0), 1U);
    BOOST_CHECK_EQUAL(GetSerializeSize(int8_t(0), 0), 1U);
    BOOST_CHECK_EQUAL(GetSerializeSize(uint8_t(0), 0), 1U);
    BOOST_CHECK_EQUAL(GetSerializeSize(int16_t(0), 0), 2U);
    BOOST_CHECK_EQUAL(GetSerializeSize(uint16_t(0), 0), 2U);
    BOOST_CHECK_EQUAL(GetSerializeSize(int32_t(0), 0), 4U);
    BOOST_CHECK_EQUAL(GetSerializeSize(uint32_t(0), 0), 4U);
    BOOST_CHECK_EQUAL(GetSerializeSize(int64_t(0), 0), 8U);
    BOOST_CHECK_EQUAL(GetSerializeSize(uint64_t(0), 0), 8U);
    BOOST_CHECK_EQUAL(GetSerializeSize(float(0), 0), 4U);
    BOOST_CHECK_EQUAL(GetSerializeSize(double(0), 0), 8U);
    BOOST_CHECK_EQUAL(GetSerializeSize(bool(0), 0), 1U);
}

BOOST_AUTO_TEST_CASE(floats_conversion) {
    // Choose values that map unambiguously to binary floating point to avoid
    // rounding issues at the compiler side.
    BOOST_CHECK_EQUAL(ser_uint32_to_float(0x00000000), 0.0F);
    BOOST_CHECK_EQUAL(ser_uint32_to_float(0x3f000000), 0.5F);
    BOOST_CHECK_EQUAL(ser_uint32_to_float(0x3f800000), 1.0F);
    BOOST_CHECK_EQUAL(ser_uint32_to_float(0x40000000), 2.0F);
    BOOST_CHECK_EQUAL(ser_uint32_to_float(0x40800000), 4.0F);
    BOOST_CHECK_EQUAL(ser_uint32_to_float(0x44444444), 785.066650390625F);

    BOOST_CHECK_EQUAL(ser_float_to_uint32(0.0F), 0x00000000U);
    BOOST_CHECK_EQUAL(ser_float_to_uint32(0.5F), 0x3f000000U);
    BOOST_CHECK_EQUAL(ser_float_to_uint32(1.0F), 0x3f800000U);
    BOOST_CHECK_EQUAL(ser_float_to_uint32(2.0F), 0x40000000U);
    BOOST_CHECK_EQUAL(ser_float_to_uint32(4.0F), 0x40800000U);
    BOOST_CHECK_EQUAL(ser_float_to_uint32(785.066650390625F), 0x44444444U);
}

BOOST_AUTO_TEST_CASE(doubles_conversion) {
    // Choose values that map unambiguously to binary floating point to avoid
    // rounding issues at the compiler side.
    BOOST_CHECK_EQUAL(ser_uint64_to_double(0x0000000000000000ULL), 0.0);
    BOOST_CHECK_EQUAL(ser_uint64_to_double(0x3fe0000000000000ULL), 0.5);
    BOOST_CHECK_EQUAL(ser_uint64_to_double(0x3ff0000000000000ULL), 1.0);
    BOOST_CHECK_EQUAL(ser_uint64_to_double(0x4000000000000000ULL), 2.0);
    BOOST_CHECK_EQUAL(ser_uint64_to_double(0x4010000000000000ULL), 4.0);
    BOOST_CHECK_EQUAL(ser_uint64_to_double(0x4088888880000000ULL),
                      785.066650390625);

    BOOST_CHECK_EQUAL(ser_double_to_uint64(0.0), 0x0000000000000000ULL);
    BOOST_CHECK_EQUAL(ser_double_to_uint64(0.5), 0x3fe0000000000000ULL);
    BOOST_CHECK_EQUAL(ser_double_to_uint64(1.0), 0x3ff0000000000000ULL);
    BOOST_CHECK_EQUAL(ser_double_to_uint64(2.0), 0x4000000000000000ULL);
    BOOST_CHECK_EQUAL(ser_double_to_uint64(4.0), 0x4010000000000000ULL);
    BOOST_CHECK_EQUAL(ser_double_to_uint64(785.066650390625),
                      0x4088888880000000ULL);
}
/*
Python code to generate the below hashes:

    def reversed_hex(x):
        return binascii.hexlify(''.join(reversed(x)))
    def dsha256(x):
        return hashlib.sha256(hashlib.sha256(x).digest()).digest()

    reversed_hex(dsha256(''.join(struct.pack('<f', x) for x in range(0,1000))))
== '8e8b4cf3e4df8b332057e3e23af42ebc663b61e0495d5e7e32d85099d7f3fe0c'
    reversed_hex(dsha256(''.join(struct.pack('<d', x) for x in range(0,1000))))
== '43d0c82591953c4eafe114590d392676a01585d25b25d433557f0d7878b23f96'
*/
BOOST_AUTO_TEST_CASE(floats) {
    CDataStream ss(SER_DISK, 0);
    // encode
    for (int i = 0; i < 1000; i++) {
        ss << float(i);
    }
    BOOST_CHECK(Hash(ss.begin(), ss.end()) ==
                uint256S("8e8b4cf3e4df8b332057e3e23af42ebc663b61e0495d5e7e32d85"
                         "099d7f3fe0c"));

    // decode
    for (int i = 0; i < 1000; i++) {
        float j;
        ss >> j;
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }
}

BOOST_AUTO_TEST_CASE(doubles) {
    CDataStream ss(SER_DISK, 0);
    // encode
    for (int i = 0; i < 1000; i++) {
        ss << double(i);
    }
    BOOST_CHECK(Hash(ss.begin(), ss.end()) ==
                uint256S("43d0c82591953c4eafe114590d392676a01585d25b25d433557f0"
                         "d7878b23f96"));

    // decode
    for (int i = 0; i < 1000; i++) {
        double j;
        ss >> j;
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }
}

BOOST_AUTO_TEST_CASE(varints) {
    // encode

    CDataStream ss(SER_DISK, 0);
    CDataStream::size_type size = 0;
    for (int i = 0; i < 100000; i++) {
        ss << VARINT(i);
        size += ::GetSerializeSize(VARINT(i), 0, 0);
        BOOST_CHECK(size == ss.size());
    }

    for (uint64_t i = 0; i < 100000000000ULL; i += 999999937) {
        ss << VARINT(i);
        size += ::GetSerializeSize(VARINT(i), 0, 0);
        BOOST_CHECK(size == ss.size());
    }

    // decode
    for (int i = 0; i < 100000; i++) {
        int j = -1;
        ss >> VARINT(j);
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }

    for (uint64_t i = 0; i < 100000000000ULL; i += 999999937) {
        uint64_t j = -1;
        ss >> VARINT(j);
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }

        // Serialise/deserialise largest fixed size types
        TestLimitMax<uint8_t, int8_t>(ss);
        TestLimitMax<uint16_t, int16_t>(ss);
        TestLimitMax<uint32_t, int32_t>(ss);
        TestLimitMax<uint64_t, int64_t>(ss);
        TestLimitMax<uintmax_t, intmax_t>(ss);

    {
        // Deserialising a larger value than can fit into any integral type
        ss.clear();
        ss.insert(ss.end(), 64, char(0x80));
        uint32_t j = 0;
        BOOST_CHECK_THROW({ss >> VARINT(j);}, std::runtime_error);
    }

    {
        // Deserialising a larger value than can fit into the given type
        ss.clear();
        ss.insert(ss.end(), 4, char(0xFF));
        uint16_t j = 0;
        BOOST_CHECK_THROW({ss >> VARINT(j);}, std::runtime_error);
    }
}

BOOST_AUTO_TEST_CASE(varints_bitpatterns) {
    CDataStream ss(SER_DISK, 0);
    ss << VARINT(0);
    BOOST_CHECK_EQUAL(HexStr(ss), "00");
    ss.clear();
    ss << VARINT(0x7f);
    BOOST_CHECK_EQUAL(HexStr(ss), "7f");
    ss.clear();
    ss << VARINT((int8_t)0x7f);
    BOOST_CHECK_EQUAL(HexStr(ss), "7f");
    ss.clear();
    ss << VARINT(0x80);
    BOOST_CHECK_EQUAL(HexStr(ss), "8000");
    ss.clear();
    ss << VARINT((uint8_t)0x80);
    BOOST_CHECK_EQUAL(HexStr(ss), "8000");
    ss.clear();
    ss << VARINT(0x1234);
    BOOST_CHECK_EQUAL(HexStr(ss), "a334");
    ss.clear();
    ss << VARINT((int16_t)0x1234);
    BOOST_CHECK_EQUAL(HexStr(ss), "a334");
    ss.clear();
    ss << VARINT(0xffff);
    BOOST_CHECK_EQUAL(HexStr(ss), "82fe7f");
    ss.clear();
    ss << VARINT((uint16_t)0xffff);
    BOOST_CHECK_EQUAL(HexStr(ss), "82fe7f");
    ss.clear();
    ss << VARINT(0x123456);
    BOOST_CHECK_EQUAL(HexStr(ss), "c7e756");
    ss.clear();
    ss << VARINT((int32_t)0x123456);
    BOOST_CHECK_EQUAL(HexStr(ss), "c7e756");
    ss.clear();
    ss << VARINT(0x80123456U);
    BOOST_CHECK_EQUAL(HexStr(ss), "86ffc7e756");
    ss.clear();
    ss << VARINT((uint32_t)0x80123456U);
    BOOST_CHECK_EQUAL(HexStr(ss), "86ffc7e756");
    ss.clear();
    ss << VARINT(0xffffffff);
    BOOST_CHECK_EQUAL(HexStr(ss), "8efefefe7f");
    ss.clear();
    ss << VARINT(0x7fffffffffffffffLL);
    BOOST_CHECK_EQUAL(HexStr(ss), "fefefefefefefefe7f");
    ss.clear();
    ss << VARINT(0xffffffffffffffffULL);
    BOOST_CHECK_EQUAL(HexStr(ss), "80fefefefefefefefe7f");
    ss.clear();
}

static bool isTooLargeWriteException(const std::ios_base::failure &ex) {
    std::ios_base::failure expectedException(
        "WriteCompactSize(): size too large");

    // The string returned by what() can be different for different platforms.
    // Instead of directly comparing the ex.what() with an expected string,
    // create an instance of exception to see if ex.what() matches  the expected
    // explanatory string returned by the exception instance.
    return strcmp(expectedException.what(), ex.what()) == 0;
}

BOOST_AUTO_TEST_CASE(compactsize) {
    CDataStream ss(SER_DISK, 0);
    std::vector<char>::size_type i, j;

    for (i = 1; i <= MAX_SIZE; i *= 2) {
        WriteCompactSize(ss, i - 1);
        WriteCompactSize(ss, i);
    }
    for (i = 1; i <= MAX_SIZE; i *= 2) {
        j = ReadCompactSize(ss);
        BOOST_CHECK_MESSAGE((i - 1) == j,
                            "decoded:" << j << " expected:" << (i - 1));
        j = ReadCompactSize(ss);
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }

    WriteCompactSize(ss, MAX_SIZE);
    BOOST_CHECK_EQUAL(ReadCompactSize(ss), MAX_SIZE);

    BOOST_CHECK_EXCEPTION(WriteCompactSize(ss, MAX_SIZE + 1), std::ios_base::failure, isTooLargeWriteException);

    BOOST_CHECK_EXCEPTION(WriteCompactSize(ss, std::numeric_limits<int64_t>::max()), std::ios_base::failure, isTooLargeWriteException);
    BOOST_CHECK_EXCEPTION(WriteCompactSize(ss, std::numeric_limits<uint64_t>::max()), std::ios_base::failure,isTooLargeWriteException);
}

static bool isCanonicalException(const std::ios_base::failure &ex) {
    std::ios_base::failure expectedException("non-canonical ReadCompactSize()");

    // The string returned by what() can be different for different platforms.
    // Instead of directly comparing the ex.what() with an expected string,
    // create an instance of exception to see if ex.what() matches  the expected
    // explanatory string returned by the exception instance.
    return strcmp(expectedException.what(), ex.what()) == 0;
}

BOOST_AUTO_TEST_CASE(noncanonical) {
    // Write some non-canonical CompactSize encodings, and make sure an
    // exception is thrown when read back.
    CDataStream ss(SER_DISK, 0);
    std::vector<char>::size_type n;

    // zero encoded with three bytes:
    ss.write("\xfd\x00\x00", 3);
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure,
                          isCanonicalException);

    // 0xfc encoded with three bytes:
    ss.write("\xfd\xfc\x00", 3);
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure,
                          isCanonicalException);

    // 0xfd encoded with three bytes is OK:
    ss.write("\xfd\xfd\x00", 3);
    n = ReadCompactSize(ss);
    BOOST_CHECK(n == 0xfd);

    // zero encoded with five bytes:
    ss.write("\xfe\x00\x00\x00\x00", 5);
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure,
                          isCanonicalException);

    // 0xffff encoded with five bytes:
    ss.write("\xfe\xff\xff\x00\x00", 5);
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure,
                          isCanonicalException);

    // zero encoded with nine bytes:
    ss.write("\xff\x00\x00\x00\x00\x00\x00\x00\x00", 9);
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure,
                          isCanonicalException);

    // 0x01ffffff encoded with nine bytes:
    ss.write("\xff\xff\xff\xff\x01\x00\x00\x00\x00", 9);
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure,
                          isCanonicalException);
}

BOOST_AUTO_TEST_CASE(insert_delete) {
    // Test inserting/deleting bytes.
    CDataStream ss(SER_DISK, 0);
    BOOST_CHECK_EQUAL(ss.size(), 0U);

    ss.write("\x00\x01\x02\xff", 4);
    BOOST_CHECK_EQUAL(ss.size(), 4U);

    char c = (char)11;

    // Inserting at beginning/end/middle:
    ss.insert(ss.begin(), c);
    BOOST_CHECK_EQUAL(ss.size(), 5U);
    BOOST_CHECK_EQUAL(ss[0], c);
    BOOST_CHECK_EQUAL(ss[1], 0);

    ss.insert(ss.end(), c);
    BOOST_CHECK_EQUAL(ss.size(), 6U);
    BOOST_CHECK_EQUAL(ss[4], (char)0xff);
    BOOST_CHECK_EQUAL(ss[5], c);

    ss.insert(ss.begin() + 2, c);
    BOOST_CHECK_EQUAL(ss.size(), 7U);
    BOOST_CHECK_EQUAL(ss[2], c);

    // Delete at beginning/end/middle
    ss.erase(ss.begin());
    BOOST_CHECK_EQUAL(ss.size(), 6U);
    BOOST_CHECK_EQUAL(ss[0], 0);

    ss.erase(ss.begin() + ss.size() - 1);
    BOOST_CHECK_EQUAL(ss.size(), 5U);
    BOOST_CHECK_EQUAL(ss[4], (char)0xff);

    ss.erase(ss.begin() + 1);
    BOOST_CHECK_EQUAL(ss.size(), 4U);
    BOOST_CHECK_EQUAL(ss[0], 0);
    BOOST_CHECK_EQUAL(ss[1], 1);
    BOOST_CHECK_EQUAL(ss[2], 2);
    BOOST_CHECK_EQUAL(ss[3], (char)0xff);

    // Make sure GetAndClear does the right thing:
    CSerializeData d;
    ss.GetAndClear(d);
    BOOST_CHECK_EQUAL(ss.size(), 0U);
}

BOOST_AUTO_TEST_CASE(class_methods) {
    int intval(100);
    bool boolval(true);
    std::string stringval("testing");
    const char *charstrval("testing charstr");
    CMutableTransaction txval;
    CSerializeMethodsTestSingle methodtest1(intval, boolval, stringval,
                                            charstrval, CTransaction(txval));
    CSerializeMethodsTestMany methodtest2(intval, boolval, stringval,
                                          charstrval, CTransaction(txval));
    CSerializeMethodsTestSingle methodtest3;
    CSerializeMethodsTestMany methodtest4;
    CDataStream ss(SER_DISK, PROTOCOL_VERSION);
    BOOST_CHECK(methodtest1 == methodtest2);
    ss << methodtest1;
    ss >> methodtest4;
    ss << methodtest2;
    ss >> methodtest3;
    BOOST_CHECK(methodtest1 == methodtest2);
    BOOST_CHECK(methodtest2 == methodtest3);
    BOOST_CHECK(methodtest3 == methodtest4);

    CDataStream ss2(SER_DISK, PROTOCOL_VERSION, intval, boolval, stringval,
                    FLATDATA(charstrval), txval);
    ss2 >> methodtest3;
    BOOST_CHECK(methodtest3 == methodtest4);
}

BOOST_AUTO_TEST_CASE(map_set_serialise) {
    // Map
    {
        std::map<int, std::string> testMap {
            { 1, "Entry1" },
            { 2, "Entry2" },
            { 3, "Entry3" },
        };

        // Serialise
        CDataStream ss(SER_DISK, 0);
        BOOST_CHECK_EQUAL(ss.size(), 0U);
        ss << testMap;

        // Deserialise
        decltype(testMap) decoded {};
        ss >> decoded;
        BOOST_CHECK(testMap == decoded);
    }

    // Unordered map
    {
        std::unordered_map<int, std::string> testMap {
            { 1, "Entry1" },
            { 2, "Entry2" },
            { 3, "Entry3" },
        };

        // Serialise
        CDataStream ss(SER_DISK, 0);
        BOOST_CHECK_EQUAL(ss.size(), 0U);
        ss << testMap;

        // Deserialise
        decltype(testMap) decoded {};
        ss >> decoded;
        BOOST_CHECK(testMap == decoded);
    }

    // Set
    {
        std::set<std::string> testSet { "Entry1", "Entry2", "Entry3" };

        // Serialise
        CDataStream ss(SER_DISK, 0);
        BOOST_CHECK_EQUAL(ss.size(), 0U);
        ss << testSet;

        // Deserialise
        decltype(testSet) decoded {};
        ss >> decoded;
        BOOST_CHECK(testSet == decoded);
    }

    // Unordered set
    {
        std::unordered_set<std::string> testSet { "Entry1", "Entry2", "Entry3" };

        // Serialise
        CDataStream ss(SER_DISK, 0);
        BOOST_CHECK_EQUAL(ss.size(), 0U);
        ss << testSet;

        // Deserialise
        decltype(testSet) decoded {};
        ss >> decoded;
        BOOST_CHECK(testSet == decoded);
    }
}

BOOST_AUTO_TEST_CASE(optional_serialise) {
    {
        std::optional<std::string> testOpt { "TestString" };

        // Serialise
        CDataStream ss(SER_DISK, 0);
        BOOST_CHECK_EQUAL(ss.size(), 0U);
        ss << testOpt;

        // Deserialise
        decltype(testOpt) decoded {};
        ss >> decoded;
        BOOST_CHECK_EQUAL(testOpt, decoded);
    }

    {
        std::optional<std::string> testOpt { std::nullopt };

        // Serialise
        CDataStream ss(SER_DISK, 0);
        BOOST_CHECK_EQUAL(ss.size(), 0U);
        ss << testOpt;

        // Deserialise
        decltype(testOpt) decoded {};
        ss >> decoded;
        BOOST_CHECK_EQUAL(testOpt, decoded);
   }
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(ser_size_tests)

BOOST_AUTO_TEST_CASE(no_args_ser_size)
{
    BOOST_CHECK_EQUAL(0U, ser_size());
}

BOOST_AUTO_TEST_CASE(txin_ser_size)
{
    BOOST_CHECK_EQUAL(41U, ser_size(CTxIn{}));

    const CTxIn cin;
    BOOST_CHECK_EQUAL(41U, ser_size(cin));
    
    CTxIn in;
    BOOST_CHECK_EQUAL(41U, ser_size(in));

    const std::vector<uint8_t> v(0xfd);
    CScript s{v.cbegin(), v.cend()};
    in.scriptSig = s;
    BOOST_CHECK_EQUAL(296U, ser_size(in));
}

BOOST_AUTO_TEST_CASE(txout_ser_size)
{
    BOOST_CHECK_EQUAL(9U, ser_size(CTxOut{}));

    const CTxOut cout;
    BOOST_CHECK_EQUAL(9U, ser_size(cout));

    CTxOut out;
    BOOST_CHECK_EQUAL(9U, ser_size(out));
    
    const std::vector<uint8_t> v(0xfd);
    CScript s{v.cbegin(), v.cend()};
    out.scriptPubKey = s;
    BOOST_CHECK_EQUAL(264U, ser_size(out));
}

BOOST_AUTO_TEST_CASE(tx_ser_size)
{
    BOOST_CHECK_EQUAL(10U, ser_size(CTransaction{}));

    const CTransaction ctx;
    BOOST_CHECK_EQUAL(10U, ser_size(ctx));

    CTransaction tx0;
    BOOST_CHECK_EQUAL(10U, ser_size(tx0));
    
    CMutableTransaction mtx1;
    mtx1.vin.resize(1);
    mtx1.vout.resize(1);
    CTransaction tx1{mtx1};
    BOOST_CHECK_EQUAL(60U, ser_size(tx1));
   
    CMutableTransaction mtx2;
    mtx2.vin.resize(0xfd);
    mtx2.vout.resize(0xfd);
    CTransaction tx2{mtx2};
    BOOST_CHECK_EQUAL(12'664U, ser_size(tx2));
}

BOOST_AUTO_TEST_CASE(btx_ser_size)
{
    BOOST_CHECK_EQUAL(33U, ser_size(BlockTransactions{}));

    const BlockTransactions cbtxs;
    BOOST_CHECK_EQUAL(33U, ser_size(cbtxs));

    BlockTransactions btxs0;
    BOOST_CHECK_EQUAL(33U, ser_size(btxs0));

    BlockTransactions btxs1;
    btxs1.txn.push_back(std::make_shared<const CTransaction>());
    BOOST_CHECK_EQUAL(43U, ser_size(btxs1));

    BlockTransactions btxs2;
    for(int i{}; i<0xfd; ++i)
        btxs2.txn.push_back(std::make_shared<const CTransaction>());
    BOOST_CHECK_EQUAL(2'565U, ser_size(btxs2));
}

BOOST_AUTO_TEST_CASE(cinv_ser_size)
{
    BOOST_CHECK_EQUAL(sizeof(CInv), ser_size(CInv{}));

    CInv cinv;
    BOOST_CHECK_EQUAL(sizeof(CInv), ser_size(cinv));

    CInv inv;
    BOOST_CHECK_EQUAL(sizeof(CInv), ser_size(inv));
    
    BOOST_CHECK_EQUAL(10*sizeof(CInv), ser_size(std::vector<CInv>(10)));
    
    const std::vector<CInv> cinvs(10);
    BOOST_CHECK_EQUAL(10*sizeof(CInv), ser_size(cinvs));

    std::vector<CInv> invs(10);
    BOOST_CHECK_EQUAL(10*sizeof(CInv), ser_size(invs));
}

BOOST_AUTO_TEST_SUITE_END();

