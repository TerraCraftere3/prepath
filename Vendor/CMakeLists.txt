# Add subdirectories
add_subdirectory(Glad)
add_subdirectory(Glfw)
add_subdirectory(glm)

# ImGui
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