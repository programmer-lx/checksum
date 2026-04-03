cmake_minimum_required(VERSION 3.28)

project(
    checksum
    LANGUAGES CXX
)

# options
option(CKS_BUILD_DLL "Build checksum library as DLL?" OFF)
option(CKS_BUILD_TESTS "Build tests?" OFF)

set(CKS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# sub-directories
add_subdirectory(src)

# test
if(CKS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
