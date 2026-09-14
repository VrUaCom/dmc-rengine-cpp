#include "dmc_rengine/exe/executable_report.hpp"

#include "json_writer.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <map>
#include <sstream>
#include <string_view>
#include <vector>

namespace dmc::rengine::exe {
namespace {

using detail::hex_u64;
using detail::JsonWriter;


void write_sections(JsonWriter& writer, const PeImage& image) {
    writer.key("sections");
    writer.begin_array();
    for (const auto& section : image.sections) {
        writer.begin_object();
        writer.member("name", section.name);
        writer.hex_member("virtual_address", section.virtual_address);
        writer.member("virtual_size", static_cast<std::uint64_t>(section.virtual_size));
        writer.hex_member("raw_offset", section.raw_offset);
        writer.member("raw_size", static_cast<std::uint64_t>(section.raw_size));
        writer.hex_member("characteristics", section.characteristics);
        writer.member("executable", (section.characteristics & 0x20000000U) != 0U);
        writer.member("readable", (section.characteristics & 0x40000000U) != 0U);
        writer.member("writable", (section.characteristics & 0x80000000U) != 0U);
        writer.end_object();
    }
    writer.end_array();
}

void write_directories(JsonWriter& writer, const PeImage& image) {
    writer.key("data_directories");
    writer.begin_array();
    for (std::size_t index = 0; index < image.data_directories.size(); ++index) {
        const auto& entry = image.data_directories[index];
        if (!entry.present()) {
            continue;
        }
        writer.begin_object();
        writer.member("index", static_cast<std::uint64_t>(index));
        writer.member("name", to_string(static_cast<PeDirectory>(index)));
        writer.hex_member("rva", entry.rva);
        writer.member("size", static_cast<std::uint64_t>(entry.size));
        writer.end_object();
    }
    writer.end_array();
}

void write_imports(JsonWriter& writer, const PeDirectories& directories,
                   const ExecutableReportOptions& options) {
    writer.key("imports");
    writer.begin_object();
    writer.member("module_count", static_cast<std::uint64_t>(directories.imports.size()));
    writer.member("function_count",
                  static_cast<std::uint64_t>(directories.imported_function_count()));
    writer.key("modules");
    writer.begin_array();
    for (const auto& module : directories.imports) {
        writer.begin_object();
        writer.member("name", module.name);
        writer.member("delay_loaded", module.delay_loaded);
        writer.member("function_count", static_cast<std::uint64_t>(module.functions.size()));
        if (options.include_import_functions) {
            writer.key("functions");
            writer.begin_array();
            for (const auto& function : module.functions) {
                writer.begin_object();
                if (function.by_ordinal) {
                    writer.member("ordinal", static_cast<std::uint64_t>(function.ordinal));
                } else {
                    writer.member("name", function.name);
                    writer.member("hint", static_cast<std::uint64_t>(function.hint));
                }
                writer.hex_member("iat_rva", function.iat_rva);
                writer.end_object();
            }
            writer.end_array();
        }
        writer.end_object();
    }
    writer.end_array();
    writer.end_object();
}

void write_exports(JsonWriter& writer, const PeDirectories& directories) {
    if (!directories.exports.has_value()) {
        return;
    }

    const auto& table = *directories.exports;
    writer.key("exports");
    writer.begin_object();
    writer.member("module_name", table.module_name);
    writer.member("ordinal_base", static_cast<std::uint64_t>(table.ordinal_base));
    writer.member("address_count", static_cast<std::uint64_t>(table.address_count));
    writer.key("symbols");
    writer.begin_array();
    for (const auto& symbol : table.symbols) {
        writer.begin_object();
        writer.member("name", symbol.name);
        writer.member("ordinal", static_cast<std::uint64_t>(symbol.ordinal));
        writer.hex_member("rva", symbol.rva);
        if (symbol.forwarded()) {
            writer.member("forwarder", symbol.forwarder);
        }
        writer.end_object();
    }
    writer.end_array();
    writer.end_object();
}

void write_functions(JsonWriter& writer, const PeDirectories& directories,
                     const ExecutableReportOptions& options) {
    if (!directories.functions.has_value()) {
        return;
    }

    const auto& table = *directories.functions;
    writer.key("functions");
    writer.begin_object();
    writer.member("count", static_cast<std::uint64_t>(table.functions.size()));
    writer.member("covered_bytes", table.covered_bytes);
    writer.member("smallest_size", static_cast<std::uint64_t>(table.smallest_size));
    writer.member("largest_size", static_cast<std::uint64_t>(table.largest_size));
    writer.member("sorted_by_address", table.sorted_by_address);

    // Power-of-two buckets keep the shape of the code section visible without
    // committing the whole table.
    std::map<std::uint32_t, std::uint64_t> histogram;
    for (const auto& function : table.functions) {
        std::uint32_t bucket = 16U;
        while (bucket < function.size() && bucket < (1U << 20U)) {
            bucket <<= 1U;
        }
        ++histogram[bucket];
    }

    writer.key("size_histogram");
    writer.begin_array();
    for (const auto& [bucket, count] : histogram) {
        writer.begin_object();
        writer.member("up_to_bytes", static_cast<std::uint64_t>(bucket));
        writer.member("functions", count);
        writer.end_object();
    }
    writer.end_array();

    if (options.include_function_ranges) {
        writer.key("ranges");
        writer.begin_array();
        for (const auto& function : table.functions) {
            writer.begin_object();
            writer.hex_member("begin_rva", function.begin_rva);
            writer.hex_member("end_rva", function.end_rva);
            writer.hex_member("unwind_rva", function.unwind_rva);
            writer.end_object();
        }
        writer.end_array();
    }
    writer.end_object();
}

void write_debug(JsonWriter& writer, const PeDirectories& directories) {
    if (directories.debug_entries.empty()) {
        return;
    }

    writer.key("debug");
    writer.begin_object();
    writer.key("entries");
    writer.begin_array();
    for (const auto& entry : directories.debug_entries) {
        writer.begin_object();
        writer.member("type", static_cast<std::uint64_t>(entry.type));
        writer.member("type_name", debug_type_name(entry.type));
        writer.member("size_of_data", static_cast<std::uint64_t>(entry.size_of_data));
        writer.hex_member("address_of_raw_data", entry.address_of_raw_data);
        writer.end_object();
    }
    writer.end_array();

    if (directories.codeview.has_value()) {
        writer.key("codeview");
        writer.begin_object();
        writer.member("guid", directories.codeview->guid_string());
        writer.member("age", static_cast<std::uint64_t>(directories.codeview->age));
        writer.member("pdb_path", directories.codeview->pdb_path);
        writer.end_object();
    }
    writer.end_object();
}

void write_relocations(JsonWriter& writer, const PeDirectories& directories) {
    if (!directories.relocations.has_value()) {
        return;
    }

    const auto& summary = *directories.relocations;
    writer.key("relocations");
    writer.begin_object();
    writer.member("block_count", static_cast<std::uint64_t>(summary.block_count));
    writer.member("entry_count", summary.entry_count);
    writer.member("absolute_entries", summary.absolute_entries);
    writer.member("dir64_entries", summary.dir64_entries);
    writer.member("other_entries", summary.other_entries);
    writer.hex_member("lowest_rva", summary.lowest_rva);
    writer.hex_member("highest_rva", summary.highest_rva);
    writer.end_object();
}

void write_tls(JsonWriter& writer, const PeDirectories& directories) {
    if (!directories.tls.has_value()) {
        return;
    }

    const auto& tls = *directories.tls;
    writer.key("tls");
    writer.begin_object();
    writer.hex_member("raw_data_start_va", tls.raw_data_start_va);
    writer.hex_member("raw_data_end_va", tls.raw_data_end_va);
    writer.hex_member("index_va", tls.index_va);
    writer.hex_member("callbacks_va", tls.callbacks_va);
    writer.member("zero_fill_size", static_cast<std::uint64_t>(tls.zero_fill_size));
    writer.member("callback_count", static_cast<std::uint64_t>(tls.callbacks.size()));
    writer.end_object();
}

void write_resources(JsonWriter& writer, const PeDirectories& directories) {
    if (directories.resource_types.empty()) {
        return;
    }

    writer.key("resources");
    writer.begin_array();
    for (const auto& entry : directories.resource_types) {
        writer.begin_object();
        writer.member("type_id", static_cast<std::uint64_t>(entry.type_id));
        if (!entry.type_name.empty()) {
            writer.member("type_name", entry.type_name);
        }
        writer.member("entry_count", static_cast<std::uint64_t>(entry.entry_count));
        writer.end_object();
    }
    writer.end_array();
}

void write_name_tables(JsonWriter& writer, const StringTableScanResult& tables) {
    writer.key("name_tables");
    writer.begin_object();
    writer.member("candidate_names", static_cast<std::uint64_t>(tables.candidate_names));
    writer.member("runs", static_cast<std::uint64_t>(tables.runs.size()));
    writer.member("entries_in_runs", static_cast<std::uint64_t>(tables.entries_in_runs));

    writer.key("largest");
    writer.begin_array();
    // Layout only, plus one representative name per run. The tables hold game
    // data and are deliberately not extracted here.
    constexpr std::size_t kReported = 40U;
    for (std::size_t index = 0; index < tables.runs.size() && index < kReported; ++index) {
        const auto& run = tables.runs[index];
        writer.begin_object();
        writer.hex_member("base_rva", run.base_rva);
        writer.member("stride", static_cast<std::uint64_t>(run.stride));
        writer.member("entries", static_cast<std::uint64_t>(run.entries));
        writer.member("longest_name", static_cast<std::uint64_t>(run.longest_name));
        writer.member("span_bytes", run.span_bytes());
        writer.member("pure_name_array", run.pure_name_array());
        if (!run.pure_name_array()) {
            writer.member("records_with_payload",
                          static_cast<std::uint64_t>(run.records_with_payload));
        }
        writer.member("sample_name", run.first_name);
        writer.end_object();
    }
    writer.end_array();
    writer.end_object();
}

void write_rtti(JsonWriter& writer, const RttiScanResult& rtti,
                const ExecutableReportOptions& options) {
    writer.key("rtti");
    writer.begin_object();
    writer.member("type_descriptors", static_cast<std::uint64_t>(rtti.type_descriptors));
    writer.member("complete_object_locators", static_cast<std::uint64_t>(rtti.locators));
    writer.member("distinct_types", static_cast<std::uint64_t>(rtti.classes.size()));
    writer.member("vtables_located", static_cast<std::uint64_t>(rtti.vtables_located));

    if (options.include_rtti_classes) {
        writer.key("class_graph");
        writer.begin_array();
        for (const auto& entry : rtti.classes) {
            writer.begin_object();
            writer.member("display_name", entry.display_name);
            writer.member("decorated_name", entry.decorated_name);
            if (!entry.display_name_complete) {
                writer.member("display_name_complete", false);
            }
            writer.hex_member("type_descriptor_rva", entry.type_descriptor_rva);
            writer.hex_member("class_hierarchy_rva", entry.class_hierarchy_rva);
            writer.member("base_count", static_cast<std::uint64_t>(entry.base_count()));

            writer.key("vtables");
            writer.begin_array();
            for (const auto& vtable : entry.vtables) {
                writer.begin_object();
                writer.hex_member("complete_object_locator_rva",
                                  vtable.complete_object_locator_rva);
                writer.hex_member("vtable_rva", vtable.vtable_rva);
                writer.member("slots", static_cast<std::uint64_t>(vtable.slot_count));
                writer.member("subobject_offset",
                              static_cast<std::uint64_t>(vtable.subobject_offset));
                writer.end_object();
            }
            writer.end_array();
            if (entry.multiple_inheritance) {
                writer.member("multiple_inheritance", true);
            }
            if (entry.virtual_inheritance) {
                writer.member("virtual_inheritance", true);
            }
            if (entry.ambiguous) {
                writer.member("ambiguous", true);
            }

            if (options.include_rtti_hierarchy && entry.hierarchy.size() > 1U) {
                writer.key("bases");
                writer.begin_array();
                // Entry zero is the class itself.
                for (std::size_t index = 1U; index < entry.hierarchy.size(); ++index) {
                    const auto& base = entry.hierarchy[index];
                    writer.begin_object();
                    writer.member("display_name", base.display_name);
                    writer.key("member_displacement");
                    writer.number(static_cast<std::int64_t>(base.member_displacement));
                    writer.end_object();
                }
                writer.end_array();
            }
            writer.end_object();
        }
        writer.end_array();
    }
    writer.end_object();
}

} // namespace

std::string to_json(const ExecutableArtifactIdentity& artifact, const PeImage& image,
                    const PeDirectories& directories, const RttiScanResult& rtti,
                    const StringTableScanResult& name_tables,
                    const ExecutableReportOptions& options) {
    std::ostringstream output;
    JsonWriter writer{output};

    writer.begin_object();
    writer.member("schema", "dmc-rengine.executable-report.v1");

    writer.key("artifact");
    writer.begin_object();
    writer.member("sha256", artifact.sha256);
    writer.member("size", artifact.size);
    writer.end_object();

    writer.key("image");
    writer.begin_object();
    writer.member("kind", to_string(image.kind));
    writer.member("machine", to_string(image.machine));
    writer.hex_member("image_base", image.image_base);
    writer.hex_member("entry_point_rva", image.entry_point_rva);
    writer.hex_member("size_of_image", image.size_of_image);
    writer.member("size_of_headers", static_cast<std::uint64_t>(image.size_of_headers));
    writer.member("section_count", static_cast<std::uint64_t>(image.section_count));
    writer.member("subsystem", static_cast<std::uint64_t>(image.subsystem));
    writer.hex_member("characteristics", image.characteristics);
    writer.hex_member("dll_characteristics", image.dll_characteristics);
    writer.member("timestamp", static_cast<std::uint64_t>(image.timestamp));
    writer.hex_member("section_alignment", image.section_alignment);
    writer.hex_member("file_alignment", image.file_alignment);
    writer.member("size_of_code", static_cast<std::uint64_t>(image.size_of_code));
    writer.member("size_of_initialized_data",
                  static_cast<std::uint64_t>(image.size_of_initialized_data));
    writer.end_object();

    write_sections(writer, image);
    write_directories(writer, image);
    write_imports(writer, directories, options);
    write_exports(writer, directories);
    write_functions(writer, directories, options);
    write_debug(writer, directories);
    write_relocations(writer, directories);
    write_tls(writer, directories);
    write_resources(writer, directories);
    write_rtti(writer, rtti, options);
    if (options.include_name_tables) {
        write_name_tables(writer, name_tables);
    }

    writer.end_object();
    output << '\n';
    return output.str();
}

} // namespace dmc::rengine::exe
