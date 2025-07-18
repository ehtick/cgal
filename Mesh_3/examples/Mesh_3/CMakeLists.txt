cmake_minimum_required(VERSION 3.12...3.31)
project(Mesh_3_Examples)

add_compile_definitions(CGAL_MESH_3_NO_DEPRECATED_SURFACE_INDEX
                        CGAL_MESH_3_NO_DEPRECATED_C3T3_ITERATORS)

if(CGAL_MESH_3_VERBOSE)
  add_compile_definitions(CGAL_MESH_3_VERBOSE)
endif()

find_package(CGAL REQUIRED COMPONENTS ImageIO)

option(CGAL_ACTIVATE_CONCURRENT_MESH_3 "Activate parallelism in Mesh_3" OFF)

if(CGAL_ACTIVATE_CONCURRENT_MESH_3 OR "$ENV{CGAL_ACTIVATE_CONCURRENT_MESH_3}")
  add_compile_definitions(CGAL_CONCURRENT_MESH_3)
  find_package(TBB REQUIRED)
  include(CGAL_TBB_support)
endif()

# Use Eigen
find_package(Eigen3 3.1.0 QUIET) #(3.1.0 or greater)
include(CGAL_Eigen3_support)
if(NOT TARGET CGAL::Eigen3_support)
  message("NOTICE: All examples require the Eigen3 library, and will not be compiled.")
  return()
endif()

set(CMAKE_POLICY_DEFAULT_CMP0167 NEW)
find_package(VTK 9.0
             COMPONENTS ImagingGeneral IOImage
             QUIET)
if(VTK_FOUND)
  message(STATUS "VTK ${VTK_VERSION} found ${VTK_LIBRARIES}")
else()
  message(STATUS "VTK was not found")
endif()

