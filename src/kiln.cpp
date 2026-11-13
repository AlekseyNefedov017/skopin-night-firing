#include "skopin/kiln.hpp"

#include <stdexcept>

namespace skopin {

Kiln::Kiln() {
    throw std::logic_error("Kiln: реализуйте");
}

Cone Kiln::cone() const {
    throw std::logic_error("Kiln::cone: реализуйте");
}

Door Kiln::door() const {
    throw std::logic_error("Kiln::door: реализуйте");
}

int Kiln::stokes() const {
    throw std::logic_error("Kiln::stokes: реализуйте");
}

std::vector<Ware> Kiln::stack() const {
    throw std::logic_error("Kiln::stack: реализуйте");
}

void Kiln::open() {
    // Повторное открытие и открытие на поливном конусе — KilnError.
    throw std::logic_error("Kiln::open: реализуйте");
}

void Kiln::close() {
    throw std::logic_error("Kiln::close: реализуйте");
}

void Kiln::fire() {
    // Отказ не тратит такт и не поднимает конус наполовину.
    throw std::logic_error("Kiln::fire: реализуйте");
}

void Kiln::cool() {
    // Ровно один конус вниз. Полива, не видевшая поливного огня, остыть не даёт.
    throw std::logic_error("Kiln::cool: реализуйте");
}

void Kiln::load(const Ware&) {
    // Внутри должна оказаться копия. Дверцу load сам не открывает.
    throw std::logic_error("Kiln::load: реализуйте");
}

void Kiln::unload(const std::string&) {
    // Снимается только верх стопки. Поливу с утильного конуса не снимают.
    throw std::logic_error("Kiln::unload: реализуйте");
}

}  // namespace skopin
