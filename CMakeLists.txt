cmake_minimum_required(VERSION 3.25)
project(sfa_hd_text VERSION 1.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(sfa_hd_text SHARED src/gametext_hd.cpp)
target_include_directories(sfa_hd_text PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_target_properties(sfa_hd_text PROPERTIES PREFIX "" OUTPUT_NAME "mod")

if (WIN32)
    set(FH_PLATFORM_DIR "windows-amd64")
elseif (APPLE)
    set(FH_PLATFORM_DIR "macos-arm64")
    set_target_properties(sfa_hd_text PROPERTIES SUFFIX ".so")
else()
    set(FH_PLATFORM_DIR "linux-amd64")
    set_target_properties(sfa_hd_text PROPERTIES SUFFIX ".so")
endif()

set(FH_PACKAGE_DIR "${CMAKE_BINARY_DIR}/sfa-hd-text")
set(FH_LIBRARY_DIR "${FH_PACKAGE_DIR}/lib/${FH_PLATFORM_DIR}")
set_target_properties(sfa_hd_text PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${FH_LIBRARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FH_LIBRARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FH_LIBRARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${FH_LIBRARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${FH_LIBRARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${FH_LIBRARY_DIR}"
)

add_custom_command(TARGET sfa_hd_text POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FH_PACKAGE_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/mod.json" "${FH_PACKAGE_DIR}/mod.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/assets/glyphs.bin" "${FH_PACKAGE_DIR}/assets/glyphs.bin"
)

set(FH_RELEASE_DIR "${CMAKE_BINARY_DIR}/package/sfa-hd-text")
add_custom_target(package
    DEPENDS sfa_hd_text
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${FH_RELEASE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FH_RELEASE_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FH_RELEASE_DIR}/lib/${FH_PLATFORM_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/mod.json" "${FH_RELEASE_DIR}/mod.json"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/glyphs.bin" "${FH_RELEASE_DIR}/assets/glyphs.bin"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:sfa_hd_text>" "${FH_RELEASE_DIR}/lib/${FH_PLATFORM_DIR}/$<TARGET_FILE_NAME:sfa_hd_text>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/textures" "${FH_RELEASE_DIR}/textures"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${FH_RELEASE_DIR}/README.md"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${FH_RELEASE_DIR}/LICENSE"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/dist"
    COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_BINARY_DIR}/package" ${CMAKE_COMMAND} -E tar cf "${CMAKE_BINARY_DIR}/dist/sfa-hd-text-${PROJECT_VERSION}-${FH_PLATFORM_DIR}.zip" --format=zip sfa-hd-text
    COMMAND ${CMAKE_COMMAND} -E chdir "${FH_RELEASE_DIR}" ${CMAKE_COMMAND} -E tar cf "${CMAKE_BINARY_DIR}/dist/sfa-hd-text-${PROJECT_VERSION}-${FH_PLATFORM_DIR}.fox" --format=zip mod.json assets textures lib README.md LICENSE
    COMMENT "Assembling Star Fox Adventures HD Text Pack"
    VERBATIM
)
