find_package(Corrade REQUIRED Utility)

corrade_add_test(coreLoggingTest LoggingTest.cpp LIBRARIES core Corrade::Utility)
