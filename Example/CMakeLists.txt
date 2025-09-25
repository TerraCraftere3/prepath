file(GLOB_RECURSE PREPATH_EXAMPLE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(prepath_example ${PREPATH_EXAMPLE_SRC})

target_compile_definitions(prepath_example PUBLIC FMT_UNICODE=0)
target_include_directories(prepath_example PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Link internal libraries
target_link_libraries(prepath_example PRIVATE 
    ${PREPATH_GLFW_NAME}     
    ${PREPATH_GLAD_NAME}
    ${PREPATH_GLM_NAME} 
    ${PREPATH_IMGUI_NAME}  
    ${PREPATH_SPDLOG_NAME}
    ${PREPATH_ASSIMP_NAME}
    prepath 
    )
if (MSVC)
    set_target_properties(prepath_example PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

target_compile_definitions(prepath_example PUBLIC GLM_ENABLE_EXPERIMENTAL)