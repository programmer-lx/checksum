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

    target_compile_definitions(${test_name} PRIVATE CKS_IS_TESTING)
    
    if(MSVC)
        target_compile_definitions(${test_name} PRIVATE UNICODE _UNICODE)
        target_compile_options(${test_name} PRIVATE /utf-8 /W4 /WX /GR-)
    else()
        target_compile_options(${test_name} PRIVATE -fno-exceptions -fno-rtti -Werror -Wall)
    endif()

    target_link_libraries(${test_name} PRIVATE checksum gtest)

    add_test(NAME test_${test_name} COMMAND ${test_name})
endfunction()

cks_add_test(crc32c.cpp crc32c)
