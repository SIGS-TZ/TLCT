#pragma once

#include <algorithm>
#include <cstddef>

namespace tlct::_hp {

template <size_t size>
class cestring
{
public:
    consteval inline cestring(const char (&str)[size]) noexcept { std::copy(str, str + size, string); }

    char string[size]{};
};

} // namespace tlct::_hp
