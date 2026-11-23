
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "scan.hpp"
// нейротесты

// Успешное сканирование одного значения
template <typename T>
void checkScan(std::string_view input, std::string_view fmt, std::remove_cv_t<T> expected) {
    auto result = stdx::scan<T>(input, fmt);

    ASSERT_TRUE(result.has_value());

    const auto &actual = std::get<0>(result->values());

    if constexpr (std::is_floating_point_v<std::remove_cv_t<T>>) {
        EXPECT_DOUBLE_EQ(actual, expected);
    } else {
        EXPECT_EQ(actual, expected);
    }
}
// Проверка ошибки
template <typename... Ts>
void checkScanError(std::string_view input, std::string_view fmt) {
    auto result = stdx::scan<Ts...>(input, fmt);
    EXPECT_FALSE(result.has_value());
}

TEST(ScanTest, SignedIntegers) {
    checkScan<int8_t>("-128", "{%d}", -128);
    checkScan<int16_t>("-32768", "{%d}", -32768);
    checkScan<int32_t>("123456", "{%d}", 123456);
    checkScan<int64_t>("-12345678900", "{%d}", -12345678900LL);
}

TEST(ScanTest, UnsignedIntegers) {
    checkScan<uint8_t>("255", "{%u}", 255);
    checkScan<uint16_t>("65535", "{%u}", 65535);
    checkScan<uint32_t>("123456", "{%u}", 123456u);
    checkScan<uint64_t>("18446744073709551615", "{%u}", 18446744073709551615ULL);
}

TEST(ScanTest, FloatingPoint) {
    checkScan<float>("3.5", "{%f}", 3.5f);
    checkScan<double>("-12.25", "{%f}", -12.25);
    checkScan<double>("0.0", "{%f}", 0.0);
}

TEST(ScanTest, Strings) {
    checkScan<std::string>("Hello", "{%s}", "Hello");
    checkScan<std::string_view>("World", "{%s}", "World");
    checkScan<std::string>("", "{%s}", "");
    checkScan<std::string>("Hello World", "{%s}", "Hello World");
}

TEST(ScanTest, CvQualifiedTypes) {
    checkScan<const int32_t>("42", "{}", 42);
    checkScan<volatile uint16_t>("100", "{%u}", 100);
    checkScan<const volatile int8_t>("-10", "{%d}", -10);
    checkScan<const double>("2.5", "{%f}", 2.5);
    checkScan<const std::string>("Hello", "{}", "Hello");
}

TEST(ScanTest, EmptyPlaceholders) {
    checkScan<int>("42", "{}", 42);
    checkScan<double>("3.14", "{}", 3.14);
    checkScan<std::string>("Alex", "{}", "Alex");
    checkScan<uint8_t>("255", "{}", 255);
}

TEST(ScanTest, MultipleValues) {
    auto result = stdx::scan<int, double, std::string>("Number: 42, Price: 3.5, Name: Alex",
                                                       "Number: {%d}, Price: {%f}, Name: {%s}");

    ASSERT_TRUE(result.has_value());

    auto [number, price, name] = result->values();

    EXPECT_EQ(number, 42);
    EXPECT_DOUBLE_EQ(price, 3.5);
    EXPECT_EQ(name, "Alex");
}

TEST(ScanTest, MixedPlaceholders) {
    auto result = stdx::scan<int, std::string, double>("42 Alex 3.5", "{} {} {}");

    ASSERT_TRUE(result.has_value());

    auto [number, name, price] = result->values();

    EXPECT_EQ(number, 42);
    EXPECT_EQ(name, "Alex");
    EXPECT_DOUBLE_EQ(price, 3.5);
}

TEST(ScanTest, WrongSpecifiers) {
    checkScanError<int>("42", "{%s}");
    checkScanError<double>("3.14", "{%d}");
    checkScanError<uint32_t>("42", "{%d}");
    checkScanError<std::string>("Hello", "{%f}");
    checkScanError<int>("42", "{%x}");
}

TEST(ScanTest, InvalidNumbers) {
    checkScanError<int>("abc", "{%d}");
    checkScanError<int>("12abc", "{%d}");
    checkScanError<int>("", "{%d}");
    checkScanError<uint32_t>("-42", "{%u}");
    checkScanError<double>("1.5abc", "{%f}");
}

TEST(ScanTest, Overflow) {
    checkScanError<int8_t>("128", "{%d}");
    checkScanError<int8_t>("-129", "{%d}");
    checkScanError<uint8_t>("256", "{%u}");
    checkScanError<uint16_t>("65536", "{%u}");
    checkScanError<int64_t>("999999999999999999999", "{%d}");
}

TEST(ScanTest, WrongArgumentCount) {
    checkScanError<int, double>("42", "{%d}");
    checkScanError<int>("42 3.14", "{%d} {%f}");
    checkScanError<int, double>("42 3.14", "{%d}");
}

TEST(ScanTest, WrongLiteralText) {
    checkScanError<int>("Age: 42", "Number: {%d}");
    checkScanError<int>("Number: 42!", "Number: {%d}");
    checkScanError<int>("42", "Value: {%d}");
}

TEST(ScanTest, InvalidFormat) {
    checkScanError<int>("42", "{%d");
    checkScanError<int>("42", "%d}");
    checkScanError<int>("42", "{%z}");
}

TEST(ScanTest, ErrorInSecondValue) {
    checkScanError<int, double>("Number: 42, Price: abc", "Number: {%d}, Price: {%f}");
}
TEST(ScanTest, SupportedTypes) {
    using stdx::details::is_supported;

    // Разрешённые типы
    static_assert(is_supported<int8_t>);
    static_assert(is_supported<int16_t>);
    static_assert(is_supported<int32_t>);
    static_assert(is_supported<int64_t>);
    static_assert(is_supported<uint8_t>);
    static_assert(is_supported<uint16_t>);
    static_assert(is_supported<uint32_t>);
    static_assert(is_supported<uint64_t>);
    static_assert(is_supported<float>);
    static_assert(is_supported<double>);
    static_assert(is_supported<std::string>);
    static_assert(is_supported<std::string_view>);

    // cv-квалификаторы
    static_assert(is_supported<const int>);
    static_assert(is_supported<volatile double>);
    static_assert(is_supported<const volatile int>);

    // Запрещённые типы
    static_assert(!is_supported<bool>);
    static_assert(!is_supported<char>);
    static_assert(!is_supported<long double>);
    static_assert(!is_supported<int &>);
    static_assert(!is_supported<double &&>);
    static_assert(!is_supported<void>);
    static_assert(!is_supported<int *>);
}