#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace {

std::string trim_copy(std::string value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return {};
  const auto last = value.find_last_not_of(" \t\r\n");
  return value.substr(first, last - first + 1);
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "usage: lsg_extract_obj_group <input.obj> <group> <output.obj>\n";
    return 2;
  }

  const std::filesystem::path input = argv[1];
  const std::string requested_group = argv[2];
  const std::filesystem::path output = argv[3];

  if (requested_group.empty()) {
    std::cerr << "OBJ group extraction failed: group name is empty\n";
    return 3;
  }

  std::ifstream in(input);
  if (!in) {
    std::cerr << "OBJ group extraction failed: cannot open input\n";
    return 4;
  }
  std::ofstream out(output);
  if (!out) {
    std::cerr << "OBJ group extraction failed: cannot open output\n";
    return 5;
  }

  std::string current_group;
  std::size_t group_declarations = 0;
  std::size_t selected_faces = 0;
  std::size_t skipped_faces = 0;
  std::size_t selected_group_declarations = 0;
  std::string line;

  out << "# Rengine LSG filtered OBJ; selected group: " << requested_group << '\n';

  while (std::getline(in, line)) {
    std::istringstream parser(line);
    std::string tag;
    parser >> tag;

    if (tag == "g") {
      std::string remainder;
      std::getline(parser, remainder);
      current_group = trim_copy(remainder);
      ++group_declarations;
      if (current_group == requested_group) {
        ++selected_group_declarations;
        out << "g " << requested_group << '\n';
      }
      continue;
    }

    if (tag == "f") {
      if (current_group == requested_group) {
        out << line << '\n';
        ++selected_faces;
      } else {
        ++skipped_faces;
      }
      continue;
    }

    // Preserve global OBJ tables and harmless metadata so original face indices remain valid.
    // Object/material state is not needed by RMS0, but comments and vertex tables are safe.
    if (tag == "v" || tag == "vt" || tag == "vn" || tag == "vp" || tag == "#" || tag.empty()) {
      out << line << '\n';
    }
  }

  if (!out) {
    std::cerr << "OBJ group extraction failed: write error\n";
    return 6;
  }
  if (group_declarations == 0) {
    std::cerr << "OBJ group extraction failed: input contains no group declarations\n";
    return 7;
  }
  if (selected_group_declarations == 0) {
    std::cerr << "OBJ group extraction failed: requested group '" << requested_group << "' was not found\n";
    return 8;
  }
  if (selected_faces == 0) {
    std::cerr << "OBJ group extraction failed: requested group contains no faces\n";
    return 9;
  }

  std::cout << "OBJ GROUP FILTER PASS: group=" << requested_group
            << " selected_faces=" << selected_faces
            << " skipped_faces=" << skipped_faces
            << " groups_seen=" << group_declarations << '\n';
  return 0;
}
