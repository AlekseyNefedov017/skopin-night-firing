#pragma once

#include "skopin/measures.hpp"

#include <optional>
#include <string>
#include <vector>

namespace skopin {

enum class Program {
    Bisque,
    Glaze,
};

enum class WareKind {
    Regular,
    Rush,
    Sagger,
};

// Запись об изделии: открытые данные без правил горна.
// Не добавляйте конструкторы и закрытые поля — это агрегат,
// тесты собирают его через фигурные скобки.
struct Ware {
    std::string mark;
    int height_cm;
    int thickness_mm;
    Program program;
    WareKind kind;
};

// nullopt — запись можно рассматривать дальше.
// Иначе одно из сообщений: "empty mark", "bad mark",
// "bad height", "bad thickness". Порядок проверок — в README.
[[nodiscard]] std::optional<std::string> validateWare(const Ware& ware);

// Влезает ли next на уже стоящую стопку по высоте и полкам.
// Капсель, программа и толщина здесь ни при чём: это правило горна, не линейки.
// Индекс 0 в stack — низ стопки.
[[nodiscard]] bool fitsInKiln(const std::vector<Ware>& stack, const Ware& next);

}  // namespace skopin
