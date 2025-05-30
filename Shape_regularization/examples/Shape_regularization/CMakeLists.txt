# Created by the script cgal_create_CMakeLists.
# This is the CMake script for compiling a set of CGAL applications.

cmake_minimum_required(VERSION 3.12...3.31)

project(Shape_regularization_Examples)

find_package(CGAL REQUIRED COMPONENTS Core)

# Find OSQP library and headers.
find_package(OSQP QUIET)
include(CGAL_OSQP_support)
if(TARGET CGAL::OSQP_support)
  message(STATUS "Found OSQP")

  set(osqp_targets
  regularize_15_segments
  regularize_100_segments_angles
  regularize_100_segments_offsets
  regularize_simple)

  foreach(osqp_target ${osqp_targets})
    create_single_source_cgal_program("${osqp_target}.cpp")
    if(TARGET ${osqp_target})
      target_link_libraries(${osqp_target} PRIVATE CGAL::OSQP_support)
    endif()
  endforeach()

  # Use Eigen.
  find_package(Eigen3 3.1.0 QUIET) # (3.1.0 or greater)
  include(CGAL_Eigen3_support)
  if(TARGET CGAL::Eigen3_support)
    message(STATUS "Found Eigen")

    create_single_source_cgal_program("regularize_real_data_2.cpp")
    target_link_libraries(regularize_real_data_2 PRIVATE CGAL::Eigen3_support CGAL::OSQP_support)
  else()
    message(STATUS "NOTICE: Eigen was not found. Eigen examples won't be available.")
  endif()
else()
  message(STATUS "NOTICE: OSQP was not found. OSQP examples won't be available.")
endif()

create_single_source_cgal_program("regularize_framework.cpp")
create_single_source_cgal_program("regularize_planes.cpp")
create_single_source_cgal_program("regularize_contour.cpp")
create_single_source_cgal_program("regularize_closed_contour.cpp")
create_single_source_cgal_program("regularize_open_contour.cpp")
