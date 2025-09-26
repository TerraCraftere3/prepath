cmake_minimum_required(VERSION 4.0)

# ---- Options ----
option(PREPATH_BUILD_EXAMPLES "Build example executables" ON)
option(PREPATH_BUILD_LIBRARIES "Build libraries like glad, glfw, imgui, spdlog and glm" ON)
set(PREPATH_GLAD_NAME "glad" CACHE STRING "Name of the glad target to link against")
set(PREPATH_IMGUI_NAME "imgui" CACHE STRING "Name of the imgui target to link against")
set(PREPATH_GLFW_NAME "glfw" CACHE STRING "Name of the glfw target to link against")
set(PREPATH_GLM_NAME "glm" CACHE STRING "Name of the glm target to link against")
set(PREPATH_SPDLOG_NAME "spdlog" CACHE STRING "Name of the spdlog target to link against")
set(PREPATH_ASSIMP_NAME "assimp" CACHE STRING "Name of the assimp target to link against")
set(PREPATH_LODEPNG_NAME "lodepng" CACHE STRING "Name of the lodepng target to link against")

# ---- Defaults ----
project(root LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ---- Runtime ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Runtime)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


# ---- Projects ----
if (PREPATH_BUILD_LIBRARIES)
    add_subdirectory(Vendor)
endif()

add_subdirectory(Prepath)

if (PREPATH_BUILD_EXAMPLES)
    add_subdirectory(Example)
endif()
