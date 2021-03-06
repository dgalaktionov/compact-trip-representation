### cmake file for building libsais Package ###
cmake_minimum_required(VERSION 2.4.2)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(AppendCompilerFlags)

## Project information ##
project(sais C)
set(PROJECT_VENDOR "Yuta Mori")
set(PROJECT_CONTACT "yuta.256@gmail.com")
set(PROJECT_URL "http://sites.google.com/site/yuta256/sais/")
set(PROJECT_DESCRIPTION "Induced-Sorting based linear suffix sorting library")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION_FULL)
string(REGEX REPLACE "[\n\r]" "" PROJECT_VERSION_FULL "${PROJECT_VERSION_FULL}")
string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+$" "\\1" PROJECT_VERSION_MAJOR "${PROJECT_VERSION_FULL}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]+$" "\\1" PROJECT_VERSION_MINOR "${PROJECT_VERSION_FULL}")
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)$" "\\1" PROJECT_VERSION_PATCH "${PROJECT_VERSION_FULL}")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
math(EXPR LIBRARY_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(LIBRARY_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(LIBRARY_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(LIBRARY_VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}")
set(LIBRARY_VERSION_FULL "${LIBRARY_VERSION}.${LIBRARY_VERSION_PATCH}")

## CPack configuration ##
set(CPACK_GENERATOR "TGZ;TBZ2;ZIP")
set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2;ZIP")
include(ProjectCPack)

## Project options ##
option(BUILD_SHARED_LIBS "Set to OFF to build static libraries" OFF)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_SAIS64 "Build libsais64" OFF)
option(WITH_LFS "Enable Large File Support" ON)

## Build type ##
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif(NOT CMAKE_BUILD_TYPE)

## Compiler options ##
if(MSVC)
  append_c_compiler_flags("/W4" "VC" CMAKE_C_FLAGS)
  append_c_compiler_flags("/Oi;/Ot;/Ox;/Oy" "VC" CMAKE_C_FLAGS_RELEASE)
  if(USE_OPENMP)
    append_c_compiler_flags("/openmp" "VC" CMAKE_C_FLAGS)
  endif(USE_OPENMP)
elseif(BORLAND)
  append_c_compiler_flags("-w;-w-8071;-w-8072" "BCC" CMAKE_C_FLAGS)
  append_c_compiler_flags("-Oi;-Og;-Os;-Ov;-Ox" "BCC" CMAKE_C_FLAGS_RELEASE)
else(MSVC)
  if(CMAKE_COMPILER_IS_GNUCC)
    append_c_compiler_flags("-Wall" "GCC" CMAKE_C_FLAGS)
    append_c_compiler_flags("-fomit-frame-pointer" "GCC" CMAKE_C_FLAGS_RELEASE)
    append_c_compiler_flags("-fPIC" "GCC" CMAKE_C_FLAGS_RELEASE)
    if(USE_OPENMP)
      append_c_compiler_flags("-fopenmp" "GCC" CMAKE_C_FLAGS)
    endif(USE_OPENMP)
  else(CMAKE_COMPILER_IS_GNUCC)
    append_c_compiler_flags("-Wall" "UNKNOWN" CMAKE_C_FLAGS)
    append_c_compiler_flags("-fomit-frame-pointer" "UNKNOWN" CMAKE_C_FLAGS_RELEASE)
    if(USE_OPENMP)
      append_c_compiler_flags("-fopenmp;-openmp;-omp" "UNKNOWN" CMAKE_C_FLAGS)
    endif(USE_OPENMP)
  endif(CMAKE_COMPILER_IS_GNUCC)
endif(MSVC)

## Add definitions ##
add_definitions(-DHAVE_CONFIG_H=1 -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS)

## Add subdirectories ##
add_subdirectory(pkgconfig)
add_subdirectory(include)
add_subdirectory(lib)
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(BUILD_EXAMPLES)

## Add 'uninstall' target ##
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CMakeModules/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/CMakeModules/cmake_uninstall.cmake")
