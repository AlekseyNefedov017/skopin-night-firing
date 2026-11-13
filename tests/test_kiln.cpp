#include "harness.hpp"
#include "make_ware.hpp"

#include "skopin/kiln.hpp"

#include <string>
#include <vector>

using skopin::Cone;
using skopin::Door;
using skopin::Kiln;
using skopin::KilnError;
using skopin::Program;
using skopin::Ware;
using skopin::WareKind;

namespace {

struct Snapshot {
    Cone cone;
    Door door;
    int stokes;
    std::vector<std::string> marks;
    std::vector<int> heights;
};

Snapshot take(const Kiln& kiln) {
    Snapshot snapshot;
    snapshot.cone = kiln.cone();
    snapshot.door = kiln.door();
    snapshot.stokes = kiln.stokes();
    for (const Ware& ware : kiln.stack()) {
        snapshot.marks.push_back(ware.mark);
        snapshot.heights.push_back(ware.height_cm);
    }
    return snapshot;
}

void expectSame(const Kiln& kiln, const Snapshot& before) {
    EXPECT(kiln.cone() == before.cone);
    EXPECT(kiln.door() == before.door);
    EXPECT_EQ(kiln.stokes(), before.stokes);
    const std::vector<Ware> stack = kiln.stack();
    EXPECT_EQ(static_cast<int>(stack.size()), static_cast<int>(before.marks.size()));
    for (std::size_t index = 0; index < stack.size() && index < before.marks.size(); ++index) {
        EXPECT_EQ(stack[index].mark, before.marks[index]);
        EXPECT_EQ(stack[index].height_cm, before.heights[index]);
    }
}

void initialStateIsObservedThroughConstKiln() {
    const Kiln kiln;
    EXPECT(kiln.cone() == Cone::Cold);
    EXPECT(kiln.door() == Door::Closed);
    EXPECT_EQ(kiln.stokes(), 0);
    EXPECT(kiln.stack().empty());
}

void oneBisqueCanBeWorkedByHand() {
    Kiln kiln;
    kiln.open();
    kiln.load(makeWare("Bowl", 12, 8, Program::Bisque));
    kiln.close();
    kiln.fire();
    kiln.open();
    kiln.unload("Bowl");
    kiln.close();
    kiln.cool();

    EXPECT(kiln.cone() == Cone::Cold);
    EXPECT(kiln.door() == Door::Closed);
    EXPECT(kiln.stack().empty());
    EXPECT_EQ(kiln.stokes(), 2);
}

void oneGlazeFromColdCostsFourStokes() {
    Kiln kiln;
    kiln.open();
    kiln.load(makeWare("Jug", 12, 8, Program::Glaze));
    kiln.close();
    kiln.fire();
    kiln.fire();
    kiln.cool();
    kiln.cool();
    kiln.open();
    kiln.unload("Jug");
    kiln.close();

    EXPECT(kiln.cone() == Cone::Cold);
    EXPECT(kiln.door() == Door::Closed);
    EXPECT(kiln.stack().empty());
    EXPECT_EQ(kiln.stokes(), 4);
}

void doorIsNotIdempotentAndGlazeStaysShut() {
    Kiln kiln;
    kiln.open();
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.open(), KilnError);
        expectSame(kiln, before);
    }
    kiln.close();
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.close(), KilnError);
        expectSame(kiln, before);
    }

    kiln.fire();
    kiln.fire();
    EXPECT(kiln.cone() == Cone::Glaze);
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.open(), KilnError);
        expectSame(kiln, before);
    }
}

void openDoorDoesNotSpendAStoke() {
    Kiln kiln;
    kiln.open();
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.fire(), KilnError);
        EXPECT_THROW(kiln.cool(), KilnError);
        expectSame(kiln, before);
    }
    EXPECT_EQ(kiln.stokes(), 0);
}

