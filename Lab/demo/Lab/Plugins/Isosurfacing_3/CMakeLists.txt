include(CGALlab_macros)

cgal_lab_plugin(isosurface_3_plugin Isosurface_3_plugin KEYWORDS
                       Isosurface_3)
target_link_libraries(
  isosurface_3_plugin PRIVATE scene_polygon_soup_item scene_image_item )
