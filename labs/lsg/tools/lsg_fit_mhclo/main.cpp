#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Vec3d {
  double x{};
  double y{};
  double z{};
};

struct ScaleAxisSpec {
  std::uint32_t v0{};
  std::uint32_t v1{};
  double denominator{};
  bool valid{};
};

struct ProxyVertexMap {
  std::array<std::uint32_t, 3> base_vertices{};
  std::array<double, 3> weights{};
  Vec3d offset{};
};

struct MhcloProxy {
  ScaleAxisSpec x_scale;
  ScaleAxisSpec y_scale;
  ScaleAxisSpec z_scale;
  std::vector<ProxyVertexMap> mappings;
};

bool finite(const Vec3d& v) noexcept {
  return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

std::string trim(std::string_view input) {
  const auto first = input.find_first_not_of(" \t\r\n");
  if (first == std::string_view::npos) return {};
  const auto last = input.find_last_not_of(" \t\r\n");
  return std::string(input.substr(first, last - first + 1));
}

bool parse_u32(std::string_view token, std::uint32_t& out) {
  if (token.empty()) return false;
  std::uint64_t value{};
  const char* begin = token.data();
  const char* end = begin + token.size();
  const auto result = std::from_chars(begin, end, value);
  if (result.ec != std::errc{} || result.ptr != end ||
      value > std::numeric_limits<std::uint32_t>::max()) {
    return false;
  }
  out = static_cast<std::uint32_t>(value);
  return true;
}

bool parse_double(std::string_view token, double& out) {
  if (token.empty()) return false;
  char* end = nullptr;
  std::string copy(token);
  out = std::strtod(copy.c_str(), &end);
  return end != copy.c_str() && *end == '\0' && std::isfinite(out);
}

std::vector<std::string> split_words(const std::string& line) {
  std::istringstream stream(line);
  std::vector<std::string> words;
  std::string word;
  while (stream >> word) words.push_back(word);
  return words;
}

bool load_obj_positions(const std::filesystem::path& path,
                        std::vector<std::string>& lines,
                        std::vector<Vec3d>& positions,
                        std::vector<std::size_t>& vertex_line_indices,
                        std::string& error) {
  std::ifstream input(path);
  if (!input) {
    error = "cannot open OBJ: " + path.string();
    return false;
  }

  std::string line;
  std::size_t line_number = 0;
  while (std::getline(input, line)) {
    ++line_number;
    lines.push_back(line);
    if (line.size() < 2 || line[0] != 'v' || line[1] != ' ') continue;

    const auto words = split_words(line);
    if (words.size() < 4 || words[0] != "v") {
      error = "invalid OBJ vertex row at line " + std::to_string(line_number);
      return false;
    }

    Vec3d p{};
    if (!parse_double(words[1], p.x) || !parse_double(words[2], p.y) ||
        !parse_double(words[3], p.z) || !finite(p)) {
      error = "invalid/non-finite OBJ vertex at line " + std::to_string(line_number);
      return false;
    }

    positions.push_back(p);
    vertex_line_indices.push_back(lines.size() - 1u);
  }

  if (positions.empty()) {
    error = "OBJ contains no vertex positions: " + path.string();
    return false;
  }
  return true;
}

bool parse_scale(const std::vector<std::string>& words,
                 ScaleAxisSpec& out,
                 std::size_t line_number,
                 std::string& error) {
  if (words.size() != 4u) {
    error = "invalid MHClO scale row at line " + std::to_string(line_number);
    return false;
  }

  std::uint32_t v0{}, v1{};
  double denominator{};
  if (!parse_u32(words[1], v0) || !parse_u32(words[2], v1) ||
      !parse_double(words[3], denominator) || denominator == 0.0) {
    error = "invalid MHClO scale values at line " + std::to_string(line_number);
    return false;
  }

  out = ScaleAxisSpec{v0, v1, denominator, true};
  return true;
}

bool is_numeric_lead(std::string_view token) {
  if (token.empty()) return false;
  const char c = token.front();
  return (c >= '0' && c <= '9') || c == '-' || c == '+';
}

bool load_mhclo(const std::filesystem::path& path,
                MhcloProxy& out,
                std::string& error) {
  std::ifstream input(path);
  if (!input) {
    error = "cannot open MHClO: " + path.string();
    return false;
  }

  MhcloProxy proxy{};
  bool in_verts = false;
  std::string line;
  std::size_t line_number = 0;

  while (std::getline(input, line)) {
    ++line_number;
    const auto cleaned = trim(line);
    if (cleaned.empty() || cleaned.front() == '#') continue;

    const auto words = split_words(cleaned);
    if (words.empty()) continue;

    if (!in_verts) {
      const auto& key = words[0];
      if (key == "x_scale") {
        if (!parse_scale(words, proxy.x_scale, line_number, error)) return false;
      } else if (key == "y_scale") {
        if (!parse_scale(words, proxy.y_scale, line_number, error)) return false;
      } else if (key == "z_scale") {
        if (!parse_scale(words, proxy.z_scale, line_number, error)) return false;
      } else if (key == "verts") {
        in_verts = true;
      }
      continue;
    }

    if (!is_numeric_lead(words[0])) {
      break;
    }

    if (words.size() != 9u) {
      error = "unsupported MHClO proxy mapping row at line " + std::to_string(line_number) +
              " (expected triple reference with 9 fields)";
      return false;
    }

    ProxyVertexMap mapping{};
    for (std::size_t i = 0; i < 3u; ++i) {
      if (!parse_u32(words[i], mapping.base_vertices[i])) {
        error = "invalid MHClO base vertex index at line " + std::to_string(line_number);
        return false;
      }
    }
    for (std::size_t i = 0; i < 3u; ++i) {
      if (!parse_double(words[3u + i], mapping.weights[i])) {
        error = "invalid MHClO weight at line " + std::to_string(line_number);
        return false;
      }
    }
    if (!parse_double(words[6], mapping.offset.x) ||
        !parse_double(words[7], mapping.offset.y) ||
        !parse_double(words[8], mapping.offset.z) ||
        !finite(mapping.offset)) {
      error = "invalid MHClO offset at line " + std::to_string(line_number);
      return false;
    }

    const double weight_sum =
        mapping.weights[0] + mapping.weights[1] + mapping.weights[2];
    if (!std::isfinite(weight_sum) || std::abs(weight_sum - 1.0) > 0.10) {
      error = "MHClO mapping weights are not plausibly normalized at line " +
              std::to_string(line_number);
      return false;
    }

    proxy.mappings.push_back(mapping);
  }

  if (!proxy.x_scale.valid || !proxy.y_scale.valid || !proxy.z_scale.valid) {
    error = "MHClO is missing x_scale/y_scale/z_scale";
    return false;
  }
  if (!in_verts) {
    error = "MHClO is missing verts section";
    return false;
  }
  if (proxy.mappings.empty()) {
    error = "MHClO contains no proxy mappings";
    return false;
  }

  out = std::move(proxy);
  return true;
}

bool validate_base_references(const MhcloProxy& proxy,
                              std::size_t base_vertex_count,
                              std::string& error) {
  auto valid_scale = [&](const ScaleAxisSpec& spec, std::string_view name) {
    if (spec.v0 >= base_vertex_count || spec.v1 >= base_vertex_count) {
      error = std::string("MHClO ") + std::string(name) +
              " references a missing base vertex";
      return false;
    }
    if (!std::isfinite(spec.denominator) || spec.denominator == 0.0) {
      error = std::string("MHClO ") + std::string(name) +
              " denominator is invalid";
      return false;
    }
    return true;
  };

  if (!valid_scale(proxy.x_scale, "x_scale") ||
      !valid_scale(proxy.y_scale, "y_scale") ||
      !valid_scale(proxy.z_scale, "z_scale")) {
    return false;
  }

  for (std::size_t i = 0; i < proxy.mappings.size(); ++i) {
    const auto& mapping = proxy.mappings[i];
    for (const auto index : mapping.base_vertices) {
      if (index >= base_vertex_count) {
        error = "MHClO mapping " + std::to_string(i) +
                " references base vertex " + std::to_string(index) +
                " outside profile OBJ";
        return false;
      }
    }
  }
  return true;
}

double axis_scale(const ScaleAxisSpec& spec,
                  const std::vector<Vec3d>& base,
                  int axis) {
  const auto component = [&](const Vec3d& v) {
    return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
  };
  return std::abs(component(base[spec.v0]) - component(base[spec.v1])) /
         spec.denominator;
}

bool fit_proxy(const std::vector<Vec3d>& base,
               const MhcloProxy& proxy,
               std::vector<Vec3d>& fitted,
               std::array<double, 3>& scales,
               std::string& error) {
  if (!validate_base_references(proxy, base.size(), error)) return false;

  scales = {
      axis_scale(proxy.x_scale, base, 0),
      axis_scale(proxy.y_scale, base, 1),
      axis_scale(proxy.z_scale, base, 2),
  };

  if (!std::isfinite(scales[0]) || !std::isfinite(scales[1]) ||
      !std::isfinite(scales[2]) || scales[0] <= 0.0 || scales[1] <= 0.0 ||
      scales[2] <= 0.0) {
    error = "computed MHClO scale matrix is invalid";
    return false;
  }

  fitted.clear();
  fitted.reserve(proxy.mappings.size());
  for (const auto& mapping : proxy.mappings) {
    Vec3d p{};
    for (std::size_t i = 0; i < 3u; ++i) {
      const auto& b = base[mapping.base_vertices[i]];
      const double w = mapping.weights[i];
      p.x += b.x * w;
      p.y += b.y * w;
      p.z += b.z * w;
    }
    p.x += scales[0] * mapping.offset.x;
    p.y += scales[1] * mapping.offset.y;
    p.z += scales[2] * mapping.offset.z;

    if (!finite(p)) {
      error = "MHClO fitting produced NaN/Inf";
      return false;
    }
    fitted.push_back(p);
  }
  return true;
}

bool write_fitted_obj(const std::filesystem::path& path,
                      std::vector<std::string> eye_lines,
                      const std::vector<std::size_t>& eye_vertex_line_indices,
                      const std::vector<Vec3d>& fitted,
                      std::string& error) {
  if (eye_vertex_line_indices.size() != fitted.size()) {
    error = "eye OBJ vertex count does not match fitted MHClO mapping count";
    return false;
  }

  for (std::size_t i = 0; i < fitted.size(); ++i) {
    std::ostringstream row;
    row.setf(std::ios::fixed);
    row << std::setprecision(9)
        << "v " << fitted[i].x << ' ' << fitted[i].y << ' ' << fitted[i].z;
    eye_lines[eye_vertex_line_indices[i]] = row.str();
  }

  std::ofstream output(path, std::ios::binary);
  if (!output) {
    error = "cannot open output OBJ: " + path.string();
    return false;
  }

  output << "# Rengine LSG fitted MakeHuman eye proxy\n";
  for (const auto& line : eye_lines) output << line << '\n';

  if (!output) {
    error = "failed while writing fitted OBJ";
    return false;
  }
  return true;
}

void update_bounds(const Vec3d& p,
                   Vec3d& minimum,
                   Vec3d& maximum) {
  minimum.x = std::min(minimum.x, p.x);
  minimum.y = std::min(minimum.y, p.y);
  minimum.z = std::min(minimum.z, p.z);
  maximum.x = std::max(maximum.x, p.x);
  maximum.y = std::max(maximum.y, p.y);
  maximum.z = std::max(maximum.z, p.z);
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 5) {
    std::cerr << "usage: lsg_fit_mhclo <profile.obj> <eye.obj> <eye.mhclo> <fitted-eye.obj>\n";
    return 2;
  }

  const std::filesystem::path profile_path = argv[1];
  const std::filesystem::path eye_path = argv[2];
  const std::filesystem::path mhclo_path = argv[3];
  const std::filesystem::path output_path = argv[4];

  std::vector<std::string> profile_lines;
  std::vector<Vec3d> profile_positions;
  std::vector<std::size_t> profile_vertex_lines;
  std::string error;
  if (!load_obj_positions(profile_path, profile_lines, profile_positions,
                          profile_vertex_lines, error)) {
    std::cerr << "MHClO fit failed: " << error << '\n';
    return 3;
  }

  std::vector<std::string> eye_lines;
  std::vector<Vec3d> eye_positions;
  std::vector<std::size_t> eye_vertex_lines;
  if (!load_obj_positions(eye_path, eye_lines, eye_positions,
                          eye_vertex_lines, error)) {
    std::cerr << "MHClO fit failed: " << error << '\n';
    return 4;
  }

  MhcloProxy proxy{};
  if (!load_mhclo(mhclo_path, proxy, error)) {
    std::cerr << "MHClO fit failed: " << error << '\n';
    return 5;
  }

  if (proxy.mappings.size() != eye_positions.size()) {
    std::cerr << "MHClO fit failed: mapping count " << proxy.mappings.size()
              << " does not match eye OBJ vertex count " << eye_positions.size()
              << '\n';
    return 6;
  }

  std::vector<Vec3d> fitted;
  std::array<double, 3> scales{};
  if (!fit_proxy(profile_positions, proxy, fitted, scales, error)) {
    std::cerr << "MHClO fit failed: " << error << '\n';
    return 7;
  }

  if (!write_fitted_obj(output_path, std::move(eye_lines),
                        eye_vertex_lines, fitted, error)) {
    std::cerr << "MHClO fit failed: " << error << '\n';
    return 8;
  }

  Vec3d minimum{
      std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::infinity()};
  Vec3d maximum{
      -std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity()};
  for (const auto& p : fitted) update_bounds(p, minimum, maximum);

  std::cout.setf(std::ios::fixed);
  std::cout << std::setprecision(9)
            << "MHCL0 FIT PASS: profile_vertices=" << profile_positions.size()
            << " eye_vertices=" << fitted.size()
            << " scale=(" << scales[0] << ',' << scales[1] << ',' << scales[2] << ')'
            << " bounds_min=(" << minimum.x << ',' << minimum.y << ',' << minimum.z << ')'
            << " bounds_max=(" << maximum.x << ',' << maximum.y << ',' << maximum.z << ')'
            << '\n';
  return 0;
}
