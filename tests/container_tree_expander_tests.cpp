#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/gdspaces/container_tree_expander.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

void put_ascii(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::string_view value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(value[index]);
    }
}

[[nodiscard]] bool starts_with(
    std::span<const std::byte> bytes,
    std::string_view magic) noexcept {
    if (bytes.size() < magic.size()) {
        return false;
    }
    for (std::size_t index = 0; index < magic.size(); ++index) {
        if (std::to_integer<unsigned char>(bytes[index]) !=
            static_cast<unsigned char>(magic[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] dmc::rengine::formats::ContainerParseResult adapt(
    dmc::rengine::formats::RelativeSlotParseResult parsed) {
    dmc::rengine::formats::ContainerParseResult result;
    result.recognized = true;
    if (parsed.document.has_value()) {
        result.document = std::move(*parsed.document);
    }
    if (!parsed.ok()) {
        result.diagnostics.push_back(dmc::rengine::formats::ParseDiagnostic{
            .severity = dmc::rengine::formats::ParseSeverity::error,
            .code = "test.structural_error",
            .message = parsed.message,
            .offset = 0U,
        });
    }
    return result;
}

class BytePurePacTestParser final :
    public dmc::rengine::formats::IContainerParser {
public:
    [[nodiscard]] std::string_view id() const noexcept override {
        return "test-byte-pure-pac";
    }

    [[nodiscard]] std::string_view format() const noexcept override {
        return "pac";
    }

    [[nodiscard]] bool supports_byte_identity_reuse() const noexcept override {
        return true;
    }

    [[nodiscard]] int probe(
        std::span<const std::byte> bytes,
        std::string_view) const noexcept override {
        return starts_with(bytes, std::string_view{"PAC\0", 4U}) ? 100 : 0;
    }

    [[nodiscard]] dmc::rengine::formats::ContainerParseResult parse(
        std::span<const std::byte> bytes,
        std::string_view) const override {
        return adapt(dmc::rengine::formats::PacParser::parse(bytes));
    }
};

class ContextSensitivePnstTestParser final :
    public dmc::rengine::formats::IContainerParser {
public:
    explicit ContextSensitivePnstTestParser(std::size_t& invocation_count)
        : invocation_count_(invocation_count) {}

    [[nodiscard]] std::string_view id() const noexcept override {
        return "test-context-sensitive-pnst";
    }

    [[nodiscard]] std::string_view format() const noexcept override {
        return "pnst";
    }

    // Intentionally does not opt into supports_byte_identity_reuse(). This
    // models a future parser whose result may depend on logical path/context.

    [[nodiscard]] int probe(
        std::span<const std::byte> bytes,
        std::string_view) const noexcept override {
        return starts_with(bytes, "PNST") ? 100 : 0;
    }

    [[nodiscard]] dmc::rengine::formats::ContainerParseResult parse(
        std::span<const std::byte> bytes,
        std::string_view) const override {
        ++invocation_count_;
        return adapt(dmc::rengine::formats::PnstParser::parse(bytes));
    }

private:
    std::size_t& invocation_count_;
};

[[nodiscard]] std::vector<std::byte> make_alias_nested_fixture() {
    std::vector<std::byte> bytes(0x80U, std::byte{0});

    // Outer PAC: slots 0 and 1 are separate slot identities sharing one
    // nested-PNST byte span. Slot 2 is an SCM leaf.
    put_ascii(bytes, 0U, std::string_view{"PAC\0", 4U});
    put_u32(bytes, 4U, 3U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x20U);
    put_u32(bytes, 16U, 0x60U);

    put_ascii(bytes, 0x20U, "PNST");
    put_u32(bytes, 0x24U, 1U);
    put_u32(bytes, 0x28U, 0x20U);
    put_ascii(bytes, 0x40U, "DDS ");

    put_ascii(bytes, 0x60U, std::string_view{"SCM\0", 4U});
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_root(
    std::vector<std::byte> bytes) {
    using namespace dmc::rengine::gdspaces;

    const auto size = static_cast<std::uint64_t>(bytes.size());
    return ResourcePayload{
        .resource = ResourceRef{
            .id = ResourceId{
                .source_id = "recursive-test",
                .logical_path = "DMC3/root.pac",
                .container_chain = {},
                .offset = 1000U,
                .size = size,
            },
            .display_name = "root.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = ByteProvenance{
            .kind = ByteOriginKind::direct_source_span,
            .authority_id = "root-file",
            .offset = 1000U,
            .stored_size = size,
            .materialized_size = size,
            .transform = ByteTransform::none,
            .crc32 = std::nullopt,
        },
    };
}

} // namespace

int main() {
    using dmc::rengine::formats::ContainerParserRegistry;
    using dmc::rengine::gdspaces::ContainerTreeExpander;
    using dmc::rengine::gdspaces::ContainerTreeExpansionLimits;
    using dmc::rengine::gdspaces::DiagnosticSeverity;
    using dmc::rengine::gdspaces::ResourceRelation;
    using dmc::rengine::profiles::dmc3::make_container_parser_registry;

    auto registry = make_container_parser_registry();
    assert(registry.size() == 2U);

    const auto root = make_root(make_alias_nested_fixture());
    const auto tree = ContainerTreeExpander::expand(root, registry);
    assert(tree.complete());
    assert(tree.fully_expanded);

    // Resource identity and parse execution are deliberately different axes.
    // Root PAC + two PNST slot aliases are three expanded identities, while
    // the identical PNST bytes need only one byte-pure parser execution.
    assert(tree.expanded_container_count == 3U);
    assert(tree.parser_execution_count == 2U);
    assert(tree.reused_parse_count == 1U);
    assert(tree.parsed_container_bytes == 0x80U + 0x40U);
    assert(tree.expansions.size() == 3U);

    assert(tree.graph.resource_count() == 6U);
    assert(tree.graph.edge_count() == 5U);

    const auto root_children = tree.graph.related(
        root.resource.id, ResourceRelation::contains);
    assert(root_children.size() == 3U);

    const auto* alias0 = root_children[0];
    const auto* alias1 = root_children[1];
    const auto* scm = root_children[2];
    assert(alias0 != nullptr && alias1 != nullptr && scm != nullptr);
    assert(alias0->format == "pnst");
    assert(alias1->format == "pnst");
    assert(alias0->container && alias1->container);

    // Equal byte range does not collapse declared slot/resource identity.
    assert(alias0->id.offset == 1032U);
    assert(alias1->id.offset == 1032U);
    assert(alias0->id.size == 0x40U);
    assert(alias1->id.size == 0x40U);
    assert(alias0->id.canonical() != alias1->id.canonical());
    assert(alias0->id.container_chain == "PAC[0]");
    assert(alias1->id.container_chain == "PAC[1]");

    const auto alias0_children = tree.graph.related(
        alias0->id, ResourceRelation::contains);
    const auto alias1_children = tree.graph.related(
        alias1->id, ResourceRelation::contains);
    assert(alias0_children.size() == 1U);
    assert(alias1_children.size() == 1U);
    assert(alias0_children[0]->format == "dds");
    assert(alias1_children[0]->format == "dds");
    assert(alias0_children[0]->id.offset == 1064U);
    assert(alias1_children[0]->id.offset == 1064U);
    assert(alias0_children[0]->id.canonical() !=
        alias1_children[0]->id.canonical());
    assert(alias0_children[0]->id.container_chain == "PAC[0]/PNST[0]");
    assert(alias1_children[0]->id.container_chain == "PAC[1]/PNST[0]");

    assert(scm->format == "scm");
    assert(!scm->container);
    assert(scm->id.offset == 1096U);

    // A parser that does not explicitly opt into byte-identity reuse must be
    // invoked once per resource identity even when valid provenance proves
    // that two aliases point at identical bytes.
    std::size_t context_sensitive_invocations = 0U;
    ContainerParserRegistry conservative_registry;
    assert(conservative_registry.register_parser(
        std::make_unique<BytePurePacTestParser>()));
    assert(conservative_registry.register_parser(
        std::make_unique<ContextSensitivePnstTestParser>(
            context_sensitive_invocations)));

    const auto conservative_tree = ContainerTreeExpander::expand(
        root, conservative_registry);
    assert(conservative_tree.complete());
    assert(conservative_tree.expanded_container_count == 3U);
    assert(conservative_tree.parser_execution_count == 3U);
    assert(conservative_tree.reused_parse_count == 0U);
    assert(conservative_tree.parsed_container_bytes == 0x100U);
    assert(context_sensitive_invocations == 2U);

    // max_nested_depth=0 expands only the root but preserves nested identities
    // and records deterministic truncation diagnostics.
    const auto shallow = ContainerTreeExpander::expand(
        root,
        registry,
        ContainerTreeExpansionLimits{
            .max_nested_depth = 0U,
            .max_expanded_containers = 16U,
            .max_graph_nodes = 64U,
            .max_parsed_container_bytes = 4096U,
        });
    assert(!shallow.complete());
    assert(!shallow.fully_expanded);
    assert(shallow.expanded_container_count == 1U);
    assert(shallow.parser_execution_count == 1U);
    assert(shallow.reused_parse_count == 0U);
    assert(shallow.graph.resource_count() == 4U);

    std::size_t depth_warnings = 0U;
    for (const auto& diagnostic : shallow.diagnostics) {
        if (diagnostic.severity == DiagnosticSeverity::warning &&
            diagnostic.code == "gdspaces.container-tree.depth-budget") {
            ++depth_warnings;
        }
    }
    assert(depth_warnings == 2U);

    // Parser byte budget is charged to actual parser executions. Root 0x80
    // fits, but adding the first nested 0x40 parse would exceed 150 bytes.
    const auto byte_limited = ContainerTreeExpander::expand(
        root,
        registry,
        ContainerTreeExpansionLimits{
            .max_nested_depth = 4U,
            .max_expanded_containers = 16U,
            .max_graph_nodes = 64U,
            .max_parsed_container_bytes = 150U,
        });
    assert(!byte_limited.complete());
    assert(byte_limited.expanded_container_count == 1U);
    assert(byte_limited.parser_execution_count == 1U);
    assert(byte_limited.parsed_container_bytes == 0x80U);
    assert(byte_limited.graph.resource_count() == 4U);

    bool found_byte_error = false;
    for (const auto& diagnostic : byte_limited.diagnostics) {
        if (diagnostic.severity == DiagnosticSeverity::error &&
            diagnostic.code == "gdspaces.container-tree.byte-budget") {
            found_byte_error = true;
        }
    }
    assert(found_byte_error);

    return 0;
}
