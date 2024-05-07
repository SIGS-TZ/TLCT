#pragma once

#include <algorithm>
#include <ranges>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/quality.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/calibration/tspc.hpp"

namespace tlct::cvt::tspc {

namespace rgs = std::ranges;

namespace _helper {

static inline std::vector<double> matchWithSSIM(const cv::Mat& gray_src, const cv::Point curr_center,
                                                const cv::Point neib_center, const cv::Range match_range)
{
    constexpr int start_shift = -13;
    constexpr int end_shift = -3;
    const cv::Range curr_cmp_roi[]{{curr_center.y + start_shift, curr_center.y + end_shift},
                                   {curr_center.x + start_shift, curr_center.x + end_shift}};
    const cv::Mat curr_cmp = gray_src(curr_cmp_roi);
    const auto pssim_calc = cv::quality::QualitySSIM::create(curr_cmp);

    std::vector<double> ssims_over_mdist;
    ssims_over_mdist.reserve(match_range.size());
    for (const int mdist : rgs::views::iota(match_range.start, match_range.end)) {
        const cv::Range neib_cmp_roi[]{{neib_center.y + start_shift + mdist, neib_center.y + end_shift + mdist},
                                       {neib_center.x + start_shift, neib_center.x + end_shift}};
        const cv::Mat neib_cmp = gray_src(neib_cmp_roi);
        const auto ssims = pssim_calc->compute(neib_cmp);
        const double ssim = ssims[0];
        ssims_over_mdist.push_back(ssim);
    }

    return ssims_over_mdist;
}

static inline int yieldPatchsizeIndex(const std::vector<double>& ssims_over_mdist, const cv::Mat& psize_indices,
                                      const cv::Point index)
{
    const auto pmax_ssim = std::max_element(ssims_over_mdist.begin(), ssims_over_mdist.end());
    const int max_ssim_idx = (int)std::distance(ssims_over_mdist.begin(), pmax_ssim);
    int patchsize_idx;
    if (index.y == 0) {
        if (index.x == 0) {
            patchsize_idx = max_ssim_idx;
        } else {
            const int left_psize_idx = psize_indices.at<int>(index.y, index.x - 1);
            if (ssims_over_mdist[left_psize_idx] > 0.85) {
                patchsize_idx = left_psize_idx;
            } else {
                patchsize_idx = max_ssim_idx; // TODO: Why `0`?
            }
        }
    } else {
        const int up_psize_idx = psize_indices.at<int>(index.y - 1, index.x);
        const double up_ssim = ssims_over_mdist[up_psize_idx];
        if (index.x == 0) {
            if (up_ssim > 0.85) {
                patchsize_idx = up_psize_idx;
            } else {
                patchsize_idx = max_ssim_idx;
            }
        } else {
            int ref_psize_idx;
            if ((index.y == psize_indices.rows - 2) && (index.x % 2 == 1)) {
                ref_psize_idx = psize_indices.at<int>(index.y - 1, index.x - 1);
            } else {
                ref_psize_idx = psize_indices.at<int>(index.y, index.x - 1);
            }
            const double ref_ssim = ssims_over_mdist[ref_psize_idx];
            if (up_ssim > 0.85 || ref_ssim > 0.85) {
                if (up_ssim < ref_ssim) {
                    patchsize_idx = ref_psize_idx;
                } else {
                    patchsize_idx = up_psize_idx;
                }
            } else {
                patchsize_idx = max_ssim_idx;
            }
        }
    }

    return patchsize_idx;
}

} // namespace _helper

TLCT_API inline void generatePatchsizes_(const cv::Mat& src, cv::Mat& patchsizes, const cfg::tspc::CalibConfig& config)
{
    const cv::Size size = config.getCentersSize();
    cv::Mat psize_indices = cv::Mat::zeros(size, CV_32SC1);

    cv::Mat gray_src;
    cv::cvtColor(src, gray_src, cv::COLOR_BGR2GRAY);

    const cv::Range match_range{15, 34};

    for (const int row : rgs::views::iota(0, size.height - 1)) {
        for (const int col : rgs::views::iota(0, size.width)) {
            const cv::Point curr_center = config.getCenter(row, col);
            const cv::Point neib_center = config.getCenter(row + 1, col);
            if (neib_center.x == 0 or neib_center.y == 0)
                continue;

            const auto ssims_over_mdist = _helper::matchWithSSIM(gray_src, curr_center, neib_center, match_range);
            const int patchsize_idx = _helper::yieldPatchsizeIndex(ssims_over_mdist, psize_indices, {col, row});
            psize_indices.at<int>(row, col) = patchsize_idx;
        }
    }
    psize_indices.row(size.height - 2).copyTo(psize_indices.row(size.height - 1));
    patchsizes = psize_indices + match_range.start;
}

TLCT_API inline cv::Mat generatePatchsizes(const cv::Mat& src, const cfg::tspc::CalibConfig& config)
{
    cv::Mat patchsizes;
    generatePatchsizes_(src, patchsizes, config);
    return patchsizes;
}

} // namespace tlct::cvt::tspc