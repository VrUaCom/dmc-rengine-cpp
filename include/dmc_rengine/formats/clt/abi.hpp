#pragma once

#include <string_view>

namespace dmc::rengine::formats::clt {

struct TextAbi final {
    static constexpr char comment_prefix = ';';
    static constexpr std::string_view embedded_extension = ".clt";
    static constexpr std::string_view terminator = "$";
    static constexpr std::string_view record_end = "End";

    static constexpr std::string_view key_cloth_num = "ClothNum";
    static constexpr std::string_view key_cloth_no = "ClothNo";
    static constexpr std::string_view key_cloth_id = "ClothId";
    static constexpr std::string_view key_gravity = "Gravity";
    static constexpr std::string_view key_spring_force = "SpringForce";
    static constexpr std::string_view key_max_speed = "MaxSpeed";
    static constexpr std::string_view key_stiffness = "Stiffness";
    static constexpr std::string_view key_wind = "Wind";
    static constexpr std::string_view key_wind_local = "WindLocal";
    static constexpr std::string_view key_wind_parent = "WindParent";
    static constexpr std::string_view key_wind_type = "WindType";
    static constexpr std::string_view key_bone = "Bone";
    static constexpr std::string_view key_limit_length = "LimitLength";
};

} // namespace dmc::rengine::formats::clt
