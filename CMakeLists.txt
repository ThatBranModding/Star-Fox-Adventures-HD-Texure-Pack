cmake_minimum_required(VERSION 3.25)
project(sfa_hd_textures VERSION 1.2.0 LANGUAGES NONE)

set(FH_RELEASE_DIR "${CMAKE_BINARY_DIR}/package/sfa-hd-textures")
set(FH_DIST_DIR "${CMAKE_BINARY_DIR}/dist")
add_custom_target(package
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${FH_RELEASE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FH_RELEASE_DIR}" "${FH_DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/mod.json" "${FH_RELEASE_DIR}/mod.json"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${FH_RELEASE_DIR}/README.md"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${FH_RELEASE_DIR}/LICENSE"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/textures" "${FH_RELEASE_DIR}/textures"
    COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_BINARY_DIR}/package" ${CMAKE_COMMAND} -E tar cf "${FH_DIST_DIR}/sfa-hd-textures-${PROJECT_VERSION}-all-platforms.zip" --format=zip sfa-hd-textures
    COMMAND ${CMAKE_COMMAND} -E chdir "${FH_RELEASE_DIR}" ${CMAKE_COMMAND} -E tar cf "${FH_DIST_DIR}/sfa-hd-textures-${PROJECT_VERSION}.fox" --format=zip mod.json textures README.md LICENSE
    COMMENT "Packaging standalone HD world textures"
    VERBATIM
)
