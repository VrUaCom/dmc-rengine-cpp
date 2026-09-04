#include "dmc_rengine/formats/efm.hpp"

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

// EFM shares the same document/group shell as MOD and SCM: a count at +0x10,
// a relocated pointer at +0x20, and a 0x40-byte group table at +0x40. The
// smallest accepted document is one group declaring zero batches.
[[nodiscard]] std::vector<std::byte> one_empty_group_fixture() {
    std::vector<std::byte> bytes(0x80U, std::byte{0});
    bytes[0] = std::byte{'E'};
    bytes[1] = std::byte{'F'};
    bytes[2] = std::byte{'M'};
    bytes[0x10U] = std::byte{1}; // group_count
    put<std::uint64_t>(bytes, 0x20U, 0U); // document_pointer, within bounds
    put<std::uint32_t>(bytes, 0x40U, 0U); // group 0 batch_count
    return bytes;
}

void test_empty_bytes_are_rejected() {
    using namespace dmc::rengine::formats;
    const auto result = EfmParser::parse({});
    assert(!result.ok());
    assert(result.error == EfmParseError::shell_rejected);
}

void test_wrong_magic_is_rejected() {
    using namespace dmc::rengine::formats;
    auto bytes = one_empty_group_fixture();
    bytes[0] = std::byte{'X'};
    const auto result = EfmParser::parse(bytes);
    assert(!result.ok());
    assert(result.error == EfmParseError::shell_rejected);
}

void test_one_empty_group_document_is_accepted() {
    using namespace dmc::rengine::formats;
    const auto bytes = one_empty_group_fixture();
    const auto result = EfmParser::parse(bytes);
    assert(result.ok());
    assert(result.document->group_count == 1U);
    assert(result.document->groups.size() == 1U);
    assert(result.document->batches.empty());
    assert(result.document->valid());
    assert(result.document->total_vertex_count() == 0U);
}

} // namespace

int main() {
    test_empty_bytes_are_rejected();
    test_wrong_magic_is_rejected();
    test_one_empty_group_document_is_accepted();
    return 0;
}
