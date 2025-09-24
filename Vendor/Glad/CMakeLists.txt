file(GLOB GLAD_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_library(glad STATIC ${GLAD_SRC})

target_include_directories(glad PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)