cmake_minimum_required (VERSION 3.13)

project (bootstrap)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (PROJECT_NAME img2sky)

project (img2sky)

file (GLOB_RECURSE INCLUDES ../src/*.h)
file (GLOB_RECURSE SOURCES ../src/*.cpp ../src/*.c)

set (SOURCE_LIST ${INCLUDES} ${SOURCES})

add_executable (${PROJECT_NAME} ${SOURCE_LIST})

target_include_directories (${PROJECT_NAME} PUBLIC ../src/)

target_link_libraries (${PROJECT_NAME})
