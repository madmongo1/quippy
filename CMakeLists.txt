cmake_minimum_required(VERSION 3.6)

include("cmake/HunterGate.cmake")
include("cmake/all_functions.cmake")

HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.18.15.tar.gz"
        SHA1 "57991b41173c0e3c7501c7929cad8323277fab4c"
        LOCAL
)

project(quippy VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

hunter_add_package(OpenSSL)
hunter_add_package(Boost COMPONENTS system thread)
hunter_add_package(gtest)
find_package(GTEST REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

add_subdirectory(AMQP-CPP)

configure_file(include/quippy/config.hpp.in include/quippy/config.hpp)
configure_file(src/version.cpp.in src/version.cpp)

set(PRIVATE_INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/include/quippy/config.hpp)
prefix_files(include.quippy.detail PREFIX "include/quippy/detail" APPEND PRIVATE_INTERFACE FILES connection_handle.hpp)
prefix_files(src PREFIX "src" APPEND SOURCE_FILES FILES version.cpp connector.cpp)
prefix_files(src.detail PREFIX "src/detail" APPEND SOURCE_FILES FILES connection.cpp connection_handler.cpp)

add_library(quippy ${SOURCE_FILES} ${PRIVATE_INTERFACE_FILES} ${PUBLIC_INTERFACE_FILES})
target_include_directories(quippy SYSTEM PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(quippy PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_link_libraries(quippy PUBLIC amqp-cpp Boost::system Boost::thread)

prefix_files(test PREFIX "test" APPEND TEST_SOURCES FILES test1.cpp)

add_executable(quippy-test ${TEST_SOURCES})
target_link_libraries(quippy-test PRIVATE quippy ${GTEST_BOTH_LIBRARIES})
target_include_directories(quippy-test SYSTEM PRIVATE ${GTEST_INCLUDE_DIRS})
