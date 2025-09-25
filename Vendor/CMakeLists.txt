# Add subdirectories
if(PREPATH_BUILD_EXAMPLES)
    message(STATUS "Prepath building GLFW")
    add_subdirectory(Glfw)
    message(STATUS "Prepath building spdlog")
    add_subdirectory(spdlog)
endif()

message(STATUS "Prepath building Glad")
add_subdirectory(Glad)
message(STATUS "Prepath building glm")
add_subdirectory(glm)

message(STATUS "Prepath building ImGui")
if(PREPATH_BUILD_EXAMPLES)
    file(GLOB IMGUI_SRC CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/ImGui/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ImGui/backends/imgui_impl_opengl3.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ImGui/backends/imgui_impl_glfw.cpp"
    )

    add_library(imgui STATIC ${IMGUI_SRC})

    target_link_libraries(imgui PUBLIC glfw)
    target_include_directories(imgui PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/ImGui"
        "${CMAKE_CURRENT_SOURCE_DIR}/ImGui/backends"
    )

    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
    message(STATUS "Prepath building Assimp")
    add_subdirectory(assimp)
endif()