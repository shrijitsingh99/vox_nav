cmake_minimum_required(VERSION 3.5)
project(vox_nav_utilities)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBFCL REQUIRED "fcl>=0.5.0")
# replace LIBFCL_LIBRARIES with full path to the library
find_library(LIBFCL_LIBRARIES_FULL ${LIBFCL_LIBRARIES} ${LIBFCL_LIBRARY_DIRS})
set(LIBFCL_LIBRARIES "${LIBFCL_LIBRARIES_FULL}")

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(pcl_ros REQUIRED)
find_package(PCL REQUIRED)
find_package(OCTOMAP REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(ompl REQUIRED)
find_package(OCTOMAP REQUIRED)
find_package(octomap_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(PCL REQUIRED)
find_package(vox_nav_msgs REQUIRED)

set(Torch_DIR "/opt/libtorch/share/cmake/Torch") 
set(CMAKE_CUDA_COMPILER "/usr/local/cuda/bin/nvcc")
option(USE_CUDA "Use CUDA" ON)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-O3 -gencode arch=compute_60,code=sm_60)
set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda")
find_package(Torch REQUIRED)
find_package(CUDA REQUIRED)
find_package(CUDNN REQUIRED)

include_directories(include
                   ${ApproxMVBB_INCLUDE_DIRS}
                   ${OMPL_INCLUDE_DIRS}
                   ${PCL_INCLUDE_DIRS}
                   ${Torch_INCLUDE_DIRS}
                   ${CUDA_INCLUDE_DIRS})

set(dependencies
  rclcpp
  sensor_msgs
  geometry_msgs
  nav_msgs
  rclcpp
  tf2
  tf2_ros
  tf2_geometry_msgs
  pcl_ros
  Eigen3
  OCTOMAP
  octomap_msgs
  visualization_msgs
  vox_nav_msgs
)

add_library(tf_helpers SHARED src/tf_helpers.cpp src/pcl_helpers.cpp)
target_link_libraries(tf_helpers ${PCL_LIBRARIES})
ament_target_dependencies(tf_helpers ${dependencies})

add_library(planner_helpers SHARED src/planner_helpers.cpp)
ament_target_dependencies(planner_helpers ${dependencies})
target_link_libraries(planner_helpers ${LIBFCL_LIBRARIES} tf_helpers ompl)

add_library(map_manager_helpers SHARED src/map_manager_helpers.cpp)
ament_target_dependencies(map_manager_helpers ${dependencies})

add_library(gps_waypoint_collector SHARED src/gps_waypoint_collector.cpp)
ament_target_dependencies(gps_waypoint_collector ${dependencies})

add_library(elevation_state_space SHARED src/elevation_state_space.cpp)
target_link_libraries(elevation_state_space ${PCL_LIBRARIES})
ament_target_dependencies(elevation_state_space ${dependencies})

add_executable(gps_waypoint_collector_node src/gps_waypoint_collector_node.cpp)
target_link_libraries(gps_waypoint_collector_node gps_waypoint_collector)
ament_target_dependencies(gps_waypoint_collector_node ${dependencies})

add_executable(pcl2octomap_converter_node  src/pcl_helpers.cpp src/pcl2octomap_converter_node.cpp)
target_link_libraries(pcl2octomap_converter_node ${PCL_LIBRARIES})
ament_target_dependencies(pcl2octomap_converter_node ${dependencies})

add_executable(planner_benchmarking_node src/planner_benchmarking_node.cpp)
ament_target_dependencies(planner_benchmarking_node ${dependencies})
target_link_libraries(planner_benchmarking_node ${LIBFCL_LIBRARIES} tf_helpers elevation_state_space planner_helpers ompl)

add_executable(dynamic_points_node      src/pointnet2_utils.cpp
                                        src/infer.cpp)
ament_target_dependencies(dynamic_points_node ${dependencies})
target_link_libraries(dynamic_points_node tf_helpers map_manager_helpers ${TORCH_LIBRARIES} ${PCL_LIBRARIES})

install(TARGETS tf_helpers 
                planner_helpers 
                map_manager_helpers
                gps_waypoint_collector 
                elevation_state_space
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(TARGETS gps_waypoint_collector_node 
                pcl2octomap_converter_node 
                planner_benchmarking_node 
                dynamic_points_node
        RUNTIME DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY include/
        DESTINATION include/)

install(DIRECTORY config launch
        DESTINATION share/${PROJECT_NAME})

ament_export_libraries(tf_helpers 
                        planner_helpers 
                        map_manager_helpers
                        gps_waypoint_collector
                        elevation_state_space)
ament_export_dependencies(${dependencies})
ament_export_include_directories(include)

ament_package()
