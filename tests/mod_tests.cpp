#include "dmc_rengine/formats/mod.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace {

template <typename T>
void put(std::vector<std::byte>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

// The smallest document `RelocatedModelShell` accepts: one group declaring
// zero batches, which is a legitimate structural state (the shell's own
// contract distinguishes "no batches" from "truncated"), not a rejected one.
[[nodiscard]] std::vector<std::byte> one_empty_group_fixture() {
    std::vector<std::byte> bytes(0x80U, std::byte{0});
    bytes[0] = std::byte{'M'};
    bytes[1] = std::byte{'O'};
    bytes[2] = std::byte{'D'};
    bytes[0x10U] = std::byte{1}; // group_count
    put<std::uint64_t>(bytes, 0x20U, 0U); // document_pointer, within bounds
    put<std::uint32_t>(bytes, 0x40U, 0U); // group 0 batch_count
    return bytes;
}

void test_empty_bytes_are_rejected() {
    using namespace dmc::rengine::formats;
    const auto result = ModParser::parse({});
    assert(!result.ok());
    assert(result.error == ModParseError::shell_rejected);
}

void test_wrong_magic_is_rejected() {
    using namespace dmc::rengine::formats;
    auto bytes = one_empty_group_fixture();
    bytes[0] = std::byte{'X'};
    const auto result = ModParser::parse(bytes);
    assert(!result.ok());
    assert(result.error == ModParseError::shell_rejected);
}

void test_one_empty_group_document_is_accepted() {
    using namespace dmc::rengine::formats;
    const auto bytes = one_empty_group_fixture();
    const auto result = ModParser::parse(bytes);
    assert(result.ok());
    assert(result.document->group_count == 1U);
    assert(result.document->groups.size() == 1U);
    assert(result.document->groups.front().batch_count == 0U);
    assert(result.document->batches.empty());
    assert(result.document->valid());
    assert(result.document->total_vertex_count() == 0U);
}

void test_truncated_group_table_is_rejected() {
    using namespace dmc::rengine::formats;
    auto bytes = one_empty_group_fixture();
    bytes.resize(0x50U); // group table for one group needs to reach 0x80
    const auto result = ModParser::parse(bytes);
    assert(!result.ok());
    assert(result.error == ModParseError::shell_rejected);
}

} // namespace

int main() {
    test_empty_bytes_are_rejected();
    test_wrong_magic_is_rejected();
    test_one_empty_group_document_is_accepted();
    test_truncated_group_table_is_rejected();
    return 0;
}
