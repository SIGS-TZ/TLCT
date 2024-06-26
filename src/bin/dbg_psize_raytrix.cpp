#include <filesystem>
#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tcfg = tlct::cfg::raytrix;
namespace tcvt = tlct::cvt::raytrix;

int main(int argc, char* argv[])
{
    using ParamConfig = tcfg::ParamConfig<tcfg::CalibConfig>;

    const auto cfg_map = tlct::cfg::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = ParamConfig::fromConfigMap(cfg_map);
    const auto& common_cfg = param_cfg.getCommonCfg();

    constexpr int upsample = 2;
    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getImgSize()).upsample(upsample);
    auto state = tcvt::State::fromLayoutAndViews(layout, common_cfg.getViews());

    const auto srcpath = tlct::cfg::CommonParamConfig::fmtSrcPath(common_cfg, 1);

    const cv::Mat& src = cv::imread(srcpath.string());
    state.feed(src);

    const cv::Mat& patchsizes = tcvt::estimatePatchsizes(state);

    std::cout << patchsizes << std::endl;
}