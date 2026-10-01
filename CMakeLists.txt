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
    set_target_properties(sfa_hd_textures PROPERTIES SUFFIX ".dylib")
else()
    set(FH_PLATFORM_DIR "linux-amd64")
    set_target_properties(sfa_hd_textures PROPERTIES SUFFIX ".so")
endif()

set(FH_PACKAGE_DIR "${CMAKE_BINARY_DIR}/sfa-hd-textures")
set(FH_LIBRARY_DIR "${FH_PACKAGE_DIR}/lib/${FH_PLATFORM_DIR}")
set_target_properties(sfa_hd_textures PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${FH_LIBRARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FH_LIBRARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FH_LIBRARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${FH_LIBRARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${FH_LIBRARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${FH_LIBRARY_DIR}"
)

add_custom_command(TARGET sfa_hd_textures POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FH_PACKAGE_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/mod.json" "${FH_PACKAGE_DIR}/mod.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/assets/glyphs.bin" "${FH_PACKAGE_DIR}/assets/glyphs.bin"
)

set(FH_RELEASE_DIR "${CMAKE_BINARY_DIR}/package/sfa-hd-textures")
add_custom_target(package
    DEPENDS sfa_hd_textures
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${FH_RELEASE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FH_RELEASE_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FH_RELEASE_DIR}/lib/${FH_PLATFORM_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/mod.json" "${FH_RELEASE_DIR}/mod.json"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/glyphs.bin" "${FH_RELEASE_DIR}/assets/glyphs.bin"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:sfa_hd_textures>" "${FH_RELEASE_DIR}/lib/${FH_PLATFORM_DIR}/$<TARGET_FILE_NAME:sfa_hd_textures>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/textures" "${FH_RELEASE_DIR}/textures"
    COMMENT "Assembling complete Star Fox Adventures HD Texture Pack"
    VERBATIM
)
