cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

project(docker_comms_emulator)

# Find the Ignition_Transport library
find_package(ignition-transport11 QUIET REQUIRED OPTIONAL_COMPONENTS log)
set(IGN_TRANSPORT_VER ${ignition-transport11_VERSION_MAJOR})

include_directories(${CMAKE_BINARY_DIR})

if (EXISTS "${CMAKE_SOURCE_DIR}/InitialSetup.cc")
  add_executable(InitialSetup InitialSetup.cc)
  target_link_libraries(InitialSetup ignition-transport${IGN_TRANSPORT_VER}::core)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/RobotToRobotCommsEmulator.cc")
  add_executable(RobotToRobotCommsEmulator RobotToRobotCommsEmulator.cc)
  target_link_libraries(RobotToRobotCommsEmulator ignition-transport${IGN_TRANSPORT_VER}::core)
endif()
