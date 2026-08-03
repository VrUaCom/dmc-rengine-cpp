#pragma once

#include "dmc_rengine/gdspaces/stage_bundle.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::integration {

enum class IntegrationMaturity {
    recognized,
    structural,
    semantic,
    editable,
    exportable,
};

[[nodiscard]] constexpr std::string_view to_string(
    IntegrationMaturity maturity) noexcept {
    switch (maturity) {
    case IntegrationMaturity::recognized: return "recognized";
    case IntegrationMaturity::structural: return "structural";
    case IntegrationMaturity::semantic: return "semantic";
    case IntegrationMaturity::editable: return "editable";
    case IntegrationMaturity::exportable: return "exportable";
    }
    return "recognized";
}

enum class ResourceWritePolicy {
    read_only,
    working_copy_only,
    guarded_export,
};

[[nodiscard]] constexpr std::string_view to_string(
    ResourceWritePolicy policy) noexcept {
    switch (policy) {
    case ResourceWritePolicy::read_only: return "read-only";
    case ResourceWritePolicy::working_copy_only: return "working-copy-only";
    case ResourceWritePolicy::guarded_export: return "guarded-export";
    }
    return "read-only";
}

struct FormatIntegrationDescriptor final {
    std::string format;
    std::string parser_id;
    IntegrationMaturity maturity{IntegrationMaturity::recognized};
    ResourceWritePolicy write_policy{ResourceWritePolicy::read_only};
    bool binary_adapter{false};
    std::optional<gdspaces::StageResourceCategory> stage_category;
    std::vector<std::string> evidence_claim_ids;
    std::vector<std::string> limitations;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool allows_working_copy() const noexcept;
    [[nodiscard]] bool allows_guarded_export() const noexcept;
};

class FormatIntegrationRegistry final {
public:
    FormatIntegrationRegistry();

    [[nodiscard]] const FormatIntegrationDescriptor* find(
        std::string_view format) const noexcept;
    [[nodiscard]] const std::vector<FormatIntegrationDescriptor>&
        formats() const noexcept;
    [[nodiscard]] std::vector<const FormatIntegrationDescriptor*> by_maturity(
        IntegrationMaturity maturity) const;

private:
    std::vector<FormatIntegrationDescriptor> formats_;
};

} // namespace dmc::rengine::integration
