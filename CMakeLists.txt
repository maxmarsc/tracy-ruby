cmake_minimum_required(VERSION 3.28)
project(tracy-ruby LANGUAGES C CXX)

# ##############################################################################
# Project Setup
# ##############################################################################

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS ON) Disable C++20 module dependency scanning
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(GNUInstallDirs)

# ##############################################################################
# Tracy & tracy_ruby setup
# ##############################################################################
# Tracy declares its options with option(), and its own cmake_minimum_required()
# resets CMP0077 to OLD, which would discard the variables set below.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(TRACY_ENABLE ON)
set(TRACY_ON_DEMAND ON)
if(SWITCH OR NOT BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS OFF)
  set(TRACY_STATIC ON)
else()
  set(BUILD_SHARED_LIBS ON)
  set(TRACY_STATIC OFF)
endif()
set(TRACY_DELAYED_INIT ON)
set(TRACY_MANUAL_LIFETIME ON)

if(SWITCH)
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

# ##############################################################################
# Target setup
# ##############################################################################

if(SWITCH)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/switch/cmake)
  find_package(Libnx REQUIRED)
  include(SwitchTools)
  include(Ruby)
else()
  find_package(Ruby 3.0 REQUIRED) # Requires Ruby >= 3.0.x

  add_library(ruby_ext_api INTERFACE)
  target_include_directories(ruby_ext_api INTERFACE ${Ruby_INCLUDE_DIRS})
  if(WIN32)
    target_link_libraries(ruby_ext_api INTERFACE ${Ruby_LIBRARIES})
  elseif(APPLE)
    target_link_options(ruby_ext_api INTERFACE LINKER:-undefined,dynamic_lookup)
  endif()
  add_library(Ruby::ruby ALIAS ruby_ext_api)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/tracy)

if(BUILD_SHARED_LIBS)
  add_library(
    tracy_ruby MODULE ${CMAKE_CURRENT_SOURCE_DIR}/ext/tracy_ruby/tracy_ruby.cpp
  )

  execute_process(
    COMMAND ${Ruby_EXECUTABLE} -e "print RbConfig::CONFIG['DLEXT']"
    OUTPUT_VARIABLE RUBY_DLEXT COMMAND_ERROR_IS_FATAL ANY
  )

  if(APPLE)
    set(_origin "@loader_path")
  else()
    set(_origin "$ORIGIN")
  endif()

  set_target_properties(
    tracy_ruby
    PROPERTIES PREFIX ""
               SUFFIX ".${RUBY_DLEXT}"
               # find libTracyClient.so next to tracy_ruby.so
               BUILD_RPATH "${_origin}"
               INSTALL_RPATH "${_origin}"
  )
else()
  add_library(
    tracy_ruby ${CMAKE_CURRENT_SOURCE_DIR}/ext/tracy_ruby/tracy_ruby.cpp
  )
endif()

target_link_libraries(
  tracy_ruby
  PUBLIC Ruby::ruby TracyClient
)

target_compile_definitions(
  tracy_ruby
  PUBLIC TRACY_ENABLE
)

set_target_properties(
  tracy_ruby
  PROPERTIES CXX_STANDARD 20
             PUBLIC_HEADER
             ${CMAKE_CURRENT_SOURCE_DIR}/ext/tracy_ruby/tracy_ruby.h
)

# ##############################################################################
# Installation
# ##############################################################################
file(RELATIVE_PATH _rel ${CMAKE_INSTALL_FULL_BINDIR}
     ${CMAKE_INSTALL_FULL_LIBDIR}
)

install(
  TARGETS tracy_ruby
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
