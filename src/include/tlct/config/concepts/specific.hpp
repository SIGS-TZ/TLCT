#pragma once

#include <concepts>

#include "tlct/config/common/map.hpp"

namespace tlct::_cfg::concepts {

template <typename Self>
concept CSpecificConfig = std::is_trivially_copyable_v<Self> && requires {
    // Constructor
    { Self() } -> std::same_as<Self>;
} && requires(const ConfigMap& cfg_map) {
    // Init from
    { Self::fromConfigMap(cfg_map) } -> std::same_as<Self>;
};

} // namespace tlct::_cfg::concepts
