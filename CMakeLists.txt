cmake_minimum_required(VERSION 3.25)
project(sfa_hd_textures VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(sfa_hd_textures SHARED src/gametext_hd.cpp)
target_include_directories(sfa_hd_textures PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_target_properties(sfa_hd_textures PROPERTIES PREFIX "" OUTPUT_NAME "mod")

if (WIN32)
    set(FH_PLATFORM_DIR "windows-amd64")
elseif (APPLE)
    set(FH_PLATFORM_DIR "macos-arm64")
else()
    set(FH_PLATFORM_DIR "linux-amd64")
endif()

# Build the code portion into build/sfa-hd-textures. The repository's existing
# multi-gigabyte textures/ tree is intentionally NOT copied on every build.
set(FH_PACKAGE_DIR "${CMAKE_BINARY_DIR}/sfa-hd-textures")
set(FH_LIBRARY_DIR "${FH_PACKAGE_DIR}/lib/${FH_PLATFORM_DIR}")
set_target_properties(sfa_hd_textures PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${FH_LIBRARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FH_LIBRARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${FH_LIBRARY_DIR}"
)

add_custom_command(TARGET sfa_hd_textures POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FH_PACKAGE_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/mod.json" "${FH_PACKAGE_DIR}/mod.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/assets/glyphs.bin" "${FH_PACKAGE_DIR}/assets/glyphs.bin"
)
