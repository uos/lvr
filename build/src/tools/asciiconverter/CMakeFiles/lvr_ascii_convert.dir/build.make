# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.4

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
CMAKE_COMMAND = /home/student/i/imitschke/programms/cmake-3.4.0-rc2/bin/cmake

# The command to remove a file.
RM = /home/student/i/imitschke/programms/cmake-3.4.0-rc2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/student/i/imitschke/meshing.largescale

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/student/i/imitschke/meshing.largescale/build

# Include any dependencies generated for this target.
include src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/depend.make

# Include the progress variables for this target.
include src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/progress.make

# Include the compile flags for this target's objects.
include src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/flags.make

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/flags.make
src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o: ../src/tools/asciiconverter/Options.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/student/i/imitschke/meshing.largescale/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o"
	cd /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter && /sw/sdev/gcc/x86_64/4.9.2/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o -c /home/student/i/imitschke/meshing.largescale/src/tools/asciiconverter/Options.cpp

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lvr_ascii_convert.dir/Options.cpp.i"
	cd /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter && /sw/sdev/gcc/x86_64/4.9.2/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/student/i/imitschke/meshing.largescale/src/tools/asciiconverter/Options.cpp > CMakeFiles/lvr_ascii_convert.dir/Options.cpp.i

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lvr_ascii_convert.dir/Options.cpp.s"
	cd /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter && /sw/sdev/gcc/x86_64/4.9.2/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/student/i/imitschke/meshing.largescale/src/tools/asciiconverter/Options.cpp -o CMakeFiles/lvr_ascii_convert.dir/Options.cpp.s

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o.requires:

.PHONY : src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o.requires

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o.provides: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o.requires
	$(MAKE) -f src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/build.make src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o.provides.build
.PHONY : src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o.provides

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o.provides.build: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o


src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/flags.make
src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o: ../src/tools/asciiconverter/Main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/student/i/imitschke/meshing.largescale/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o"
	cd /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter && /sw/sdev/gcc/x86_64/4.9.2/bin/g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o -c /home/student/i/imitschke/meshing.largescale/src/tools/asciiconverter/Main.cpp

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lvr_ascii_convert.dir/Main.cpp.i"
	cd /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter && /sw/sdev/gcc/x86_64/4.9.2/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/student/i/imitschke/meshing.largescale/src/tools/asciiconverter/Main.cpp > CMakeFiles/lvr_ascii_convert.dir/Main.cpp.i

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lvr_ascii_convert.dir/Main.cpp.s"
	cd /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter && /sw/sdev/gcc/x86_64/4.9.2/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/student/i/imitschke/meshing.largescale/src/tools/asciiconverter/Main.cpp -o CMakeFiles/lvr_ascii_convert.dir/Main.cpp.s

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o.requires:

.PHONY : src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o.requires

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o.provides: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o.requires
	$(MAKE) -f src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/build.make src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o.provides.build
.PHONY : src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o.provides

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o.provides.build: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o


# Object files for target lvr_ascii_convert
lvr_ascii_convert_OBJECTS = \
"CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o" \
"CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o"

# External object files for target lvr_ascii_convert
lvr_ascii_convert_EXTERNAL_OBJECTS =

bin/lvr_ascii_convert: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o
bin/lvr_ascii_convert: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o
bin/lvr_ascii_convert: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/build.make
bin/lvr_ascii_convert: lib/liblvr_static.a
bin/lvr_ascii_convert: lib/liblvrlas_static.a
bin/lvr_ascii_convert: lib/liblvrrply_static.a
bin/lvr_ascii_convert: lib/liblvrslam6d_static.a
bin/lvr_ascii_convert: /usr/lib64/libGLU.so
bin/lvr_ascii_convert: /usr/lib64/libGL.so
bin/lvr_ascii_convert: /usr/lib64/libglut.so
bin/lvr_ascii_convert: /usr/lib64/libXmu.so
bin/lvr_ascii_convert: /usr/lib64/libXi.so
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libboost_program_options.so
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libboost_system.so
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libboost_thread.so
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libboost_filesystem.so
bin/lvr_ascii_convert: /usr/lib64/libGLU.so
bin/lvr_ascii_convert: /usr/lib64/libGL.so
bin/lvr_ascii_convert: /usr/lib64/libglut.so
bin/lvr_ascii_convert: /usr/lib64/libXmu.so
bin/lvr_ascii_convert: /usr/lib64/libXi.so
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_videostab.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_superres.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_stitching.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_contrib.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_nonfree.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_ocl.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_gpu.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_photo.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_objdetect.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_legacy.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_video.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_ml.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_calib3d.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_features2d.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_highgui.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_imgproc.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_flann.so.2.4.11
bin/lvr_ascii_convert: /home/student/i/imitschke/local/lib/libopencv_core.so.2.4.11
bin/lvr_ascii_convert: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/student/i/imitschke/meshing.largescale/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../../bin/lvr_ascii_convert"
	cd /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lvr_ascii_convert.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/build: bin/lvr_ascii_convert

.PHONY : src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/build

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/requires: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Options.cpp.o.requires
src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/requires: src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/Main.cpp.o.requires

.PHONY : src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/requires

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/clean:
	cd /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter && $(CMAKE_COMMAND) -P CMakeFiles/lvr_ascii_convert.dir/cmake_clean.cmake
.PHONY : src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/clean

src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/depend:
	cd /home/student/i/imitschke/meshing.largescale/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/student/i/imitschke/meshing.largescale /home/student/i/imitschke/meshing.largescale/src/tools/asciiconverter /home/student/i/imitschke/meshing.largescale/build /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter /home/student/i/imitschke/meshing.largescale/build/src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/tools/asciiconverter/CMakeFiles/lvr_ascii_convert.dir/depend

