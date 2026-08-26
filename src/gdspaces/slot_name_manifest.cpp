#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"

#include "dmc_rengine/gdspaces/text_record.hpp"

#include <algorithm>
#include <cctype>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] bool plausible_name(std::string_view line) noexcept {
    if (line.empty() || line.size() > SlotNameManifest::k_max_name_bytes) {
        return false;
    }
    // A name list is names. A line with no extension, a space, or a directive
    // marker belongs to some other kind of text block — the `# GAME` scene
    // block is text and names nothing, and must not be read as a manifest.
    if (line.find('.') == std::string_view::npos) {
        return false;
    }
    return std::all_of(line.begin(), line.end(), [](unsigned char value) {
        return std::isalnum(value) != 0 || value == '.' || value == '_' ||
            value == '-';
    });
}

} // namespace

std::vector<std::string> SlotNameManifest::parse(
    std::span<const std::byte> slot_zero) {
    if (slot_zero.empty() || slot_zero.size() > k_max_manifest_bytes) {
        return {};
    }
    // The same rule that decides a record is readable decides this one is
    // eligible, so a manifest can never be something the tree calls binary.
    const auto inspected = TextRecord::inspect(slot_zero);
    if (!inspected.recognized) {
        return {};
    }

    std::string text;
    text.reserve(inspected.text_bytes);
    for (std::size_t index = 0U; index < inspected.text_bytes; ++index) {
        text.push_back(std::to_integer<char>(slot_zero[index]));
    }

    std::vector<std::string> names;
    std::size_t start = 0U;
    while (start < text.size()) {
        auto end = text.find("\r\n", start);
        if (end == std::string::npos) {
            end = text.size();
        }
        const std::string_view line{text.data() + start, end - start};
        if (!line.empty()) {
            if (!plausible_name(line)) {
                // One bad line disqualifies the whole record. A partial name
                // list would attribute some slots and silently skip others,
                // which is worse than attributing none.
                return {};
            }
            names.emplace_back(line);
        }
        start = end + 2U;
    }
    return names;
}

std::string SlotNameManifest::extension_of(std::string_view name) {
    const auto dot = name.rfind('.');
    if (dot == std::string_view::npos || dot + 1U >= name.size()) {
        return {};
    }
    std::string extension(name.substr(dot + 1U));
    std::transform(
        extension.begin(), extension.end(), extension.begin(),
        [](unsigned char value) {
            return static_cast<char>(std::tolower(value));
        });
    return extension;
}

} // namespace dmc::rengine::gdspaces
