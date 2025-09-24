file(GLOB_RECURSE EXAMPLE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(example ${EXAMPLE_SRC})

target_compile_definitions(example PUBLIC FMT_UNICODE=0)
target_include_directories(example PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Link internal libraries
target_link_libraries(example PRIVATE glfw glad imgui glm)
if (MSVC)
    set_target_properties(example PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

target_compile_definitions(example PUBLIC GLM_ENABLE_EXPERIMENTAL)