void conesMoveOneStep() {
    Kiln kiln;
    kiln.fire();
    EXPECT(kiln.cone() == Cone::Bisque);
    EXPECT_EQ(kiln.stokes(), 1);
    kiln.fire();
    EXPECT(kiln.cone() == Cone::Glaze);
    EXPECT_EQ(kiln.stokes(), 2);
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.fire(), KilnError);
        expectSame(kiln, before);
    }
    kiln.cool();
    EXPECT(kiln.cone() == Cone::Bisque);
    EXPECT_EQ(kiln.stokes(), 3);
    kiln.cool();
    EXPECT(kiln.cone() == Cone::Cold);
    EXPECT_EQ(kiln.stokes(), 4);
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.cool(), KilnError);
        expectSame(kiln, before);
    }
}

void kilnKeepsItsOwnCopy() {
    Kiln kiln;
    Ware ware = makeWare("A", 10, 8, Program::Bisque);
    kiln.open();
    kiln.load(ware);
    ware.height_cm = 99;
    ware.mark = "mutated";
    ware.thickness_mm = 1;

    EXPECT_EQ(kiln.stack().at(0).mark, std::string("A"));
    EXPECT_EQ(kiln.stack().at(0).height_cm, 10);
    EXPECT_EQ(kiln.stack().at(0).thickness_mm, 8);
}

void rejectedLoadDoesNotKeepTheWare() {
    Kiln kiln;
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.load(makeWare("A", 10, 8, Program::Bisque)), KilnError);
        expectSame(kiln, before);
    }

    kiln.open();
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.load(makeWare(" ", 10, 8, Program::Bisque)), KilnError);
        expectSame(kiln, before);
    }

    kiln.load(makeWare("A", 10, 8, Program::Bisque));
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.load(makeWare("A", 10, 8, Program::Glaze)), KilnError);
        expectSame(kiln, before);
    }
}

void aWareThatDoesNotFitLeavesTheFirstOneInside() {
    Kiln kiln;
    kiln.open();
    kiln.load(makeWare("A", 30, 8, Program::Bisque));
    const Snapshot before = take(kiln);
    EXPECT_THROW(kiln.load(makeWare("B", 20, 8, Program::Bisque)), KilnError);
    expectSame(kiln, before);
    EXPECT(kiln.door() == Door::Open);
}

void saggerOccupiesTheKilnAlone() {
    Kiln kiln;
    kiln.open();
    kiln.load(makeWare("O", 10, 8, Program::Bisque, WareKind::Sagger));
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.load(makeWare("R", 10, 8, Program::Bisque)), KilnError);
        expectSame(kiln, before);
    }

    Kiln other;
    other.open();
    other.load(makeWare("R", 10, 8, Program::Bisque));
    const Snapshot before = take(other);
    EXPECT_THROW(other.load(makeWare("O", 10, 8, Program::Bisque, WareKind::Sagger)), KilnError);
    expectSame(other, before);
}

void twoThatFitStayBottomToTop() {
    Kiln kiln;
    kiln.open();
    kiln.load(makeWare("A", 23, 8, Program::Bisque));
    kiln.load(makeWare("B", 23, 8, Program::Bisque));
    EXPECT_EQ(kiln.stack().at(0).mark, std::string("A"));
    EXPECT_EQ(kiln.stack().at(1).mark, std::string("B"));
    EXPECT_THROW(kiln.load(makeWare("C", 1, 8, Program::Bisque)), KilnError);
}

void unloadIsLifoAndNeedsTheDoor() {
    Kiln kiln;
    kiln.open();
    kiln.load(makeWare("A", 10, 8, Program::Bisque));
    kiln.load(makeWare("B", 10, 8, Program::Bisque));
    kiln.close();

    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.unload("B"), KilnError);
        expectSame(kiln, before);
    }

    kiln.open();
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.unload("A"), KilnError);
        expectSame(kiln, before);
    }
    kiln.unload("B");
    EXPECT_EQ(kiln.stack().at(0).mark, std::string("A"));
    kiln.unload("A");
    EXPECT(kiln.stack().empty());
}

void mixedProgramSeizesTheKiln() {
    Kiln kiln;
    kiln.open();
    kiln.load(makeWare("A", 10, 8, Program::Bisque));
    kiln.load(makeWare("B", 10, 8, Program::Glaze));
    kiln.close();
    kiln.fire();
    EXPECT(kiln.cone() == Cone::Bisque);
    EXPECT_EQ(kiln.stokes(), 1);

    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.cool(), KilnError);
        EXPECT_THROW(kiln.fire(), KilnError);
        expectSame(kiln, before);
    }

    kiln.open();
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.unload("B"), KilnError);
        expectSame(kiln, before);
    }
    EXPECT_THROW(kiln.unload("A"), KilnError);
}

