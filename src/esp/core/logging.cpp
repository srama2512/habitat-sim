// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "logging.h"

#include <algorithm>
#include <cstdlib>

#include <Corrade/Containers/StaticArray.h>
#include <Corrade/Containers/String.h>
#include <Corrade/Utility/String.h>

namespace Cr = Corrade;

namespace esp {
namespace logging {

Subsystem subsystemFromName(const Corrade::Containers::StringView name) {
  const Cr::Containers::String lowerCaseName =
      Cr::Utility::String::lowercase(name);
#define _c(SubsysName)                                                         \
  if (lowerCaseName ==                                                         \
      Cr::Utility::String::lowercase(Cr::Containers::StringView{#SubsysName})) \
  return Subsystem::SubsysName

  _c(Gfx);
  _c(Scene);
  _c(Sim);
  _c(Physics);
  _c(Other);

#undef _c

  CORRADE_ASSERT_UNREACHABLE("Unknown subsystem '"
                                 << Cr::Utility::Debug::nospace << name
                                 << Cr::Utility::Debug::nospace << "'",
                             {});
}

LoggingLevel levelFromName(const Corrade::Containers::StringView name) {
  const Cr::Containers::String lowerCaseName =
      Cr::Utility::String::lowercase(name);
#define _c(LevelName)                                                         \
  if (lowerCaseName ==                                                        \
      Cr::Utility::String::lowercase(Cr::Containers::StringView{#LevelName})) \
  return LoggingLevel::LevelName

  _c(verbose);
  _c(debug);
  _c(warning);
  _c(quiet);
  _c(error);

#undef _c
  CORRADE_ASSERT_UNREACHABLE("Unknown logging level name '"
                                 << Cr::Utility::Debug::nospace << name
                                 << Cr::Utility::Debug::nospace << "'",
                             {});
}

CORRADE_THREAD_LOCAL LoggingSubsystemTracker*
    LoggingSubsystemTracker::instance = nullptr;
LoggingSubsystemTracker& LoggingSubsystemTracker::Instance() {
  if (!instance)
    instance = new LoggingSubsystemTracker{};
  return *instance;
}

void LoggingSubsystemTracker::DeleteInstance() {
  delete instance;
  instance = nullptr;
}

LoggingSubsystemTracker::LoggingSubsystemTracker()
    : loggingLevels_{Cr::NoInit, uint8_t(Subsystem::NumSubsystems)} {
  std::fill(loggingLevels_.begin(), loggingLevels_.end(), DEFAULT_LEVEL);
  if (const char* envVar = std::getenv(LOGGING_ENV_VAR_NAME))
    processEnvString(Cr::Containers::StringView{envVar});
}

void LoggingSubsystemTracker::processEnvString(
    const Cr::Containers::StringView envString) {
  std::fill(loggingLevels_.begin(), loggingLevels_.end(), DEFAULT_LEVEL);

  for (const Cr::Containers::StringView setLevelCommand :
       envString.split(';')) {
    if (setLevelCommand.contains("=")) {
      const auto parts = setLevelCommand.partition('=');
      LoggingLevel lvl = levelFromName(parts[2]);

      for (const Cr::Containers::StringView subsystemName : parts[0].split(','))
        loggingLevels_[uint8_t(subsystemFromName(subsystemName))] = lvl;
    } else {
      std::fill(loggingLevels_.begin(), loggingLevels_.end(),
                levelFromName(setLevelCommand));
    }
  }
}

LoggingLevel LoggingSubsystemTracker::levelFor(Subsystem subsystem) const {
  return loggingLevels_[uint8_t(subsystem)];
}

namespace {
template <class Logger>
Logger outputForImpl(Subsystem subsystem, LoggingLevel level) {
  if (level >= LoggingSubsystemTracker::Instance().levelFor(subsystem))
    return Logger{Logger::output()};
  else
    return Logger{nullptr};
}
}  // namespace

Cr::Utility::Debug debugOutputFor(Subsystem subsystem) {
  return outputForImpl<Cr::Utility::Debug>(subsystem, LoggingLevel::debug);
}

Cr::Utility::Warning warningOutputFor(Subsystem subsystem) {
  return outputForImpl<Cr::Utility::Warning>(subsystem, LoggingLevel::warning);
}

Cr::Utility::Error errorOutputFor(Subsystem subsystem) {
  return outputForImpl<Cr::Utility::Error>(subsystem, LoggingLevel::error);
}

}  // namespace logging
}  // namespace esp