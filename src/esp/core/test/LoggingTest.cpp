// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "esp/core/logging.h"

#include <sstream>

#include <Corrade/Containers/StaticArray.h>
#include <Corrade/Containers/String.h>
#include <Corrade/Containers/StringStl.h>
#include <Corrade/Containers/StringView.h>
#include <Corrade/TestSuite/Tester.h>

namespace Cr = Corrade;

namespace esp {

namespace sim {
namespace test {
namespace {
void debug(const Cr::Containers::StringView statement) {
  ESP_DEBUG() << statement;
}
void warning(const Cr::Containers::StringView statement) {
  ESP_WARNING() << statement;
}
}  // namespace
}  // namespace test
}  // namespace sim

namespace gfx {
namespace test {
namespace {
void debug(const Cr::Containers::StringView statement) {
  ESP_DEBUG() << statement;
}
void warning(const Cr::Containers::StringView statement) {
  ESP_WARNING() << statement;
}
}  // namespace
}  // namespace test
}  // namespace gfx

namespace logging {
namespace test {
namespace {

struct LoggingTest : Cr::TestSuite::Tester {
  explicit LoggingTest();

  void envVarTest();
};

constexpr const struct {
  const char* envString;
  const char* expected;
} EnvVarTestData[]{
    {nullptr,
     "[Subsystem: Default] DebugDefault\n[Subsystem: Default] WarningDefault\n"
     "[Subsystem: Sim] DebugSim\n[Subsystem: Sim] WarningSim\n[Subsystem: "
     "Gfx] "
     "DebugGfx\n[Subsystem: Gfx] WarningGfx\n"},
    {"debug",
     "[Subsystem: Default] DebugDefault\n[Subsystem: Default] WarningDefault\n"
     "[Subsystem: Sim] DebugSim\n[Subsystem: Sim] WarningSim\n[Subsystem: "
     "Gfx] "
     "DebugGfx\n[Subsystem: Gfx] WarningGfx\n"},
    {"quiet", ""},
    {"error", ""},
    {"quiet:Sim,Gfx=verbose",
     "[Subsystem: Sim] DebugSim\n[Subsystem: Sim] WarningSim\n[Subsystem: Gfx] "
     "DebugGfx\n[Subsystem: Gfx] WarningGfx\n"},
    {"warning:Gfx=debug",
     "[Subsystem: Default] WarningDefault\n"
     "[Subsystem: Sim] WarningSim\n[Subsystem: Gfx] "
     "DebugGfx\n[Subsystem: Gfx] WarningGfx\n"},
};  // namespace

LoggingTest::LoggingTest() {
  addInstancedTests({&LoggingTest::envVarTest},
                    Cr::Containers::arraySize(EnvVarTestData));
}

void LoggingTest::envVarTest() {
  auto&& data = EnvVarTestData[testCaseInstanceId()];

  LoggingContext ctx{data.envString};

  std::ostringstream out;
  Cr::Utility::Debug debugCapture{&out};
  Cr::Utility::Warning warnCapture{&out};

  ESP_DEBUG() << "DebugDefault";
  ESP_WARNING() << "WarningDefault";

  sim::test::debug("DebugSim");
  sim::test::warning("WarningSim");

  gfx::test::debug("DebugGfx");
  gfx::test::warning("WarningGfx");

  CORRADE_COMPARE(Cr::Containers::StringView{out.str()},
                  Cr::Containers::StringView{data.expected});
}

}  // namespace
}  // namespace test
}  // namespace logging
}  // namespace esp

CORRADE_TEST_MAIN(esp::logging::test::LoggingTest)
