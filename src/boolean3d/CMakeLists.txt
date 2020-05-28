cmake_minimum_required ( VERSION 3.0.2 )

project ( boolean3d )

find_package(CGAL CONFIG REQUIRED COMPONENTS Core)
find_package ( glm REQUIRED )

add_library ( boolean3d 
    boolean3d.h
    boolean3d.cpp
    )

target_include_directories( boolean3d
	PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${GLM_INCLUDE_DIRS}
    )

target_link_libraries ( boolean3d
    PUBLIC
    CGAL::CGAL
    )
