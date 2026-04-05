# lib
file(GLOB_RECURSE CKS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR} "*.c")
if(CKS_BUILD_DLL)
    add_library(checksum SHARED ${CKS_SOURCE})
    target_compile_definitions(checksum PRIVATE CKS_BUILD_DLL)
else()
    add_library(checksum STATIC ${CKS_SOURCE})
    target_compile_definitions(checksum PUBLIC CKS_BUILD_STATIC)
endif()

target_include_directories(checksum PUBLIC ${CKS_ROOT_DIR}/include)

# C11
target_compile_features(checksum PRIVATE c_std_11)

# compiler options
if(MSVC)
    target_compile_definitions(checksum PRIVATE UNICODE _UNICODE)
    target_compile_options(checksum PRIVATE /utf-8 /W4 /WX)
else()
    target_compile_options(checksum PRIVATE -Werror -Wall)
endif()
