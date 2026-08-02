#include "dmc_rengine/core/sha256.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> as_bytes(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const auto character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

} // namespace

int main() {
    const std::vector<std::byte> empty;
    const auto empty_digest = dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{empty});
    assert(empty_digest.hex() ==
        "e3b0c44298fc1c149afbf4c8996fb924"
        "27ae41e4649b934ca495991b7852b855");

    const auto abc = as_bytes("abc");
    const auto abc_digest = dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{abc});
    assert(abc_digest.hex() ==
        "ba7816bf8f01cfea414140de5dae2223"
        "b00361a396177a9cb410ff61f20015ad");

    return 0;
}
