#include "dmc_rengine/profiles/dmc3/texture_reencode.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>
#include <functional>
#include <map>
#include <sstream>

#include "dmc_rengine/codecs/dds_bcn_encode.hpp"

namespace dmc::rengine::profiles::dmc3 {
namespace {

namespace bcn = codecs::dds_bcn;

constexpr std::size_t k_sector = 0x800U;
constexpr std::size_t k_descriptor = 0x70U;
constexpr std::uint32_t k_max_textures = 4096U;
// Decoding budget per level while re-encoding (16384^2 is the D3D11 limit).
constexpr std::uint64_t k_max_level_pixels = 16384ULL * 16384ULL;

[[nodiscard]] std::uint32_t u32(std::span<const std::byte> s, std::size_t o) noexcept {
    std::uint32_t v = 0U;
    for (std::size_t i = 0U; i < 4U; ++i) v |= std::to_integer<std::uint32_t>(s[o + i]) << (8U * i);
    return v;
}

[[nodiscard]] std::uint64_t u64(std::span<const std::byte> s, std::size_t o) noexcept {
    return static_cast<std::uint64_t>(u32(s, o)) | (static_cast<std::uint64_t>(u32(s, o + 4U)) << 32U);
}

void put_u32(std::vector<std::byte>& b, std::size_t o, std::uint32_t v) noexcept {
    for (std::size_t i = 0U; i < 4U; ++i) b[o + i] = static_cast<std::byte>((v >> (8U * i)) & 0xFFU);
}

[[nodiscard]] std::uint16_t u16(std::span<const std::byte> s, std::size_t o) noexcept {
    return static_cast<std::uint16_t>(std::to_integer<std::uint32_t>(s[o]) |
                                      (std::to_integer<std::uint32_t>(s[o + 1U]) << 8U));
}

[[nodiscard]] std::uint64_t align_up(std::uint64_t v, std::uint64_t a) noexcept {
    return (v + a - 1U) / a * a;
}

// A gfxTexture record at `o` whose DDS (+0x64 bytes at +0x70) parses and ends
// before `end`: the checks dmc3.exe makes when it loads one.
[[nodiscard]] std::optional<bcn::Document> texture_at(std::span<const std::byte> s, std::size_t o,
                                                      std::uint64_t end) noexcept {
    if (o + k_descriptor + bcn::legacy_header_size > s.size()) return std::nullopt;
    if (u64(s, o) != 0U || u64(s, o + 0x20U) != 0x40U || u64(s, o + 0x68U) != 8U) return std::nullopt;
    if (u16(s, o + 0x10U) == 0U || u16(s, o + 0x12U) == 0U) return std::nullopt;
    const std::uint64_t size = u32(s, o + 0x64U);
    if (o + k_descriptor + size > end || end > s.size()) return std::nullopt;
    const auto parsed = bcn::parse(s.subspan(o + k_descriptor, static_cast<std::size_t>(size)));
    if (!parsed.ok() || parsed.document.total_size != size) return std::nullopt;
    return parsed.document;
}

[[nodiscard]] double psnr(const bcn::RgbaImage& a, const bcn::RgbaImage& b) noexcept {
    if (a.rgba8.size() != b.rgba8.size() || a.rgba8.empty()) return 0.0;
    double mse = 0.0;
    for (std::size_t i = 0U; i < a.rgba8.size(); ++i) {
        const double d = static_cast<double>(a.rgba8[i]) - b.rgba8[i];
        mse += d * d;
    }
    mse /= static_cast<double>(a.rgba8.size());
    return mse == 0.0 ? 99.0 : 10.0 * std::log10(255.0 * 255.0 / mse);
}

struct EncodedTexture final {
    std::vector<std::byte> dds;
    bcn::Document document;
    ReencodedTexture report;
};

// Decodes every stored level of `dds` and writes them in the target format.
[[nodiscard]] std::optional<EncodedTexture> reencode_dds_bytes(std::span<const std::byte> dds,
                                                               const TextureReencodeOptions& options,
                                                               std::string* error) {
    const auto parsed = bcn::parse(dds);
    if (!parsed.ok()) {
        *error = "DDS does not parse: " + std::string(bcn::to_string(parsed.status));
        return std::nullopt;
    }
    const auto& d = parsed.document;
    std::vector<bcn::RgbaImage> levels;
    levels.reserve(d.mip_count);
    for (std::uint32_t l = 0U; l < d.mip_count; ++l) {
        auto decoded = bcn::decode_level_rgba8(dds, d, l, k_max_level_pixels);
        if (!decoded.ok) {
            *error = "DDS level " + std::to_string(l) + " does not decode";
            return std::nullopt;
        }
        levels.push_back(std::move(decoded.image));
    }
    auto encoded = bcn::encode_dds(options.format, levels, {.force_dx10 = options.force_dx10});
    if (!encoded.ok) {
        *error = std::string(encoded.detail);
        return std::nullopt;
    }
    const auto check = bcn::parse(std::span<const std::byte>{encoded.bytes.data(), encoded.bytes.size()});
    if (!check.ok()) {
        *error = "re-encoded DDS does not parse";
        return std::nullopt;
    }
    EncodedTexture out;
    const auto round = bcn::decode_level_rgba8(
        std::span<const std::byte>{encoded.bytes.data(), encoded.bytes.size()}, check.document, 0U,
        k_max_level_pixels);
    out.report.from = d.format;
    out.report.to = options.format;
    out.report.from_dx10 = d.dx10_header;
    out.report.to_dx10 = check.document.dx10_header;
    out.report.width = d.width;
    out.report.height = d.height;
    out.report.mips = d.mip_count;
    out.report.old_bytes = d.total_size;
    out.report.new_bytes = encoded.bytes.size();
    out.report.psnr_db = round.ok ? psnr(levels.front(), round.image) : 0.0;
    out.document = check.document;
    out.dds = std::move(encoded.bytes);
    return out;
}

// New gfxTexture header for a re-encoded DDS (see the header comment).
std::vector<std::byte> rewrite_descriptor(std::span<const std::byte> old_desc, const EncodedTexture& t) {
    std::vector<std::byte> d(old_desc.begin(), old_desc.end());
    const auto block = bcn::block_bytes(t.document.format);
    const auto old_word = u32(old_desc, 0x08U);
    const auto low = old_word & 0xFFU;
    const bool canonical = low == 0x86U || low == 0x88U;
    const auto new_low = canonical ? (block == 8U ? 0x86U : 0x88U) : low;
    put_u32(d, 0x08U, (old_word & 0xFFFF0000U) | ((t.document.mip_count & 0xFFU) << 8U) | new_low);
    const auto logical_w = static_cast<std::uint32_t>(u16(old_desc, 0x10U));
    put_u32(d, 0x18U, logical_w * block / 4U);
    put_u32(d, 0x38U, t.document.payload_size);
    const auto old_format = u32(old_desc, 0x60U);
    if (old_format == 0U || old_format == 4U) put_u32(d, 0x60U, block == 8U ? 0U : 4U);
    put_u32(d, 0x64U, t.document.total_size);
    return d;
}

[[nodiscard]] bool reencode_ptx_bytes(std::span<const std::byte> s, const TextureReencodeOptions& options,
                                      int pac_slot, std::vector<std::byte>* out,
                                      std::vector<ReencodedTexture>* reports, std::string* error) {
    const auto count = u32(s, 0U);
    std::vector<std::vector<std::byte>> records;
    std::vector<std::uint32_t> spans;
    std::uint64_t sector = 1U;
    for (std::uint32_t k = 0U; k < count; ++k) {
        const auto span = u32(s, 4U + std::size_t{k} * 4U);
        const auto at = static_cast<std::size_t>(sector * k_sector);
        const auto end = (sector + span) * k_sector;
        if (!texture_at(s, at, end)) {
            *error = "PTX texture " + std::to_string(k) + " fails the load-path checks";
            return false;
        }
        const auto size = u32(s, at + 0x64U);
        auto encoded = reencode_dds_bytes(s.subspan(at + k_descriptor, size), options, error);
        if (!encoded) {
            *error = "PTX texture " + std::to_string(k) + ": " + *error;
            return false;
        }
        encoded->report.pac_slot = pac_slot;
        encoded->report.index = k;
        auto record = rewrite_descriptor(s.subspan(at, k_descriptor), *encoded);
        record.insert(record.end(), encoded->dds.begin(), encoded->dds.end());
        const auto needed = static_cast<std::uint32_t>(align_up(record.size(), k_sector) / k_sector);
        // Keep the original span when the texture still fits (same layout).
        spans.push_back(needed <= span ? span : needed);
        record.resize(static_cast<std::size_t>(spans.back()) * k_sector, std::byte{0});
        records.push_back(std::move(record));
        reports->push_back(encoded->report);
        sector += span;
    }
    out->assign(k_sector, std::byte{0});
    put_u32(*out, 0U, count);
    for (std::uint32_t k = 0U; k < count; ++k) put_u32(*out, 4U + std::size_t{k} * 4U, spans[k]);
    for (const auto& r : records) out->insert(out->end(), r.begin(), r.end());
    return true;
}

[[nodiscard]] bool reencode_wrapped_bytes(std::span<const std::byte> s, const TextureReencodeOptions& options,
                                          int pac_slot, std::vector<std::byte>* out,
                                          std::vector<ReencodedTexture>* reports, std::string* error) {
    const auto size = u32(s, 0x64U);
    auto encoded = reencode_dds_bytes(s.subspan(k_descriptor, size), options, error);
    if (!encoded) return false;
    encoded->report.pac_slot = pac_slot;
    *out = rewrite_descriptor(s.subspan(0U, k_descriptor), *encoded);
    out->insert(out->end(), encoded->dds.begin(), encoded->dds.end());
    reports->push_back(encoded->report);
    return true;
}

[[nodiscard]] bool is_dds(std::span<const std::byte> s) noexcept {
    const auto p = bcn::parse(s);
    return p.ok() && p.document.total_size == s.size();
}

// Re-encodes one non-container payload; false when it holds no texture.
[[nodiscard]] bool reencode_payload_into(std::span<const std::byte> s, const TextureReencodeOptions& options,
                                    int pac_slot, std::vector<std::byte>* out,
                                    std::vector<ReencodedTexture>* reports, std::string* error,
                                    ReencodeContainer* kind) {
    if (is_dds(s)) {
        auto encoded = reencode_dds_bytes(s, options, error);
        if (!encoded) return false;
        encoded->report.pac_slot = pac_slot;
        reports->push_back(encoded->report);
        *out = std::move(encoded->dds);
        *kind = ReencodeContainer::dds;
        return true;
    }
    if (is_wrapped_texture(s)) {
        *kind = ReencodeContainer::wrapped_texture;
        return reencode_wrapped_bytes(s, options, pac_slot, out, reports, error);
    }
    if (is_texture_bundle(s)) {
        *kind = ReencodeContainer::ptx;
        return reencode_ptx_bytes(s, options, pac_slot, out, reports, error);
    }
    *error = "no DDS, gfxTexture or PTX texture";
    return false;
}

[[nodiscard]] bool is_pac(std::span<const std::byte> s) noexcept {
    return s.size() >= 8U && s[0] == std::byte{'P'} && s[1] == std::byte{'A'} && s[2] == std::byte{'C'} &&
        s[3] == std::byte{0};
}

}  // namespace

bool is_wrapped_texture(std::span<const std::byte> s) noexcept {
    if (s.size() < k_descriptor + bcn::legacy_header_size) return false;
    const auto doc = texture_at(s, 0U, s.size());
    return doc.has_value() && k_descriptor + doc->total_size == s.size();
}

bool is_texture_bundle(std::span<const std::byte> s) noexcept {
    if (s.size() < k_sector * 2U || s.size() % k_sector != 0U) return false;
    const auto count = u32(s, 0U);
    if (count == 0U || count > k_max_textures || 4U + std::size_t{count} * 4U > k_sector) return false;
    std::uint64_t sector = 1U;
    for (std::uint32_t k = 0U; k < count; ++k) {
        const auto span = u32(s, 4U + std::size_t{k} * 4U);
        if (span == 0U || (sector + span) * k_sector > s.size()) return false;
        if (!texture_at(s, static_cast<std::size_t>(sector * k_sector), (sector + span) * k_sector)) return false;
        sector += span;
    }
    return sector * k_sector == s.size();
}

namespace {
constexpr int k_max_nesting = 8;

[[nodiscard]] bool holds_textures_at(std::span<const std::byte> s, int depth) noexcept {
    if (!holds_textures(s) && depth < k_max_nesting && s.size() >= 8U && s[0] == std::byte{'P'} &&
        s[1] == std::byte{'A'} && s[2] == std::byte{'C'} && s[3] == std::byte{0}) {
        const auto slots = read_pac_slots(s);
        if (!slots) return false;
        for (const auto& e : *slots) {
            if (e.offset == 0U) continue;
            const auto payload = s.subspan(static_cast<std::size_t>(e.offset), static_cast<std::size_t>(e.size));
            if (holds_textures_at(payload, depth + 1)) return true;
        }
        return false;
    }
    return holds_textures(s);
}
}  // namespace

bool holds_textures(std::span<const std::byte> s, bool nested) noexcept {
    return nested ? holds_textures_at(s, 0) : holds_textures(s);
}

bool holds_textures(std::span<const std::byte> s) noexcept {
    try {
        if (!is_pac(s)) return is_dds(s) || is_wrapped_texture(s) || is_texture_bundle(s);
        const auto slots = read_pac_slots(s);
        if (!slots) return false;
        for (const auto& e : *slots) {
            if (e.offset == 0U) continue;
            auto payload = s.subspan(static_cast<std::size_t>(e.offset), static_cast<std::size_t>(e.size));
            for (std::size_t trim = 0U; trim < 16U && trim < payload.size(); ++trim) {
                const auto p = payload.first(payload.size() - trim);
                if (is_texture_bundle(p) || is_wrapped_texture(p) || is_dds(p)) return true;
                if (p.back() != std::byte{0}) break;
            }
        }
    } catch (...) {
    }
    return false;
}

std::optional<std::vector<PacSlotExtent>> read_pac_slots(std::span<const std::byte> s) {
    if (!is_pac(s)) return std::nullopt;
    const auto count = u32(s, 4U);
    if (count == 0U || count > (1U << 20U) || 8U + std::uint64_t{count} * 4U > s.size()) return std::nullopt;
    std::vector<std::uint64_t> offsets(count);
    for (std::uint32_t i = 0U; i < count; ++i) offsets[i] = u32(s, 8U + std::size_t{i} * 4U);
    std::vector<std::uint64_t> sorted;
    for (const auto o : offsets) {
        if (o == 0U) continue;
        if (o < 8U + std::uint64_t{count} * 4U || o > s.size()) return std::nullopt;
        sorted.push_back(o);
    }
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
    std::vector<PacSlotExtent> out(count);
    for (std::uint32_t i = 0U; i < count; ++i) {
        if (offsets[i] == 0U) continue;
        const auto next = std::upper_bound(sorted.begin(), sorted.end(), offsets[i]);
        const auto end = next == sorted.end() ? s.size() : *next;
        out[i] = {offsets[i], end - offsets[i]};
    }
    return out;
}

std::optional<std::vector<std::byte>> replace_pac_slots(std::span<const std::byte> pac,
                                                        std::span<const PacSlotReplacement> replacements) {
    const auto slots = read_pac_slots(pac);
    if (!slots) return std::nullopt;
    std::map<std::uint64_t, std::span<const std::byte>> by_offset;  // distinct extents in file order
    std::uint64_t first = pac.size();
    for (const auto& e : *slots) {
        if (e.offset == 0U) continue;
        by_offset.emplace(e.offset, pac.subspan(static_cast<std::size_t>(e.offset), static_cast<std::size_t>(e.size)));
        first = std::min(first, e.offset);
    }
    for (const auto& r : replacements) {
        if (r.slot >= slots->size() || (*slots)[r.slot].offset == 0U) return std::nullopt;
        by_offset[(*slots)[r.slot].offset] = std::span<const std::byte>{r.bytes.data(), r.bytes.size()};
    }
    std::vector<std::byte> out(pac.begin(), pac.begin() + static_cast<std::ptrdiff_t>(first));
    std::map<std::uint64_t, std::uint64_t> moved;
    for (const auto& [offset, bytes] : by_offset) {
        out.resize(static_cast<std::size_t>(align_up(out.size(), 16U)), std::byte{0});
        if (out.size() > std::numeric_limits<std::uint32_t>::max()) return std::nullopt;
        moved[offset] = out.size();
        out.insert(out.end(), bytes.begin(), bytes.end());
    }
    for (std::uint32_t i = 0U; i < slots->size(); ++i) {
        const auto o = (*slots)[i].offset;
        put_u32(out, 8U + std::size_t{i} * 4U, o == 0U ? 0U : static_cast<std::uint32_t>(moved[o]));
    }
    return out;
}

std::vector<bcn::Document> list_textures(std::span<const std::byte> s, bool nested) {
    if (!nested) return list_textures(s);
    std::vector<bcn::Document> out = list_textures(s);
    const std::function<void(std::span<const std::byte>, int)> descend = [&](std::span<const std::byte> p, int depth) {
        if (depth >= k_max_nesting) return;
        const auto slots = read_pac_slots(p);
        if (!slots) return;
        for (const auto& e : *slots) {
            if (e.offset == 0U) continue;
            const auto payload = p.subspan(static_cast<std::size_t>(e.offset), static_cast<std::size_t>(e.size));
            if (!is_pac(payload)) continue;
            const auto inner = list_textures(payload);
            out.insert(out.end(), inner.begin(), inner.end());
            descend(payload, depth + 1);
        }
    };
    descend(s, 0);
    return out;
}

std::vector<bcn::Document> list_textures(std::span<const std::byte> s) {
    std::vector<bcn::Document> out;
    const auto payload_textures = [&](std::span<const std::byte> p) {
        if (is_dds(p)) {
            out.push_back(bcn::parse(p).document);
            return true;
        }
        if (is_wrapped_texture(p)) {
            out.push_back(*texture_at(p, 0U, p.size()));
            return true;
        }
        if (is_texture_bundle(p)) {
            std::uint64_t sector = 1U;
            for (std::uint32_t k = 0U; k < u32(p, 0U); ++k) {
                const auto span = u32(p, 4U + std::size_t{k} * 4U);
                out.push_back(*texture_at(p, static_cast<std::size_t>(sector * k_sector), (sector + span) * k_sector));
                sector += span;
            }
            return true;
        }
        return false;
    };
    if (!is_pac(s)) {
        (void)payload_textures(s);
        return out;
    }
    const auto slots = read_pac_slots(s);
    if (!slots) return out;
    for (const auto& e : *slots) {
        if (e.offset == 0U) continue;
        const auto payload = s.subspan(static_cast<std::size_t>(e.offset), static_cast<std::size_t>(e.size));
        for (std::size_t trim = 0U; trim < 16U && trim < payload.size(); ++trim) {
            if (payload_textures(payload.first(payload.size() - trim))) break;
            if (payload[payload.size() - trim - 1U] != std::byte{0}) break;
        }
    }
    return out;
}

std::span<const std::byte> texture_payload(std::span<const std::byte> extent) noexcept {
    // A slot's extent may end in zero alignment after its data (slots start
    // 16-byte aligned): drop up to 15 trailing zeros for the detection.
    for (std::size_t trim = 0U; trim < 16U && trim < extent.size(); ++trim) {
        const auto p = extent.first(extent.size() - trim);
        if (is_texture_bundle(p) || is_wrapped_texture(p) || is_dds(p)) return p;
        if (p.back() != std::byte{0}) break;
    }
    return {};
}

PayloadReencodeResult reencode_payload(std::span<const std::byte> payload, const TextureReencodeOptions& options,
                                       int pac_slot) {
    PayloadReencodeResult out;
    try {
        out.ok = reencode_payload_into(payload, options, pac_slot, &out.bytes, &out.textures, &out.detail,
                                       &out.container);
    } catch (...) {
        out = {};
        out.detail = "texture re-encode allocation failed";
    }
    return out;
}

namespace {
[[nodiscard]] TextureReencodeResult reencode_textures_at(std::span<const std::byte> source,
                                                         const TextureReencodeOptions& options, int depth);
}  // namespace

TextureReencodeResult reencode_textures(std::span<const std::byte> source, const TextureReencodeOptions& options) {
    return reencode_textures_at(source, options, 0);
}

namespace {
TextureReencodeResult reencode_textures_at(std::span<const std::byte> source, const TextureReencodeOptions& options,
                                           int depth) {
    TextureReencodeResult result;
    std::string error;
    try {
        if (!is_pac(source)) {
            if (!reencode_payload_into(source, options, -1, &result.bytes, &result.textures, &error, &result.container)) {
                result.detail = error;
                return result;
            }
        } else {
            result.container = ReencodeContainer::pac;
            const auto slots = read_pac_slots(source);
            if (!slots) {
                result.detail = "PAC slot table does not parse";
                return result;
            }
            std::vector<PacSlotReplacement> replacements;
            for (std::uint32_t i = 0U; i < slots->size(); ++i) {
                const auto& e = (*slots)[i];
                if (e.offset == 0U || (options.pac_slot >= 0 && static_cast<std::uint32_t>(options.pac_slot) != i)) {
                    continue;
                }
                auto payload = texture_payload(
                    source.subspan(static_cast<std::size_t>(e.offset), static_cast<std::size_t>(e.size)));
                std::vector<std::byte> encoded;
                ReencodeContainer kind{};
                std::string slot_error;
                const auto extent =
                    source.subspan(static_cast<std::size_t>(e.offset), static_cast<std::size_t>(e.size));
                if (payload.empty() && options.nested && depth < k_max_nesting && is_pac(extent)) {
                    // A PAC inside the PAC: rebuild it with its own textures re-encoded.
                    auto inner_options = options;
                    inner_options.pac_slot = -1;
                    auto inner = reencode_textures_at(extent, inner_options, depth + 1);
                    if (inner.ok) {
                        for (auto& t : inner.textures) t.pac_path.insert(t.pac_path.begin(), i);
                        result.textures.insert(result.textures.end(), inner.textures.begin(), inner.textures.end());
                        replacements.push_back({i, std::move(inner.bytes)});
                    }
                    continue;
                }
                if (payload.empty() ||
                    !reencode_payload_into(payload, options, static_cast<int>(i), &encoded, &result.textures,
                                           &slot_error, &kind)) {
                    if (payload.empty()) slot_error = "no DDS, gfxTexture or PTX texture";
                    if (options.pac_slot >= 0) {
                        result.detail = "PAC slot " + std::to_string(i) + ": " + slot_error;
                        return result;
                    }
                    continue;  // not a texture slot
                }
                replacements.push_back({i, std::move(encoded)});
            }
            if (replacements.empty()) {
                result.detail = "PAC holds no PTX / DDS texture slot";
                return result;
            }
            auto rebuilt = replace_pac_slots(source, replacements);
            if (!rebuilt) {
                result.detail = "PAC rebuild failed";
                return result;
            }
            result.bytes = std::move(*rebuilt);
        }
    } catch (...) {
        result.bytes.clear();
        result.textures.clear();
        result.detail = "texture re-encode allocation failed";
        return result;
    }
    std::ostringstream detail;
    detail << result.textures.size() << " texture(s) -> " << bcn::format_name(options.format)
           << (options.force_dx10 || !bcn::legacy_fourcc(options.format) ? " DX10" : "") << ", "
           << source.size() << " -> " << result.bytes.size() << " bytes";
    result.detail = detail.str();
    result.ok = !result.textures.empty();
    return result;
}
}  // namespace

}  // namespace dmc::rengine::profiles::dmc3
