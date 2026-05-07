cmake_minimum_required(VERSION 3.12)

project(chip-8-emulator C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
)
FetchContent_MakeAvailable(raylib)

set(SOURCES 
    src/main.c
    src/emulator.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/include
)