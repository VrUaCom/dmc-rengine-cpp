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

struct GroupSelector {
  std::string value;
  bool prefix{};

  bool matches(std::string_view group) const noexcept {
    if (!prefix) return group == value;
    return group.size() >= value.size() && group.substr(0, value.size()) == value;
  }
};

GroupSelector parse_selector(std::string_view text) {
  constexpr std::string_view marker = "prefix:";
  if (text.substr(0, marker.size()) == marker) {
    return {std::string{text.substr(marker.size())}, true};
  }
  return {std::string{text}, false};
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "usage: lsg_extract_obj_group <input.obj> <group|prefix:group-prefix> <output.obj>\n";
    return 2;
  }

  const std::filesystem::path input = argv[1];
  const std::string selector_text = argv[2];
  const GroupSelector selector = parse_selector(selector_text);
  const std::filesystem::path output = argv[3];

  if (selector.value.empty()) {
    std::cerr << "OBJ group extraction failed: selector is empty\n";
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

  out << "# Rengine LSG filtered OBJ; selector: " << selector_text << '\n';

  while (std::getline(in, line)) {
    std::istringstream parser(line);
    std::string tag;
    parser >> tag;

    if (tag == "g") {
      std::string remainder;
      std::getline(parser, remainder);
      current_group = trim_copy(remainder);
      ++group_declarations;
      if (selector.matches(current_group)) {
        ++selected_group_declarations;
        out << "g " << current_group << '\n';
      }
      continue;
    }

    if (tag == "f") {
      if (selector.matches(current_group)) {
        out << line << '\n';
        ++selected_faces;
      } else {
        ++skipped_faces;
      }
      continue;
    }

    // Preserve global OBJ tables so original face indices remain valid. RMS0 ignores
    // object/material state, therefore non-selected helper groups cannot leak through.
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
    std::cerr << "OBJ group extraction failed: selector '" << selector_text << "' matched no groups\n";
    return 8;
  }
  if (selected_faces == 0) {
    std::cerr << "OBJ group extraction failed: selected groups contain no faces\n";
    return 9;
  }

  std::cout << "OBJ GROUP FILTER PASS: selector=" << selector_text
            << " selected_groups=" << selected_group_declarations
            << " selected_faces=" << selected_faces
            << " skipped_faces=" << skipped_faces
            << " groups_seen=" << group_declarations << '\n';
  return 0;
}
