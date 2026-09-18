#include <cmath>
#include <cstddef>
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

struct TargetSpec {
  std::filesystem::path path;
  double weight{};
};

bool parse_weight(const char* text, double& out) {
  if (text == nullptr || *text == '\0') return false;
  char* end = nullptr;
  out = std::strtod(text, &end);
  return end != text && *end == '\0' && std::isfinite(out);
}

bool load_obj(const std::filesystem::path& path,
              std::vector<std::string>& lines,
              std::vector<Vec3d>& positions,
              std::vector<std::size_t>& vertex_line_indices,
              std::string& error) {
  std::ifstream stream(path);
  if (!stream) {
    error = "cannot open input OBJ";
    return false;
  }

  std::string line;
  std::size_t line_number = 0;
  while (std::getline(stream, line)) {
    ++line_number;
    lines.push_back(line);

    std::istringstream parser(line);
    std::string tag;
    parser >> tag;
    if (tag != "v") continue;

    Vec3d p{};
    if (!(parser >> p.x >> p.y >> p.z) ||
        !std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) {
      error = "invalid OBJ vertex at line " + std::to_string(line_number);
      return false;
    }
    positions.push_back(p);
    vertex_line_indices.push_back(lines.size() - 1u);
  }

  if (positions.empty()) {
    error = "OBJ contains no vertices";
    return false;
  }
  return true;
}

bool apply_target(const TargetSpec& spec,
                  std::vector<Vec3d>& positions,
                  std::size_t& entries,
                  double& max_weighted_delta,
                  std::string& error) {
  std::ifstream stream(spec.path);
  if (!stream) {
    error = "cannot open target: " + spec.path.string();
    return false;
  }

  std::string line;
  std::size_t line_number = 0;
  std::size_t target_entries = 0;
  while (std::getline(stream, line)) {
    ++line_number;
    if (line.empty() || line[0] == '#') continue;

    std::istringstream parser(line);
    std::size_t vertex_index{};
    double dx{}, dy{}, dz{};
    if (!(parser >> vertex_index >> dx >> dy >> dz) ||
        !std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(dz)) {
      error = "invalid target row " + spec.path.string() + ":" + std::to_string(line_number);
      return false;
    }
    if (vertex_index >= positions.size()) {
      error = "target vertex index out of range in " + spec.path.string() +
              ":" + std::to_string(line_number);
      return false;
    }

    const double wx = dx * spec.weight;
    const double wy = dy * spec.weight;
    const double wz = dz * spec.weight;
    positions[vertex_index].x += wx;
    positions[vertex_index].y += wy;
    positions[vertex_index].z += wz;

    const double magnitude = std::sqrt(wx * wx + wy * wy + wz * wz);
    max_weighted_delta = std::max(max_weighted_delta, magnitude);
    ++target_entries;
  }

  if (target_entries == 0) {
    error = "target contains no deltas: " + spec.path.string();
    return false;
  }
  entries += target_entries;
  return true;
}

bool write_obj(const std::filesystem::path& path,
               std::vector<std::string> lines,
               const std::vector<Vec3d>& positions,
               const std::vector<std::size_t>& vertex_line_indices,
               std::string& error) {
  if (positions.size() != vertex_line_indices.size()) {
    error = "internal vertex-line mapping mismatch";
    return false;
  }

  for (std::size_t i = 0; i < positions.size(); ++i) {
    const auto& p = positions[i];
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) {
      error = "non-finite output vertex";
      return false;
    }
    std::ostringstream row;
    row << std::setprecision(10) << "v " << p.x << ' ' << p.y << ' ' << p.z;
    lines[vertex_line_indices[i]] = row.str();
  }

  std::ofstream stream(path);
  if (!stream) {
    error = "cannot open output OBJ";
    return false;
  }
  stream << "# Rengine LSG MakeHuman macro target application\n";
  for (const auto& line : lines) stream << line << '\n';
  if (!stream) {
    error = "failed while writing output OBJ";
    return false;
  }
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 5 || ((argc - 3) % 2) != 0) {
    std::cerr << "usage: lsg_apply_makehuman_targets <input.obj> <output.obj> "
                 "<target1> <weight1> [<target2> <weight2> ...]\n";
    return 2;
  }

  const std::filesystem::path input = argv[1];
  const std::filesystem::path output = argv[2];

  std::vector<TargetSpec> targets;
  double weight_sum = 0.0;
  for (int i = 3; i + 1 < argc; i += 2) {
    double weight{};
    if (!parse_weight(argv[i + 1], weight) || weight < 0.0 || weight > 1.0) {
      std::cerr << "MakeHuman target application failed: weight must be finite in [0,1]\n";
      return 3;
    }
    targets.push_back({argv[i], weight});
    weight_sum += weight;
  }

  if (targets.empty() || !(weight_sum > 0.0) || std::abs(weight_sum - 1.0) > 1.0e-6) {
    std::cerr << "MakeHuman target application failed: target weights must sum to 1.0\n";
    return 4;
  }

  std::vector<std::string> lines;
  std::vector<Vec3d> positions;
  std::vector<std::size_t> vertex_line_indices;
  std::string error;
  if (!load_obj(input, lines, positions, vertex_line_indices, error)) {
    std::cerr << "MakeHuman target application failed: " << error << '\n';
    return 5;
  }

  std::size_t entries = 0;
  double max_weighted_delta = 0.0;
  for (const auto& target : targets) {
    if (!apply_target(target, positions, entries, max_weighted_delta, error)) {
      std::cerr << "MakeHuman target application failed: " << error << '\n';
      return 6;
    }
  }

  if (!write_obj(output, std::move(lines), positions, vertex_line_indices, error)) {
    std::cerr << "MakeHuman target application failed: " << error << '\n';
    return 7;
  }

  std::cout << "MAKEHUMAN TARGET PASS: vertices=" << positions.size()
            << " targets=" << targets.size()
            << " entries=" << entries
            << " weight_sum=" << std::fixed << std::setprecision(6) << weight_sum
            << " max_weighted_delta=" << std::setprecision(8) << max_weighted_delta
            << '\n';
  return 0;
}
