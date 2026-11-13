#include "skopin/ware.hpp"

#include <stdexcept>

namespace skopin {

std::optional<std::string> validateWare(const Ware&) {
    throw std::logic_error("validateWare: реализуйте");
}

bool fitsInKiln(const std::vector<Ware>&, const Ware&) {
    throw std::logic_error("fitsInKiln: реализуйте");
}

}  // namespace skopin
