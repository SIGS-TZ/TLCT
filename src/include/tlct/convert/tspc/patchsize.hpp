#pragma once

#include <algorithm>
#include <ranges>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/quality.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::cvt::inline tspc {

namespace rgs = std::ranges;

namespace _helper {

static inline int estimatePatchsize(const cfg::tspc::Layout& layout, const cv::Mat& gray_src,
                                    const cv::Mat& psize_indices, const cv::Point index, const cv::Range match_range)
{
    const cv::Point2d curr_center = layout.getMICenter(index);
    const cv::Point2d neib_center = layout.getMICenter(index.y, index.x + 1);

    const int start_shift = -13 * layout.getUpsample();
    const int end_shift = -2 * layout.getUpsample();

    const cv::Range curr_cmp_row_range{iround(curr_center.y + start_shift), iround(curr_center.y + end_shift)};
    const cv::Range curr_cmp_col_range{iround(curr_center.x + start_shift), iround(curr_center.x + end_shift)};
    if (curr_cmp_row_range.end > gray_src.rows || curr_cmp_col_range.end > gray_src.cols) {
        return 0;
    }
    const cv::Mat curr_cmp = gray_src({curr_cmp_row_range, curr_cmp_col_range});
    const auto pssim_calc = cv::quality::QualitySSIM::create(curr_cmp);

    std::vector<double> ssims_over_mdist;
    ssims_over_mdist.reserve(match_range.size());
    for (const int mdist : rgs::views::iota(match_range.start, match_range.end)) {
        const cv::Range neib_cmp_row_range{iround(neib_center.y + start_shift), iround(neib_center.y + end_shift)};
        const cv::Range neib_cmp_col_range{iround(neib_center.x + start_shift) + mdist,
                                           iround(neib_center.x + end_shift) + mdist};
        if (neib_cmp_row_range.end > gray_src.rows || neib_cmp_col_range.end > gray_src.cols) {
            break;
        }

        const cv::Mat neib_cmp = gray_src({neib_cmp_row_range, neib_cmp_col_range});
        const double ssim = pssim_calc->compute(neib_cmp)[0];
        ssims_over_mdist.push_back(ssim);
    }

    const auto pmax_ssim = std::max_element(ssims_over_mdist.begin(), ssims_over_mdist.end());
    const int max_ssim_idx = (int)std::distance(ssims_over_mdist.begin(), pmax_ssim);
    int patchsize_idx;
    if (index.x == 0) {
        if (index.y == 0) {
            patchsize_idx = max_ssim_idx;
        } else {
            const int up_psize_idx = psize_indices.at<int>(index.y - 1, index.x);
            const double up_ssim = ssims_over_mdist[up_psize_idx];
            if (up_ssim > 0.85) {
                patchsize_idx = up_psize_idx;
            } else {
                patchsize_idx = max_ssim_idx; // TODO: Why `0`?
            }
        }
    } else {
        const int left_psize_idx = psize_indices.at<int>(index.y, index.x - 1);
        const double left_ssim = ssims_over_mdist[left_psize_idx];
        if (index.y == 0) {
            if (left_ssim > 0.85) {
                patchsize_idx = left_psize_idx;
            } else {
                patchsize_idx = max_ssim_idx;
            }
        } else {
            int ref_psize_idx;
            if ((index.x == psize_indices.cols - 2) && (index.y % 2 == 1)) {
                ref_psize_idx = psize_indices.at<int>(index.y - 1, index.x - 1);
            } else {
                ref_psize_idx = psize_indices.at<int>(index.y - 1, index.x);
            }
            const double ref_ssim = ssims_over_mdist[ref_psize_idx];
            if (left_ssim > 0.85 || ref_ssim > 0.85) {
                if (left_ssim < ref_ssim) {
                    patchsize_idx = ref_psize_idx;
                } else {
                    patchsize_idx = left_psize_idx;
                }
            } else {
                patchsize_idx = max_ssim_idx;
            }
        }
    }

    return patchsize_idx;
}

} // namespace _helper

TLCT_API inline void estimatePatchsizes_(const cfg::tspc::Layout& layout, const cv::Mat& src, cv::Mat& patchsizes)
{
    cv::Mat psize_indices = cv::Mat::zeros(layout.getMIRows(), layout.getMICols(), CV_32SC1);

    cv::Mat gray_src;
    cv::cvtColor(src, gray_src, cv::COLOR_BGR2GRAY);

    const int match_start = 15 * layout.getUpsample();
    const int match_end = 29 * layout.getUpsample();

    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols() - 1)) {
            const cv::Point2d neib_center = layout.getMICenter(row, col + 1);
            if (neib_center.x == 0.0 or neib_center.y == 0.0)
                continue;

            const cv::Point index{col, row};
            const int patchsize_idx =
                _helper::estimatePatchsize(layout, gray_src, psize_indices, index, {match_start, match_end});
            psize_indices.at<int>(row, col) = patchsize_idx;
        }
    }
    psize_indices.col(layout.getMICols() - 2).copyTo(psize_indices.col(layout.getMICols() - 1));
    patchsizes = psize_indices + match_start;
}

TLCT_API inline cv::Mat estimatePatchsizes(const cfg::tspc::Layout& layout, const cv::Mat& src)
{
    cv::Mat patchsizes;
    estimatePatchsizes_(layout, src, patchsizes);
    return patchsizes;
}

} // namespace tlct::cvt::inline tspc