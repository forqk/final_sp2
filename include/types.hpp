#pragma once
#include <string>
#include <tuple>

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования


template <typename T>
constexpr bool is_supported =
    std::is_same_v<std::remove_cv_t<T>, int8_t> ||
    std::is_same_v<std::remove_cv_t<T>, int16_t> ||
    std::is_same_v<std::remove_cv_t<T>, int32_t> ||
    std::is_same_v<std::remove_cv_t<T>, int64_t> ||
    std::is_same_v<std::remove_cv_t<T>, uint8_t> ||
    std::is_same_v<std::remove_cv_t<T>, uint16_t> ||
    std::is_same_v<std::remove_cv_t<T>, uint32_t> ||
    std::is_same_v<std::remove_cv_t<T>, uint64_t> ||
    std::is_same_v<std::remove_cv_t<T>, float> ||
    std::is_same_v<std::remove_cv_t<T>, double> ||
    std::is_same_v<std::remove_cv_t<T>, std::string_view> ||
    std::is_same_v<std::remove_cv_t<T>, std::string>;

struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования
template <typename... Ts>
struct scan_result {
    // здесь ваш код
public:
    scan_result(std::tuple<Ts...> values) : types_(std::move(values)) {}
    std::tuple<Ts...> &values() { return types_; }

private:
    std::tuple<Ts...> types_;
};

}  // namespace stdx::details
