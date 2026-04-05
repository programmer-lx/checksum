# xxHash static library
set(XXHASH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/xxHash)

# xxHash source files (with x86 dispatch for runtime CPU detection)
set(XXHASH_SOURCES ${XXHASH_DIR}/xxhash.c)

# Add x86 dispatch on x86 platforms
if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64|i[3-6]86|x86" OR 
   CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64|i[3-6]86|x86")
    list(APPEND XXHASH_SOURCES ${XXHASH_DIR}/xxh_x86dispatch.c)
endif()

add_library(xxhash STATIC ${XXHASH_SOURCES})

target_include_directories(xxhash PUBLIC ${XXHASH_DIR})

# C11 standard
target_compile_features(xxhash PRIVATE c_std_11)

# Compiler options
if(MSVC)
    target_compile_options(xxhash PRIVATE /W4)
else()
    target_compile_options(xxhash PRIVATE -Wall)
endif()
