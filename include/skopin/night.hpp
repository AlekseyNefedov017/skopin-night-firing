#pragma once

#include "skopin/ware.hpp"

#include <string>
#include <vector>

namespace skopin {

struct Refusal {
    std::string mark;
    std::string reason;
};

struct NightReport {
    std::vector<std::string> passed;
    std::vector<Refusal> refused;
    int stokes = 0;
};

// Обжигает очередь за ночь.
// reason у отказа: "invalid", "duplicate" или "thick".
// stokes — сколько раз успешно сработали fire и cool реального горна.
// На корректной по правилам README очереди функция не бросает исключений:
// сомнительное изделие попадает в refused до load.
[[nodiscard]] NightReport runNightFire(const std::vector<Ware>& queue);

}  // namespace skopin
