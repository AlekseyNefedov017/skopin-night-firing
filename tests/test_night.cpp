#include "harness.hpp"
#include "make_ware.hpp"

#include "skopin/night.hpp"

#include <string>
#include <vector>

using skopin::NightReport;
using skopin::Program;
using skopin::Refusal;
using skopin::Ware;
using skopin::WareKind;
using skopin::runNightFire;

namespace {

void expectClosed(const std::vector<Ware>& queue, const NightReport& report) {
    EXPECT_EQ(static_cast<int>(report.passed.size() + report.refused.size()), static_cast<int>(queue.size()));
}

void expectRefused(const NightReport& report, const std::vector<Refusal>& expected) {
    EXPECT_EQ(static_cast<int>(report.refused.size()), static_cast<int>(expected.size()));
    const std::size_t count = report.refused.size() < expected.size() ? report.refused.size() : expected.size();
    for (std::size_t index = 0; index < count; ++index) {
        EXPECT_EQ(report.refused[index].mark, expected[index].mark);
        EXPECT_EQ(report.refused[index].reason, expected[index].reason);
    }
}

void emptyNightDoesNothing() {
    const NightReport report = runNightFire({});
    EXPECT(report.passed.empty());
    EXPECT(report.refused.empty());
    EXPECT_EQ(report.stokes, 0);
}

void singleBisqueCostsTwoStokes() {
    const std::vector<Ware> queue{makeWare("Bowl", 10, 8, Program::Bisque)};
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"Bowl"};
    EXPECT_EQ(report.passed, passed);
    EXPECT(report.refused.empty());
    EXPECT_EQ(report.stokes, 2);
    expectClosed(queue, report);
}

void singleGlazeCostsFourStokes() {
    const std::vector<Ware> queue{makeWare("Jug", 10, 8, Program::Glaze)};
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"Jug"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 4);
}

void twoThatFitShareOneFiring() {
    const std::vector<Ware> queue{
        makeWare("A", 23, 8, Program::Bisque),
        makeWare("B", 23, 8, Program::Bisque),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"A", "B"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 2);
}

void twoThatMissByTheShelfGoSeparately() {
    const std::vector<Ware> queue{
        makeWare("A", 24, 8, Program::Bisque),
        makeWare("B", 24, 8, Program::Bisque),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"A", "B"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 4);
}

void fiveShortWaresAreOneFiring() {
    std::vector<Ware> queue;
    for (int index = 0; index < 5; ++index) {
        queue.push_back(makeWare("S" + std::to_string(index + 1), 8, 8, Program::Bisque));
    }
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"S1", "S2", "S3", "S4", "S5"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 2);
}

void saggerDoesNotTakeANeighborEvenWhenCentimetersAllow() {
    const std::vector<Ware> queue{
        makeWare("O", 20, 8, Program::Bisque, WareKind::Sagger),
        makeWare("R", 10, 8, Program::Bisque),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"O", "R"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 4);
}

void bisqueThenGlazeSavesTheReheat() {
    const std::vector<Ware> queue{
        makeWare("A", 10, 8, Program::Bisque),
        makeWare("B", 10, 8, Program::Glaze),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"A", "B"};
    EXPECT_EQ(report.passed, passed);
    // Утиль оставляют на утильном конусе, поливу сажают горячей: 1 + 3, не 2 + 4.
    EXPECT_EQ(report.stokes, 4);
}

void readmeRushCrossesAPieceThatDoesNotFit() {
    // Разбор из README: А 30, Б 30, В скорая 8.
    const std::vector<Ware> queue{
        makeWare("А", 30, 8, Program::Bisque),
        makeWare("Б", 30, 8, Program::Bisque),
        makeWare("В", 8, 8, Program::Bisque, WareKind::Rush),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"А", "В", "Б"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 4);
}

void regularDoesNotJumpAndRushDoesNotClearTheWay() {
    const std::vector<Ware> queue{
        makeWare("А", 30, 8, Program::Bisque),
        makeWare("Б", 30, 8, Program::Bisque),
        makeWare("В", 6, 8, Program::Bisque, WareKind::Rush),
        makeWare("Г", 8, 8, Program::Bisque),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"А", "В", "Б", "Г"};
    EXPECT_EQ(report.passed, passed);
    // D к A и C по сантиметрам влезает (48), но флаг уже поднят.
    EXPECT_EQ(report.stokes, 4);
}

void rushDoesNotCrossAnotherProgram() {
    const std::vector<Ware> queue{
        makeWare("А", 20, 8, Program::Bisque),
        makeWare("Б", 20, 8, Program::Glaze),
        makeWare("В", 10, 8, Program::Bisque, WareKind::Rush),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"А", "Б", "В"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 6);
}

void saggerIsAWallEvenForRush() {
    const std::vector<Ware> queue{
        makeWare("A", 10, 8, Program::Bisque),
        makeWare("S", 10, 8, Program::Bisque, WareKind::Sagger),
        makeWare("C", 10, 8, Program::Bisque, WareKind::Rush),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"A", "S", "C"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 6);
}