void thicknessBoundaryOnFireIsAtomic() {
    Kiln kiln;
    kiln.open();
    kiln.load(makeWare("Ok", 10, 12, Program::Bisque));
    kiln.close();
    kiln.fire();
    EXPECT(kiln.cone() == Cone::Bisque);
    EXPECT_EQ(kiln.stokes(), 1);

    Kiln thick;
    thick.open();
    thick.load(makeWare("Crack", 10, 13, Program::Glaze));
    thick.close();
    const Snapshot before = take(thick);
    EXPECT_THROW(thick.fire(), KilnError);
    expectSame(thick, before);
    thick.open();
    thick.unload("Crack");
    EXPECT(thick.stack().empty());
    EXPECT_EQ(thick.stokes(), 0);
}

void hotLoadAndUnseenGlaze() {
    Kiln kiln;
    kiln.fire();
    kiln.open();
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.load(makeWare("ColdClay", 10, 8, Program::Bisque)), KilnError);
        expectSame(kiln, before);
    }
    kiln.load(makeWare("Jug", 10, 8, Program::Glaze));
    kiln.close();
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.cool(), KilnError);
        expectSame(kiln, before);
    }
    kiln.fire();
    EXPECT(kiln.cone() == Cone::Glaze);
    EXPECT_EQ(kiln.stokes(), 2);
    kiln.cool();
    EXPECT(kiln.cone() == Cone::Bisque);
    kiln.open();
    {
        const Snapshot before = take(kiln);
        EXPECT_THROW(kiln.unload("Jug"), KilnError);
        expectSame(kiln, before);
    }
    kiln.close();
    kiln.cool();
    EXPECT(kiln.cone() == Cone::Cold);
    kiln.open();
    kiln.unload("Jug");
    EXPECT(kiln.stack().empty());
    EXPECT_EQ(kiln.stokes(), 4);
}

}  // namespace

void registerKilnTests(std::vector<skopin_test::TestCase>& tests) {
    tests.push_back({"kiln / пустой горн холодный и закрытый", initialStateIsObservedThroughConstKiln});
    tests.push_back({"kiln / один утиль проходит руками", oneBisqueCanBeWorkedByHand});
    tests.push_back({"kiln / одна полива с холода — четыре такта", oneGlazeFromColdCostsFourStokes});
    tests.push_back({"kiln / дверца не прощает повтор, на поливе её не открыть", doorIsNotIdempotentAndGlazeStaysShut});
    tests.push_back({"kiln / открытая дверца не даёт сжечь такт", openDoorDoesNotSpendAStoke});
    tests.push_back({"kiln / конус шагает по одному и пустой горн тоже", conesMoveOneStep});
    tests.push_back({"kiln / внутрь попадает копия, исходник можно портить", kilnKeepsItsOwnCopy});
    tests.push_back({"kiln / отказ на загрузке не оставляет след", rejectedLoadDoesNotKeepTheWare});
    tests.push_back({"kiln / второе изделие не вытесняет первое", aWareThatDoesNotFitLeavesTheFirstOneInside});
    tests.push_back({"kiln / капсель ни с кем не делит горн", saggerOccupiesTheKilnAlone});
    tests.push_back({"kiln / 23 и 23 встают снизу вверх, лишний сантиметр — нет", twoThatFitStayBottomToTop});
    tests.push_back({"kiln / снять можно только верх и только при открытой дверце", unloadIsLifoAndNeedsTheDoor});
    tests.push_back({"kiln / утиль вместе с поливой заклинивает горн", mixedProgramSeizesTheKiln});
    tests.push_back({"kiln / толщина 12 ещё горит, 13 — нет, и такт не тратится", thicknessBoundaryOnFireIsAtomic});
    tests.push_back({"kiln / горячая посадка поливы: рано не остудить и с утильного не снять", hotLoadAndUnseenGlaze});
}
