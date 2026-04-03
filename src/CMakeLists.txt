# lib
file(GLOB_RECURSE CKS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
if(CKS_BUILD_DLL)
    add_library(checksum SHARED ${CKS_SOURCE})
else()
    add_library(checksum STATIC ${CKS_SOURCE})
endif()

target_include_directories(checksum PUBLIC ${CKS_ROOT_DIR}/include)

# C++20
target_compile_features(checksum PRIVATE cxx_std_20)

# compiler options
if(MSVC)
    target_compile_definitions(checksum PRIVATE UNICODE _UNICODE)
    target_compile_options(checksum PRIVATE /utf-8 /W4 /WX /EHs-c- /GR-)
else()
    target_compile_options(checksum PRIVATE -fno-exceptions -fno-rtti -Werror -Wall)
endif()
