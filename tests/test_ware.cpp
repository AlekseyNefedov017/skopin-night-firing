#include "harness.hpp"
#include "make_ware.hpp"

#include "skopin/ware.hpp"

#include <optional>
#include <string>
#include <vector>

using skopin::Program;
using skopin::Ware;
using skopin::WareKind;
using skopin::fitsInKiln;
using skopin::validateWare;

namespace {

void expectError(const Ware& ware, const char* reason) {
    const std::optional<std::string> error = validateWare(ware);
    EXPECT(error.has_value());
    EXPECT_EQ(*error, reason);
}

void emptyMarkWinsOverOtherGarbage() {
    expectError(makeWare("", -5, -5, Program::Bisque), "empty mark");
}

void whitespaceMarkIsRejectedBeforeHeight() {
    expectError(makeWare("A B", 0, 0, Program::Glaze), "bad mark");
    expectError(makeWare("A\tB", 10, 8, Program::Bisque), "bad mark");
    expectError(makeWare("A\nB", 10, 8, Program::Bisque), "bad mark");
}

void badHeightAndBadThicknessHavePriority() {
    expectError(makeWare("AB", 0, -5, Program::Bisque), "bad height");
    expectError(makeWare("AB", -1, 8, Program::Bisque), "bad height");
    expectError(makeWare("AB", 49, 8, Program::Bisque), "bad height");
    expectError(makeWare("AB", 10, 0, Program::Bisque), "bad thickness");
}

void ordinaryMarksAreValid() {
    const Ware bowl = makeWare("Bowl", 48, 12, Program::Bisque);
    EXPECT(!validateWare(bowl).has_value());
    const Ware local = makeWare("Скопин-1", 1, 1, Program::Glaze, WareKind::Rush);
    EXPECT(!validateWare(local).has_value());
    const Ware underscored = makeWare("A_1", 23, 40, Program::Bisque, WareKind::Sagger);
    EXPECT(!validateWare(underscored).has_value());
}

void singleWareUsesNoShelf() {
    EXPECT(fitsInKiln({}, makeWare("A", 48, 8, Program::Bisque)));
    EXPECT(!fitsInKiln({}, makeWare("A", 49, 8, Program::Bisque)));
}

void oneShelfBetweenNeighborsAndNotUnderTheFloor() {
    const std::vector<Ware> twenty_three{makeWare("A", 23, 8, Program::Bisque)};
    EXPECT(fitsInKiln(twenty_three, makeWare("B", 23, 8, Program::Bisque)));
    EXPECT(!fitsInKiln(twenty_three, makeWare("B", 24, 8, Program::Bisque)));
}

void threeWaresUseTwoShelves() {
    const std::vector<Ware> inside{
        makeWare("A", 10, 8, Program::Bisque),
        makeWare("B", 10, 8, Program::Bisque),
    };
    EXPECT(fitsInKiln(inside, makeWare("C", 24, 8, Program::Bisque)));
    EXPECT(!fitsInKiln(inside, makeWare("C", 25, 8, Program::Bisque)));
}

void centimetersIgnoreKindAndProgram() {
    const std::vector<Ware> sagger{
        makeWare("O", 20, 8, Program::Bisque, WareKind::Sagger),
    };
    // 20 + 2 + 10 = 32. Запрет «капсель один» — правило горна, не линейки.
    EXPECT(fitsInKiln(sagger, makeWare("S", 10, 8, Program::Glaze, WareKind::Rush)));
}

}  // namespace

void registerWareTests(std::vector<skopin_test::TestCase>& tests) {
    tests.push_back({"ware / пустое клеймо важнее остальных ошибок", emptyMarkWinsOverOtherGarbage});
    tests.push_back({"ware / пробел в клейме важнее высоты", whitespaceMarkIsRejectedBeforeHeight});
    tests.push_back({"ware / высота проверяется раньше толщины", badHeightAndBadThicknessHavePriority});
    tests.push_back({"ware / обычное, русское и составное клейма годны", ordinaryMarksAreValid});
    tests.push_back({"ware / одно изделие не платит полку у пода", singleWareUsesNoShelf});
    tests.push_back({"ware / между двумя изделиями ровно одна полка", oneShelfBetweenNeighborsAndNotUnderTheFloor});
    tests.push_back({"ware / три изделия — две полки, граница ровно 48", threeWaresUseTwoShelves});
    tests.push_back({"ware / линейка не знает про капсель и программу", centimetersIgnoreKindAndProgram});
}
