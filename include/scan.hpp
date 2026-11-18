#pragma once

#include "parse.hpp"
#include "types.hpp"
#include <expected>
#include <iostream>

namespace stdx {

template <typename T>
auto parse_at(const std::vector<std::string_view>& input, const std::vector<std::string_view>& format, std::size_t index) {
    return details::parse_value_with_format<T>(input[index], format[index]);
}

template <typename... Ts>
auto parse_tuple(const std::vector<std::string_view>& input_parts, const std::vector<std::string_view>& format_parts) {
    std::size_t i = 0;
    return std::tuple{parse_at<Ts>(input_parts, format_parts, i++)...};
}
// замените болванку функции scan на рабочую версию
template <typename... Ts>
std::expected<details::scan_result<Ts...>, details::scan_error> scan(std::string_view input, std::string_view format) {

    static_assert((details::is_supported<Ts> && ...), "Unsupported type in scan");

    auto answer = details::parse_sources(input, format);

    if (!answer) {
        return std::unexpected(answer.error());
    }

    auto [format_parts, input_parts] = answer.value();

    if (input_parts.size() != sizeof...(Ts) || format_parts.size() != sizeof...(Ts)) {
        return std::unexpected(details::scan_error{"Types count mismatch"});
    }

    auto result = parse_tuple<Ts...>(input_parts, format_parts);

    return std::apply([](auto&... r) -> std::expected<details::scan_result<Ts...>, details::scan_error> {
        if ((!r.has_value() || ...)){
            return std::unexpected(details::scan_error{"wrong value in parse"});
        }
        return details::scan_result<Ts...>{std::tuple<Ts...>{r.value()...}};}, result);
}


}  // namespace stdx
