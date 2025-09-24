cmake_minimum_required(VERSION 3.16)
project(prepath LANGUAGES CXX)

# ---- Sources ----
file(GLOB_RECURSE PREPATH_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Prepath/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Prepath/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Prepath/*.hpp"
)

# ---- Library ----
add_library(prepath ${PREPATH_SRC})

target_compile_definitions(prepath PUBLIC
    FMT_UNICODE=0
    GLM_ENABLE_EXPERIMENTAL
)

target_include_directories(prepath PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(prepath PUBLIC
    ${PREPATH_GLAD_NAME}
    ${PREPATH_GLM_NAME}
)