void struckOutWaresDoNotSplitASet() {
    const std::vector<Ware> thick_between{
        makeWare("A", 10, 8, Program::Bisque),
        makeWare("B", 10, 30, Program::Glaze),
        makeWare("C", 10, 8, Program::Bisque),
    };
    const NightReport thick_report = runNightFire(thick_between);
    const std::vector<std::string> thick_passed{"A", "C"};
    EXPECT_EQ(thick_report.passed, thick_passed);
    EXPECT_EQ(thick_report.stokes, 2);
    expectRefused(thick_report, {Refusal{"B", "thick"}});
    expectClosed(thick_between, thick_report);

    const std::vector<Ware> invalid_between{
        makeWare("A", 10, 8, Program::Bisque),
        makeWare("bad mark", 10, 8, Program::Glaze),
        makeWare("C", 10, 12, Program::Bisque),
    };
    const NightReport invalid_report = runNightFire(invalid_between);
    const std::vector<std::string> invalid_passed{"A", "C"};
    EXPECT_EQ(invalid_report.passed, invalid_passed);
    EXPECT_EQ(invalid_report.stokes, 2);
    expectRefused(invalid_report, {Refusal{"bad mark", "invalid"}});

    const std::vector<Ware> duplicated{
        makeWare("A", 10, 8, Program::Bisque),
        makeWare("A", 10, 8, Program::Glaze),
        makeWare("C", 10, 8, Program::Bisque),
    };
    const NightReport duplicate_report = runNightFire(duplicated);
    const std::vector<std::string> duplicate_passed{"A", "C"};
    EXPECT_EQ(duplicate_report.passed, duplicate_passed);
    EXPECT_EQ(duplicate_report.stokes, 2);
    expectRefused(duplicate_report, {Refusal{"A", "duplicate"}});
    expectClosed(duplicated, duplicate_report);
}

void markIsConsumedOnFirstSight() {
    const std::vector<Ware> invalid_then_copy{
        makeWare("bad mark", 10, 8, Program::Bisque),
        makeWare("bad mark", 12, 8, Program::Bisque),
    };
    const NightReport invalid_report = runNightFire(invalid_then_copy);
    EXPECT(invalid_report.passed.empty());
    EXPECT_EQ(invalid_report.stokes, 0);
    expectRefused(
        invalid_report,
        {Refusal{"bad mark", "invalid"}, Refusal{"bad mark", "duplicate"}}
    );

    const std::vector<Ware> thick_then_thin{
        makeWare("X", 10, 40, Program::Bisque),
        makeWare("X", 10, 8, Program::Glaze),
    };
    const NightReport thick_report = runNightFire(thick_then_thin);
    EXPECT(thick_report.passed.empty());
    EXPECT_EQ(thick_report.stokes, 0);
    expectRefused(thick_report, {Refusal{"X", "thick"}, Refusal{"X", "duplicate"}});
    expectClosed(thick_then_thin, thick_report);
}

void thicknessBoundaryIsTheCrackLimit() {
    const std::vector<Ware> queue{
        makeWare("Fit", 10, 12, Program::Glaze),
        makeWare("Crack", 10, 13, Program::Glaze),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"Fit"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 4);
    expectRefused(report, {Refusal{"Crack", "thick"}});
}

void capstoneNight() {
    // Вычеркнутые не стоят в очереди. Скорая перелезает только через своё
    // изделие, которому не хватило высоты. Чужая программа и капсель — стена.
    const std::vector<Ware> queue{
        makeWare("R1", 10, 8, Program::Bisque),
        makeWare("G1", 12, 8, Program::Glaze),
        makeWare("RUSH1", 8, 8, Program::Bisque, WareKind::Rush),
        makeWare("R2", 10, 8, Program::Bisque),
        makeWare("THICK", 10, 40, Program::Bisque),
        makeWare("bad mark", 10, 8, Program::Bisque),
        makeWare("SAG", 20, 8, Program::Bisque, WareKind::Sagger),
        makeWare("RUSH2", 10, 8, Program::Bisque, WareKind::Rush),
        makeWare("R1", 10, 8, Program::Glaze),
    };
    const NightReport report = runNightFire(queue);
    const std::vector<std::string> passed{"R1", "G1", "RUSH1", "R2", "SAG", "RUSH2"};
    EXPECT_EQ(report.passed, passed);
    EXPECT_EQ(report.stokes, 10);
    expectRefused(
        report,
        {
            Refusal{"THICK", "thick"},
            Refusal{"bad mark", "invalid"},
            Refusal{"R1", "duplicate"},
        }
    );
    expectClosed(queue, report);
}

}  // namespace

void registerNightTests(std::vector<skopin_test::TestCase>& tests) {
    tests.push_back({"night / пустая ночь", emptyNightDoesNothing});
    tests.push_back({"night / один утиль — два такта", singleBisqueCostsTwoStokes});
    tests.push_back({"night / одна полива — четыре такта", singleGlazeCostsFourStokes});
    tests.push_back({"night / 23 и 23 делят один огонь", twoThatFitShareOneFiring});
    tests.push_back({"night / 24 и 24 не встают вместе", twoThatMissByTheShelfGoSeparately});
    tests.push_back({"night / пять коротких — один обжиг, не пять", fiveShortWaresAreOneFiring});
    tests.push_back({"night / капсель идёт один, даже если сантиметры есть", saggerDoesNotTakeANeighborEvenWhenCentimetersAllow});
    tests.push_back({"night / после утиля поливу сажают горячей", bisqueThenGlazeSavesTheReheat});
    tests.push_back({"night / разбор из README: скорая перелезает через тесное", readmeRushCrossesAPieceThatDoesNotFit});
    tests.push_back({"night / обычное не вклинивается, скорая не снимает флаг", regularDoesNotJumpAndRushDoesNotClearTheWay});
    tests.push_back({"night / скорая не перелезает через чужую программу", rushDoesNotCrossAnotherProgram});
    tests.push_back({"night / капсель — стена даже для скорой", saggerIsAWallEvenForRush});
    tests.push_back({"night / вычеркнутые не рвут садку", struckOutWaresDoNotSplitASet});
    tests.push_back({"night / клеймо занято с первого появления", markIsConsumedOnFirstSight});
    tests.push_back({"night / толщина 12 проходит, 13 уже нет", thicknessBoundaryIsTheCrackLimit});
    tests.push_back({"night / ночь целиком", capstoneNight});
}
