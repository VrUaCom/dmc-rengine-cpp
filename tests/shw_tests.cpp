#include "dmc_rengine/formats/shw.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

// No SHW payload exists in the supplied corpus, so nothing here can build a
// populated document without inventing entry semantics the recovered routine
// never gave up (see ShwContract: it relocates four bases per entry and
// indexes through none of them). What the routine's own offsets *do* support
// without invention is a document declaring zero entries — a legitimate
// structural state, not a guess about entry contents.
[[nodiscard]] std::vector<std::byte> zero_entries_fixture() {
    std::vector<std::byte> bytes(0x30U, std::byte{0}); // up to entry_table_offset
    bytes[0] = std::byte{'S'};
    bytes[1] = std::byte{'H'};
    bytes[2] = std::byte{'W'};
    bytes[0x10U] = std::byte{0}; // entry_count
    return bytes;
}

void test_empty_bytes_are_rejected() {
    using namespace dmc::rengine::formats;
    const auto result = ShwParser::parse({});
    assert(!result.ok());
    assert(result.error == ShwParseError::truncated_header);
}

void test_wrong_magic_is_rejected() {
    using namespace dmc::rengine::formats;
    auto bytes = zero_entries_fixture();
    bytes[0] = std::byte{'X'};
    const auto result = ShwParser::parse(bytes);
    assert(!result.ok());
    assert(result.error == ShwParseError::invalid_magic);
}

void test_zero_entry_document_is_accepted() {
    using namespace dmc::rengine::formats;
    const auto bytes = zero_entries_fixture();
    const auto result = ShwParser::parse(bytes);
    assert(result.ok());
    assert(result.document->entry_count == 0U);
    assert(result.document->entries.empty());
    assert(result.document->valid());
}

// One entry fits exactly when the document is exactly long enough to hold
// its table; shrinking past that must refuse rather than read past the end.
void test_one_entry_document_needs_its_full_table() {
    using namespace dmc::rengine::formats;
    std::vector<std::byte> bytes(0x70U, std::byte{0});
    bytes[0] = std::byte{'S'};
    bytes[1] = std::byte{'H'};
    bytes[2] = std::byte{'W'};
    bytes[0x10U] = std::byte{1}; // entry_count
    // Every pointer field defaults to zero, which is in bounds.
    const auto result = ShwParser::parse(bytes);
    assert(result.ok());
    assert(result.document->entries.size() == 1U);

    bytes.resize(0x38U); // shorter than entry_table_offset + entry_stride
    const auto shrunk = ShwParser::parse(bytes);
    assert(!shrunk.ok());
    assert(shrunk.error == ShwParseError::truncated_entry_table);
}

} // namespace

int main() {
    test_empty_bytes_are_rejected();
    test_wrong_magic_is_rejected();
    test_zero_entry_document_is_accepted();
    test_one_entry_document_needs_its_full_table();
    return 0;
}
