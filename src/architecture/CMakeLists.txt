cmake_minimum_required ( VERSION 3.0.2 )

project ( architecture )

find_package ( glm REQUIRED )
find_package ( GLEW REQUIRED )

add_library ( architecture 
    castle.h
    castle.cpp
    )

target_include_directories( architecture
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${GLM_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    )

target_link_libraries ( architecture
    PRIVATE
    ${GLEW_LIBRARIES}
    )
