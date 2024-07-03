#include <filesystem>
#include <sstream>

#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tcfg = tlct::cfg::tspc;
namespace tcvt = tlct::cvt::tspc;

int main(int argc, char* argv[])
{
    const auto cfg_map = tlct::cfg::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = tcfg::ParamConfig::fromConfigMap(cfg_map);
    const auto& common_cfg = param_cfg.getGenericCfg();

    constexpr int upsample = 2;
    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getSpecificCfg().getImgSize())
            .upsample(upsample);
    auto state = tcvt::State::fromParamCfg(param_cfg);
    state.setInspector(tcvt::_hp::Inspector::fromCommonCfgAndLayout(common_cfg, layout));

    const cv::Range range = common_cfg.getRange();
    for (int i = range.start; i <= range.end; i++) {
        const auto srcpath = common_cfg.fmtSrcPath(i);
        state.feed(cv::imread(srcpath.string()));

        int img_cnt = 1;
        const auto dstdir = common_cfg.fmtDstPath(i);
        fs::create_directories(dstdir);

        for (const auto& mv : state) {
            std::stringstream filename_s;
            filename_s << "image_" << std::setw(3) << std::setfill('0') << img_cnt << ".png";
            const fs::path saveto_path = dstdir / filename_s.str();
            cv::imwrite(saveto_path.string(), mv);
            img_cnt++;
        }
    }
}
