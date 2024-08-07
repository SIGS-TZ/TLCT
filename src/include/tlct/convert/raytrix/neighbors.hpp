#pragma once

#include "tlct/config/raytrix.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper/neighbors.hpp"

namespace tlct::_cvt::raytrix {

namespace tcfg = tlct::cfg::raytrix;

using Neighbors = Neighbors_<tcfg::Layout>;

static_assert(tlct::cvt::concepts::CNeighbors<Neighbors>);

} // namespace tlct::_cvt::raytrix
