# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dgalaktionov/Documents/trayectorias/libcds2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dgalaktionov/Documents/trayectorias/libcds2

# Include any dependencies generated for this target.
include CMakeFiles/BitStreamTests.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/BitStreamTests.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/BitStreamTests.dir/flags.make

CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o: CMakeFiles/BitStreamTests.dir/flags.make
CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o: tests/utils/BitStreamTests.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/dgalaktionov/Documents/trayectorias/libcds2/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o -c /home/dgalaktionov/Documents/trayectorias/libcds2/tests/utils/BitStreamTests.cpp

CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/dgalaktionov/Documents/trayectorias/libcds2/tests/utils/BitStreamTests.cpp > CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.i

CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/dgalaktionov/Documents/trayectorias/libcds2/tests/utils/BitStreamTests.cpp -o CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.s

CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o.requires:
.PHONY : CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o.requires

CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o.provides: CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o.requires
	$(MAKE) -f CMakeFiles/BitStreamTests.dir/build.make CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o.provides.build
.PHONY : CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o.provides

CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o.provides.build: CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o

# Object files for target BitStreamTests
BitStreamTests_OBJECTS = \
"CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o"

# External object files for target BitStreamTests
BitStreamTests_EXTERNAL_OBJECTS =

BitStreamTests: CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o
BitStreamTests: CMakeFiles/BitStreamTests.dir/build.make
BitStreamTests: libcds.a
BitStreamTests: CMakeFiles/BitStreamTests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable BitStreamTests"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/BitStreamTests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/BitStreamTests.dir/build: BitStreamTests
.PHONY : CMakeFiles/BitStreamTests.dir/build

CMakeFiles/BitStreamTests.dir/requires: CMakeFiles/BitStreamTests.dir/tests/utils/BitStreamTests.cpp.o.requires
.PHONY : CMakeFiles/BitStreamTests.dir/requires

CMakeFiles/BitStreamTests.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/BitStreamTests.dir/cmake_clean.cmake
.PHONY : CMakeFiles/BitStreamTests.dir/clean

CMakeFiles/BitStreamTests.dir/depend:
	cd /home/dgalaktionov/Documents/trayectorias/libcds2 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dgalaktionov/Documents/trayectorias/libcds2 /home/dgalaktionov/Documents/trayectorias/libcds2 /home/dgalaktionov/Documents/trayectorias/libcds2 /home/dgalaktionov/Documents/trayectorias/libcds2 /home/dgalaktionov/Documents/trayectorias/libcds2/CMakeFiles/BitStreamTests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/BitStreamTests.dir/depend

