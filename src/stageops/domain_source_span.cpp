#include "dmc_rengine/stageops/domain_source_span.hpp"

#include <charconv>
#include <string_view>

namespace dmc::rengine::stageops {
namespace {

template <class UInt>
[[nodiscard]] std::optional<UInt> parse_unsigned(
    const StageDomainObject& object,
    std::string_view key) noexcept {
    const auto iterator = object.attributes.find(key);
    if (iterator == object.attributes.end() || iterator->second.empty()) {
        return std::nullopt;
    }

    UInt value{};
    const auto begin = iterator->second.data();
    const auto end = begin + iterator->second.size();
    const auto parsed = std::from_chars(begin, end, value);
    if (parsed.ec != std::errc{} || parsed.ptr != end) {
        return std::nullopt;
    }
    return value;
}

} // namespace

std::optional<StageDomainSourceSpan> domain_source_span(
    const StageDomainObject& object) noexcept {
    if (!object.valid()) {
        return std::nullopt;
    }

    const auto offset = parse_unsigned<std::uint64_t>(object, "offset");
    const auto size = parse_unsigned<std::uint64_t>(object, "size");
    if (!offset.has_value() || !size.has_value() || *size == 0U) {
        return std::nullopt;
    }

    const auto line = parse_unsigned<std::uint32_t>(object, "line");
    const auto column = parse_unsigned<std::uint32_t>(object, "column");
    StageDomainSourceSpan span{
        .resource_id = object.resource_id,
        .offset = *offset,
        .size = *size,
        .line = line,
        .column = column,
        .byte_source = object.byte_source,
        .byte_revision = object.byte_revision,
    };
    return span.valid()
        ? std::optional<StageDomainSourceSpan>{std::move(span)}
        : std::nullopt;
}

} // namespace dmc::rengine::stageops
