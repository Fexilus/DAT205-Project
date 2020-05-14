cmake_minimum_required ( VERSION 3.0.2 )

project ( mousepicking )

find_package ( glm REQUIRED )
find_package ( GLEW REQUIRED )

# Find *all* shaders.
file(GLOB_RECURSE SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.frag"
)
# Separate filter for shaders.
source_group("Shaders" FILES ${SHADERS})

add_library ( mousepicking 
    picking.h
    picking.cpp
    ${SHADERS}
    )

target_include_directories( mousepicking
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${GLM_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    )

target_link_libraries ( mousepicking
    PRIVATE
    ${GLEW_LIBRARIES}
    )
