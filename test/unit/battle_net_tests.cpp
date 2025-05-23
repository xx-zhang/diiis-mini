#include <gtest/gtest.h>
#include "core/logger.h"
#include "utils/debug.h"

int main(int argc, char** argv) {
    // Initialize the logger for tests
    d3server::core::Logger::getInstance().init("battle_net_test.log", true);
    
    // Turn off debug output during tests by default
    d3server::utils::Debug::setEnabled(false);
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run all tests
    int result = RUN_ALL_TESTS();
    
    return result;
} 