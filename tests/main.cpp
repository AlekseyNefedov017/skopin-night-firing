#include "harness.hpp"

#include <vector>

void registerWareTests(std::vector<skopin_test::TestCase>& tests);
void registerKilnTests(std::vector<skopin_test::TestCase>& tests);
void registerNightTests(std::vector<skopin_test::TestCase>& tests);

int main() {
    std::vector<skopin_test::TestCase> tests;
    registerWareTests(tests);
    registerKilnTests(tests);
    registerNightTests(tests);
    return skopin_test::runAll(tests);
}
