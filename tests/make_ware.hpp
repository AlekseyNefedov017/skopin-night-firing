#pragma once

#include "skopin/ware.hpp"

#include <utility>

inline skopin::Ware makeWare(
    std::string mark,
    int height_cm,
    int thickness_mm,
    skopin::Program program,
    skopin::WareKind kind = skopin::WareKind::Regular
) {
    return skopin::Ware{std::move(mark), height_cm, thickness_mm, program, kind};
}
