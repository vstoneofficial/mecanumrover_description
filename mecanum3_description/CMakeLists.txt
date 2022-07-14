cmake_minimum_required(VERSION 2.8.3)
project(mecanum3_description)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  rospy
)

catkin_package(

)


include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)