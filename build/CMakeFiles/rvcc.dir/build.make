# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_SOURCE_DIR = /home/git_dir/riscv_compiler_self_make

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/git_dir/riscv_compiler_self_make/build

# Include any dependencies generated for this target.
include CMakeFiles/rvcc.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rvcc.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rvcc.dir/flags.make

CMakeFiles/rvcc.dir/main.c.o: CMakeFiles/rvcc.dir/flags.make
CMakeFiles/rvcc.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/git_dir/riscv_compiler_self_make/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/rvcc.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/rvcc.dir/main.c.o   -c /home/git_dir/riscv_compiler_self_make/main.c

CMakeFiles/rvcc.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/rvcc.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/git_dir/riscv_compiler_self_make/main.c > CMakeFiles/rvcc.dir/main.c.i

CMakeFiles/rvcc.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/rvcc.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/git_dir/riscv_compiler_self_make/main.c -o CMakeFiles/rvcc.dir/main.c.s

# Object files for target rvcc
rvcc_OBJECTS = \
"CMakeFiles/rvcc.dir/main.c.o"

# External object files for target rvcc
rvcc_EXTERNAL_OBJECTS =

rvcc: CMakeFiles/rvcc.dir/main.c.o
rvcc: CMakeFiles/rvcc.dir/build.make
rvcc: CMakeFiles/rvcc.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/git_dir/riscv_compiler_self_make/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable rvcc"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rvcc.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rvcc.dir/build: rvcc

.PHONY : CMakeFiles/rvcc.dir/build

CMakeFiles/rvcc.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rvcc.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rvcc.dir/clean

CMakeFiles/rvcc.dir/depend:
	cd /home/git_dir/riscv_compiler_self_make/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/git_dir/riscv_compiler_self_make /home/git_dir/riscv_compiler_self_make /home/git_dir/riscv_compiler_self_make/build /home/git_dir/riscv_compiler_self_make/build /home/git_dir/riscv_compiler_self_make/build/CMakeFiles/rvcc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rvcc.dir/depend

