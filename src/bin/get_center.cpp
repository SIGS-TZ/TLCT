#include <iostream>

#include "tlct.hpp"

namespace tcfg = tlct::cfg::tspc;

int main(int argc, char* argv[])
{
    using ParamConfig = tcfg::ParamConfig<tcfg::CalibConfig>;

    const auto cfg_map = tlct::cfg::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = ParamConfig::fromConfigMap(cfg_map);

    constexpr int upsample = 1;
    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getImgSize()).upsample(upsample);

    const int row = std::stoi(argv[2]);
    const int col = std::stoi(argv[3]);

    const auto center = layout.getMICenter(row, col);
    std::cout << center << std::endl;
}