#pragma once

#include <ranges>
#include <string>

#include <opencv2/core.hpp>
#include <pugixml.hpp>

#include "tlct/common/defines.h"

namespace tlct::cfg::inline tspc {

namespace rgs = std::ranges;

class CalibConfig
{
public:
    friend class Layout;

    TLCT_API CalibConfig() noexcept : micenters_(), diameter_(0.0), rotation_(0.0) {};
    TLCT_API CalibConfig(const CalibConfig& cfg)
        : micenters_(cfg.micenters_.clone()), diameter_(cfg.diameter_), rotation_(cfg.rotation_) {};
    TLCT_API CalibConfig(CalibConfig&& cfg) noexcept
        : micenters_(std::move(cfg.micenters_)), diameter_(cfg.diameter_), rotation_(cfg.rotation_) {};
    TLCT_API CalibConfig(cv::Mat&& micenters, double diameter, double rotation) noexcept
        : micenters_(micenters), diameter_(diameter), rotation_(rotation) {};

    TLCT_API static CalibConfig fromXMLDoc(const pugi::xml_document& doc);
    TLCT_API static CalibConfig fromXMLPath(const char* path);

    [[nodiscard]] TLCT_API double getDiameter() const noexcept;
    [[nodiscard]] TLCT_API double getRotation() const noexcept;

private:
    cv::Mat micenters_; // CV_64FC2
    double diameter_;
    double rotation_;
};

inline CalibConfig CalibConfig::fromXMLDoc(const pugi::xml_document& doc)
{

    const auto data_node = doc.child("TSPCCalibData");
    const double diameter = data_node.child("diameter").text().as_double();
    const double rotation = data_node.child("rotation").text().as_double();

    const auto centers_node = data_node.child("centers");
    const int rows = centers_node.child("rows").text().as_int();
    const int cols = centers_node.child("cols").text().as_int();

    const std::string coord_str = centers_node.child("coords").text().as_string();
    auto subrg_view = coord_str | rgs::views::split(' ');
    auto subrg_iter = subrg_view.begin();

    auto subrg2double = [](const auto& subrg) {
        const std::string s{subrg.begin(), subrg.end()};
        const double v = std::stod(s);
        return v;
    };

    cv::Mat micenters(rows, cols, CV_64FC2);
    for (const int row : rgs::views::iota(0, rows)) {
        auto prow = micenters.ptr<cv::Point2d>(row);

        for (const int col : rgs::views::iota(0, cols)) {
            const double x = subrg2double(*subrg_iter);
            subrg_iter++;
            const double y = subrg2double(*subrg_iter);
            subrg_iter++;
            prow[col] = {x, y};
        }
    }

    return {std::move(micenters), diameter, rotation};
}

inline CalibConfig CalibConfig::fromXMLPath(const char* path)
{
    pugi::xml_document doc;
    const auto ret = doc.load_file(path, pugi::parse_minimal, pugi::encoding_utf8);
    if (!ret) {
        return {};
    }
    return CalibConfig::fromXMLDoc(doc);
}

inline double CalibConfig::getDiameter() const noexcept { return diameter_; }

inline double CalibConfig::getRotation() const noexcept { return rotation_; }

} // namespace tlct::cfg::inline tspc
