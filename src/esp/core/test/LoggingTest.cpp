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
    {"", "DebugOther WarningOther DebugSim WarningSim DebugGfx WarningGfx"},
    {"debug",
     "DebugOther WarningOther DebugSim WarningSim DebugGfx WarningGfx"},
    {"quiet", ""},
    {"error", ""},
    {"quiet;Sim,Gfx=verbose", "DebugSim WarningSim DebugGfx WarningGfx"},
    {"warning;Gfx=debug", "WarningOther WarningSim DebugGfx WarningGfx"},
};

LoggingTest::LoggingTest() {
  addInstancedTests({&LoggingTest::envVarTest},
                    Cr::Containers::arraySize(EnvVarTestData));
}

void LoggingTest::envVarTest() {
  auto&& data = EnvVarTestData[testCaseInstanceId()];

  LoggingSubsystemTracker::DeleteInstance();
  LoggingSubsystemTracker::Instance().processEnvString(data.envString);

  std::ostringstream out;
  Cr::Utility::Debug debugCapture{&out};
  Cr::Utility::Warning warnCapture{&out};

  ESP_DEBUG() << "DebugOther";
  ESP_WARNING() << "WarningOther";

  sim::test::debug("DebugSim");
  sim::test::warning("WarningSim");

  gfx::test::debug("DebugGfx");
  gfx::test::warning("WarningGfx");

  auto result = Cr::Containers::StringView{" "}.join(
      Cr::Containers::String{out.str()}.splitWithoutEmptyParts('\n'));

  CORRADE_COMPARE(result, data.expected);
  LoggingSubsystemTracker::DeleteInstance();
}

}  // namespace
}  // namespace test
}  // namespace logging
}  // namespace esp

CORRADE_TEST_MAIN(esp::logging::test::LoggingTest)