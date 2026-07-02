cmake_minimum_required(VERSION 3.28)
project(tracy-ruby LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable C++20 module dependency scanning
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)


set(TRACY_ENABLE ON)
set(TRACY_ON_DEMAND ON)
set(TRACY_STATIC ON)
set(TRACY_DELAYED_INIT ON)
set(TRACY_MANUAL_LIFETIME ON)

if (SWITCH)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/switch/cmake)
    
    find_package(Libnx REQUIRED)
    include(SwitchTools)
    include(Ruby)

    # network & discovery
    set(TRACY_ONLY_IPV4 ON)
    set(TRACY_NO_BROADCAST ON)
    # automatic sampling
    set(TRACY_NO_SAMPLING ON)
    set(TRACY_NO_CONTEXT_SWITCH ON)
    set(TRACY_NO_SYSTEM_TRACING ON)
    set(TRACY_NO_VSYNC_CAPTURE ON)
    set(TRACY_NO_VSYNC_CAPTURE ON)
    set(TRACY_NO_FRAME_IMAGE ON)
    # Misc
    set(TRACY_NO_CRASH_HANDLER ON)
    set(CMAKE_DL_LIBS "")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/tracy)

add_library(tracy_ruby STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/tracy_ruby/tracy_ruby.cpp
)

target_link_libraries(tracy_ruby
    PUBLIC Ruby::ruby TracyClient
)

set_target_properties(
    tracy_ruby
    PROPERTIES CXX_STANDARD 20
)