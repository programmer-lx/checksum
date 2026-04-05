project(
    checksum_test
    LANGUAGES C CXX
)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/programmer-lx/googletest.git
        GIT_TAG        v1.17.0
)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# add tests
function(cks_add_test test_file test_name)
    add_executable(${test_name} ${test_file})

    # C++20 for tests
    set_target_properties(${test_name} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )

    target_compile_definitions(${test_name} PRIVATE CKS_IS_TESTING)
    
    if(MSVC)
        target_compile_definitions(${test_name} PRIVATE UNICODE _UNICODE)
        target_compile_options(${test_name} PRIVATE /utf-8 /W4 /WX /GR-)
    else()
        target_compile_options(${test_name} PRIVATE -fno-exceptions -fno-rtti -Werror -Wall)
    endif()

    target_link_libraries(${test_name} PRIVATE checksum gtest)

    # copy DLL
    if(CKS_BUILD_DLL)
        add_custom_command(TARGET ${test_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:checksum>             # 源文件：checksum.dll 的路径
                $<TARGET_FILE_DIR:${test_name}>     # 目标目录
        )
    endif()

    add_test(NAME test_${test_name} COMMAND ${test_name})
endfunction()

if(CKS_BUILD_DLL)
    cks_add_test(dll.cpp dll)
else()
    cks_add_test(build.cpp build_test)
    cks_add_test(str.cpp str)
    cks_add_test(crc32c.cpp crc32c)
    cks_add_test(sha256.cpp sha256)
    cks_add_test(md5.cpp md5)
    cks_add_test(xxhash3_64.cpp xxhash3_64)
    cks_add_test(xxhash3_128.cpp xxhash3_128)
endif()
