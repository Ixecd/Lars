# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/qc/Lars/lars_loadbalance_agent

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/qc/Lars/lars_loadbalance_agent/build

# Include any dependencies generated for this target.
include CMakeFiles/lars_loadbalance_agent.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/lars_loadbalance_agent.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/lars_loadbalance_agent.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lars_loadbalance_agent.dir/flags.make

CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.o: CMakeFiles/lars_loadbalance_agent.dir/flags.make
CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.o: /home/qc/Lars/lars_loadbalance_agent/src/agent_udp_server.cc
CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.o: CMakeFiles/lars_loadbalance_agent.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/qc/Lars/lars_loadbalance_agent/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.o -MF CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.o.d -o CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.o -c /home/qc/Lars/lars_loadbalance_agent/src/agent_udp_server.cc

CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qc/Lars/lars_loadbalance_agent/src/agent_udp_server.cc > CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.i

CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qc/Lars/lars_loadbalance_agent/src/agent_udp_server.cc -o CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.s

CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.o: CMakeFiles/lars_loadbalance_agent.dir/flags.make
CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.o: /home/qc/Lars/lars_loadbalance_agent/src/dns_client.cc
CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.o: CMakeFiles/lars_loadbalance_agent.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/qc/Lars/lars_loadbalance_agent/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.o -MF CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.o.d -o CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.o -c /home/qc/Lars/lars_loadbalance_agent/src/dns_client.cc

CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qc/Lars/lars_loadbalance_agent/src/dns_client.cc > CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.i

CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qc/Lars/lars_loadbalance_agent/src/dns_client.cc -o CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.s

CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.o: CMakeFiles/lars_loadbalance_agent.dir/flags.make
CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.o: /home/qc/Lars/lars_loadbalance_agent/src/host_info.cc
CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.o: CMakeFiles/lars_loadbalance_agent.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/qc/Lars/lars_loadbalance_agent/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.o -MF CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.o.d -o CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.o -c /home/qc/Lars/lars_loadbalance_agent/src/host_info.cc

CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qc/Lars/lars_loadbalance_agent/src/host_info.cc > CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.i

CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qc/Lars/lars_loadbalance_agent/src/host_info.cc -o CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.s

CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.o: CMakeFiles/lars_loadbalance_agent.dir/flags.make
CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.o: /home/qc/Lars/lars_loadbalance_agent/src/load_balance.cc
CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.o: CMakeFiles/lars_loadbalance_agent.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/qc/Lars/lars_loadbalance_agent/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.o -MF CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.o.d -o CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.o -c /home/qc/Lars/lars_loadbalance_agent/src/load_balance.cc

CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qc/Lars/lars_loadbalance_agent/src/load_balance.cc > CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.i

CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qc/Lars/lars_loadbalance_agent/src/load_balance.cc -o CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.s

CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.o: CMakeFiles/lars_loadbalance_agent.dir/flags.make
CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.o: /home/qc/Lars/lars_loadbalance_agent/src/main_server.cc
CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.o: CMakeFiles/lars_loadbalance_agent.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/qc/Lars/lars_loadbalance_agent/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.o -MF CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.o.d -o CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.o -c /home/qc/Lars/lars_loadbalance_agent/src/main_server.cc

CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qc/Lars/lars_loadbalance_agent/src/main_server.cc > CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.i

CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qc/Lars/lars_loadbalance_agent/src/main_server.cc -o CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.s

CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.o: CMakeFiles/lars_loadbalance_agent.dir/flags.make
CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.o: /home/qc/Lars/lars_loadbalance_agent/src/reporter_client.cc
CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.o: CMakeFiles/lars_loadbalance_agent.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/qc/Lars/lars_loadbalance_agent/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.o -MF CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.o.d -o CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.o -c /home/qc/Lars/lars_loadbalance_agent/src/reporter_client.cc

CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qc/Lars/lars_loadbalance_agent/src/reporter_client.cc > CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.i

CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qc/Lars/lars_loadbalance_agent/src/reporter_client.cc -o CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.s

CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.o: CMakeFiles/lars_loadbalance_agent.dir/flags.make
CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.o: /home/qc/Lars/lars_loadbalance_agent/src/route_lb.cc
CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.o: CMakeFiles/lars_loadbalance_agent.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/qc/Lars/lars_loadbalance_agent/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.o -MF CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.o.d -o CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.o -c /home/qc/Lars/lars_loadbalance_agent/src/route_lb.cc

CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qc/Lars/lars_loadbalance_agent/src/route_lb.cc > CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.i

CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qc/Lars/lars_loadbalance_agent/src/route_lb.cc -o CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.s

# Object files for target lars_loadbalance_agent
lars_loadbalance_agent_OBJECTS = \
"CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.o" \
"CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.o" \
"CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.o" \
"CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.o" \
"CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.o" \
"CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.o" \
"CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.o"

# External object files for target lars_loadbalance_agent
lars_loadbalance_agent_EXTERNAL_OBJECTS =

/home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent: CMakeFiles/lars_loadbalance_agent.dir/src/agent_udp_server.cc.o
/home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent: CMakeFiles/lars_loadbalance_agent.dir/src/dns_client.cc.o
/home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent: CMakeFiles/lars_loadbalance_agent.dir/src/host_info.cc.o
/home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent: CMakeFiles/lars_loadbalance_agent.dir/src/load_balance.cc.o
/home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent: CMakeFiles/lars_loadbalance_agent.dir/src/main_server.cc.o
/home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent: CMakeFiles/lars_loadbalance_agent.dir/src/reporter_client.cc.o
/home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent: CMakeFiles/lars_loadbalance_agent.dir/src/route_lb.cc.o
/home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent: CMakeFiles/lars_loadbalance_agent.dir/build.make
/home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent: CMakeFiles/lars_loadbalance_agent.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/qc/Lars/lars_loadbalance_agent/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX executable /home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lars_loadbalance_agent.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lars_loadbalance_agent.dir/build: /home/qc/Lars/lars_loadbalance_agent/bin/lars_loadbalance_agent
.PHONY : CMakeFiles/lars_loadbalance_agent.dir/build

CMakeFiles/lars_loadbalance_agent.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lars_loadbalance_agent.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lars_loadbalance_agent.dir/clean

CMakeFiles/lars_loadbalance_agent.dir/depend:
	cd /home/qc/Lars/lars_loadbalance_agent/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/qc/Lars/lars_loadbalance_agent /home/qc/Lars/lars_loadbalance_agent /home/qc/Lars/lars_loadbalance_agent/build /home/qc/Lars/lars_loadbalance_agent/build /home/qc/Lars/lars_loadbalance_agent/build/CMakeFiles/lars_loadbalance_agent.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/lars_loadbalance_agent.dir/depend

