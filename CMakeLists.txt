cmake_minimum_required(VERSION 3.8)

# Name of project
project(Image_Vectorization)

# Specify project header files
set(HDRS
    include/EdgeDetection.h
    include/Hybrid.h
    include/Node.h
    include/PixelGraph.h
)

# Specify project source files
set(SRCS
    src/EdgeDetection.cpp
    src/Hybrid.cpp
    src/Main.cpp
    src/Node.cpp
    src/PixelGraph.cpp
)

# Add executable based on the source files, using the project name as the executable name
set(MAIN_EXECUTABLE ${PROJECT_NAME})
add_executable(${MAIN_EXECUTABLE} ${HDRS} ${SRCS})

# search for headers in the "include" directory
include_directories(include)

# Link OpenCV
set(OpenCV_LIB_RELEASE C:/OpenCV3.2/build/x64/vc14/lib/opencv_world320.lib)
set(OpenCV_LIB_DEBUG C:/OpenCV3.2/build/x64/vc14/lib/opencv_world320d.lib)
set(OpenCV_INCLUDE C:/OpenCV3.2/build/include)

include_directories(${OpenCV_INCLUDE})
target_link_libraries(${MAIN_EXECUTABLE} ${OpenCV_LIB_RELEASE})
target_link_libraries(${MAIN_EXECUTABLE} ${OpenCV_LIB_DEBUG})

# Create a dependency to copy the ./input/ directory to the build folder
add_custom_target(inputDirCopy)
add_custom_command(
        TARGET inputDirCopy PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/input"
                "input")
add_dependencies(${MAIN_EXECUTABLE} inputDirCopy)

# The rules here are specific to Windows Systems
if(WIN32)
    # This will use the proper libraries in debug mode in Visual Studio
    set_target_properties(${MAIN_EXECUTABLE} PROPERTIES DEBUG_POSTFIX _d)

    # Automatically generate source groups for Visual Studio
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HDRS} ${SRCS})
endif(WIN32)