create_single_source_cgal_program("mesh_hybrid_mesh_domain.cpp")
target_link_libraries(mesh_hybrid_mesh_domain PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_implicit_sphere.cpp")
target_link_libraries(mesh_implicit_sphere PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_implicit_ellipsoid.cpp")
target_link_libraries(mesh_implicit_ellipsoid PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_implicit_sphere_variable_size.cpp")
target_link_libraries(mesh_implicit_sphere_variable_size PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_two_implicit_spheres_with_balls.cpp")
target_link_libraries(mesh_two_implicit_spheres_with_balls PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_implicit_domains_2.cpp"
                                  "implicit_functions.cpp")
target_link_libraries(mesh_implicit_domains_2 PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_cubes_intersection.cpp")
target_link_libraries(mesh_cubes_intersection PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_cubes_intersection_with_features.cpp")
target_link_libraries(mesh_cubes_intersection_with_features PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_implicit_domains.cpp"
                                  "implicit_functions.cpp")
target_link_libraries(mesh_implicit_domains PRIVATE CGAL::Eigen3_support)

find_package( ITT QUIET )
if(TARGET ITT::ITT)
  target_link_libraries(mesh_implicit_domains PRIVATE ITT::ITT)
  target_compile_definitions(mesh_implicit_domains PRIVATE CGAL_MESH_3_USE_INTEL_ITT)
endif()

create_single_source_cgal_program( "mesh_polyhedral_domain.cpp" )
target_link_libraries(mesh_polyhedral_domain PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_polyhedral_domain_sm.cpp")
target_link_libraries(mesh_polyhedral_domain_sm PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program(
  "mesh_polyhedral_domain_with_surface_inside.cpp")
target_link_libraries(mesh_polyhedral_domain_with_surface_inside
                      PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("remesh_polyhedral_surface.cpp")
target_link_libraries(remesh_polyhedral_surface PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("remesh_polyhedral_surface_sm.cpp")
target_link_libraries(remesh_polyhedral_surface_sm PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_polyhedral_domain_with_features.cpp")
target_link_libraries(mesh_polyhedral_domain_with_features
                      PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_polyhedral_domain_with_features_sizing.cpp")
target_link_libraries(mesh_polyhedral_domain_with_features_sizing
                      PRIVATE CGAL::Eigen3_support)

                    create_single_source_cgal_program("mesh_polyhedral_domain_with_edge_distance.cpp")
target_link_libraries(mesh_polyhedral_domain_with_edge_distance
                      PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_polyhedral_domain_with_features_sm.cpp")
target_link_libraries(mesh_polyhedral_domain_with_features_sm
                      PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program(
  "mesh_polyhedral_domain_with_lipschitz_sizing.cpp")
target_link_libraries(mesh_polyhedral_domain_with_lipschitz_sizing
                      PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_polyhedral_complex.cpp")
target_link_libraries(mesh_polyhedral_complex PRIVATE CGAL::Eigen3_support)

create_single_source_cgal_program("mesh_polyhedral_complex_sm.cpp")
target_link_libraries(mesh_polyhedral_complex_sm PRIVATE CGAL::Eigen3_support)

if(TARGET CGAL::CGAL_ImageIO)
  if(VTK_FOUND)
    create_single_source_cgal_program("mesh_3D_gray_vtk_image.cpp")
    target_link_libraries(mesh_3D_gray_vtk_image
                          PRIVATE CGAL::Eigen3_support
                                 CGAL::CGAL_ImageIO
                          PRIVATE ${VTK_LIBRARIES})
    add_to_cached_list(CGAL_EXECUTABLE_TARGETS mesh_3D_gray_vtk_image)
  endif()

  create_single_source_cgal_program("mesh_3D_gray_image.cpp")
  target_link_libraries(mesh_3D_gray_image PRIVATE CGAL::Eigen3_support)

  create_single_source_cgal_program("mesh_3D_gray_image_multiple_values.cpp")
  target_link_libraries(mesh_3D_gray_image_multiple_values
                        PRIVATE CGAL::Eigen3_support)

  create_single_source_cgal_program("mesh_3D_image_with_features.cpp")
  target_link_libraries(mesh_3D_image_with_features PRIVATE CGAL::Eigen3_support)

  create_single_source_cgal_program("mesh_3D_image_with_input_features.cpp")
  target_link_libraries(mesh_3D_image_with_input_features PRIVATE CGAL::Eigen3_support)

  create_single_source_cgal_program("mesh_3D_image_with_detection_of_features.cpp")
  target_link_libraries(mesh_3D_image_with_detection_of_features PRIVATE CGAL::Eigen3_support)

  if(CGAL_ImageIO_USE_ZLIB)
    create_single_source_cgal_program("mesh_optimization_example.cpp")
    target_link_libraries(mesh_optimization_example PRIVATE CGAL::Eigen3_support)

    create_single_source_cgal_program("mesh_optimization_lloyd_example.cpp")
    target_link_libraries(mesh_optimization_lloyd_example
                          PRIVATE CGAL::Eigen3_support)

    create_single_source_cgal_program("mesh_3D_image.cpp")
    target_link_libraries(mesh_3D_image PRIVATE CGAL::Eigen3_support)

    create_single_source_cgal_program(
      "mesh_3D_image_with_custom_initialization.cpp")
    target_link_libraries(mesh_3D_image_with_custom_initialization
                          PRIVATE CGAL::Eigen3_support)

    create_single_source_cgal_program(
      "mesh_3D_image_with_image_initialization.cpp")
    target_link_libraries(mesh_3D_image_with_image_initialization
                          PRIVATE CGAL::Eigen3_support)

    create_single_source_cgal_program(
      "mesh_3D_image_with_initial_points.cpp")
    target_link_libraries(mesh_3D_image_with_initial_points
                          PUBLIC CGAL::Eigen3_support)

    create_single_source_cgal_program("mesh_3D_image_variable_size.cpp")
    target_link_libraries(mesh_3D_image_variable_size
                          PRIVATE CGAL::Eigen3_support)

    set(CMAKE_POLICY_DEFAULT_CMP0167 NEW)
    find_package(ITK NAMES ITK InsightToolkit
                     QUIET COMPONENTS ITKCommon ITKThresholding ITKSmoothing ITKImageIntensity)
    if(ITK_FOUND)
      include(CGAL_ITK_support)
      message(STATUS "ITK found")
      include(${ITK_USE_FILE})

      create_single_source_cgal_program("mesh_3D_weighted_image.cpp")
      target_link_libraries(mesh_3D_weighted_image
                            PRIVATE CGAL::Eigen3_support CGAL::ITK_support)

      create_single_source_cgal_program("mesh_3D_weighted_image_with_detection_of_features.cpp")
      target_link_libraries(mesh_3D_weighted_image_with_detection_of_features
                            PRIVATE CGAL::Eigen3_support CGAL::ITK_support)
    else()
      message(STATUS "NOTICE: The examples that need ITK will not be compiled.")
    endif()

  else()
    message(STATUS "NOTICE: The examples mesh_3D_image.cpp, mesh_3D_weighted_image.cpp, mesh_3D_image_variable_size.cpp, mesh_optimization_example.cpp and mesh_optimization_lloyd_example.cpp need CGAL_ImageIO to be configured with ZLIB support, and will not be compiled.")
  endif()

else()
  message(STATUS "NOTICE: Some examples need the CGAL_ImageIO library, and will not be compiled.")
endif()

if(CGAL_ACTIVATE_CONCURRENT_MESH_3 AND TARGET CGAL::TBB_support)
  foreach(
    target
    mesh_3D_image
    mesh_3D_weighted_image
    mesh_3D_weighted_image_with_detection_of_features
    mesh_3D_image_variable_size
    mesh_3D_image_with_custom_initialization
    mesh_3D_image_with_initial_points
    mesh_3D_image_with_image_initialization
    mesh_3D_image_with_features
    mesh_3D_image_with_detection_of_features
    mesh_3D_image_with_input_features
    mesh_implicit_domains
    mesh_implicit_sphere
    mesh_implicit_sphere_variable_size
    mesh_optimization_example
    mesh_optimization_lloyd_example
    mesh_polyhedral_complex
    mesh_polyhedral_complex_sm
    mesh_polyhedral_domain
    mesh_polyhedral_domain_sm
    mesh_polyhedral_domain_with_features
    mesh_polyhedral_domain_with_features_sizing
    mesh_polyhedral_domain_with_edge_distance
    mesh_polyhedral_domain_with_features_sm
    mesh_polyhedral_domain_with_lipschitz_sizing
    mesh_two_implicit_spheres_with_balls)
    if(TARGET ${target})
      target_link_libraries(${target} PRIVATE CGAL::TBB_support)
    endif()
  endforeach()
endif()
