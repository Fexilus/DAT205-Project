cmake_minimum_required ( VERSION 3.0.2 )

project ( architecture )

find_package ( glm REQUIRED )
find_package ( GLEW REQUIRED )

add_library ( architecture 
    castle.h
    castle.cpp
	shape.h
	shape.cpp
    )

target_include_directories( architecture
	PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external_src/cpp11-range
    PRIVATE
    ${GLM_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    )

target_link_libraries ( architecture
    PUBLIC
    boolean3d
    PRIVATE
    ${GLEW_LIBRARIES}
    )
