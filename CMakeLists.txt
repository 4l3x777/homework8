cmake_minimum_required(VERSION 3.25)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
project(bayan VERSION ${PROJECT_VESRION})

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.13.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

set(BOOST_INCLUDE_LIBRARIES program_options algorithm uuid crc)
set(BOOST_ENABLE_CMAKE ON)
#Windows help debug
#set(Boost_DEBUG ON)
FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.81.0
)

FetchContent_MakeAvailable(Boost)

add_executable(
    bayan 
    bayan.cpp
)

add_executable(
    test_bayan_gtest 
    test_bayan_gtest.cpp
)

set_target_properties(
    bayan
    test_bayan_gtest
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(
    test_bayan_gtest 
    PROPERTIES 
    COMPILE_DEFINITIONS 
    STATIC_LINK
)

target_link_libraries(
    bayan
    Boost::program_options
    Boost::algorithm
    Boost::crc
    Boost::uuid
)

target_link_libraries(
    test_bayan_gtest    
    gtest 
    gtest_main
    Boost::program_options
    Boost::algorithm
    Boost::crc
    Boost::uuid
)

if (MSVC)
    target_compile_options(bayan PRIVATE /W4)
    target_compile_options(test_bayan_gtest PRIVATE /W4)
else ()
    target_link_options(bayan PRIVATE -static -static-libgcc -static-libstdc++)
    target_link_options(test_bayan_gtest PRIVATE -static -static-libgcc -static-libstdc++)

    target_compile_options(
        bayan 
        PRIVATE
        -Wall 
        -Wextra 
        -pedantic 
    )
    target_compile_options(
        test_bayan_gtest 
        PRIVATE
        -Wall 
        -Wextra 
        -pedantic 
    )
endif()

install (
    TARGETS 
    bayan 
    RUNTIME 
    DESTINATION 
    bin
)

# CPack area
if (WIN32)
    set(CPACK_GENERATOR "ZIP")
else ()
    set(CPACK_GENERATOR "DEB")
endif()
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT https://github.com/4l3x777/homework8)
include(CPack)

# Tests area
enable_testing()

add_test(
    NAME 
    unit_tests_test_bayan_gtest 
    COMMAND 
    test_bayan_gtest
)
