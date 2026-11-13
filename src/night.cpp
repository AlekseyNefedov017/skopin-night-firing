#include "skopin/night.hpp"

#include "skopin/kiln.hpp"

#include <stdexcept>

namespace skopin {

NightReport runNightFire(const std::vector<Ware>&) {
    // 1. Слева направо вычеркнуть duplicate, затем invalid, затем thick.
    //    Вычеркнутые не стоят в очереди и не рвут садку.
    // 2. Пока кто-то ждёт, собрать одну садку по правилам README.
    //    На стопку сажать с конца к началу: первое взятое снимается первым.
    // 3. Провести её объектом Kiln. После утиля, за которым сразу полива,
    //    горн не остужать. stokes взять из kiln.stokes() в конце.
    throw std::logic_error("runNightFire: реализуйте");
}

}  // namespace skopin
