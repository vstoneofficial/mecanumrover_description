cmake_minimum_required(VERSION 3.8)
project(mecanumrover_description)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rviz_common REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Use the Jazzy RViz libraries with Qt-thread shutdown before ROS teardown.
add_executable(mecanumrover_rviz rviz_viewer.cpp)
target_compile_features(mecanumrover_rviz PRIVATE cxx_std_17)
ament_target_dependencies(mecanumrover_rviz rclcpp rviz_common)
target_link_libraries(mecanumrover_rviz Qt5::Widgets)
install(TARGETS mecanumrover_rviz DESTINATION lib/${PROJECT_NAME})

message(STATUS ">>> Installing rover_twist_relay.py into lib/${PROJECT_NAME}")

# Python node
install(
  PROGRAMS
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/rover_twist_relay.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/joint_state_gui.py
  DESTINATION lib/${PROJECT_NAME}
)

# YAML for rover_twist_relay (explicit install)
install(
  FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/rover_twist_relay.yaml
  DESTINATION share/${PROJECT_NAME}/scripts
)

# Resource directories
install(DIRECTORY
  launch
  meshes
  rviz
  urdf
  params
  worlds
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

