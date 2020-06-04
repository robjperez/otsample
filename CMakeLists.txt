cmake_minimum_required(VERSION 3.0)

project(otsample)

add_subdirectory(imgui/)
add_subdirectory(glfw/)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLEW REQUIRED glew)

set(TARGET sample)

add_executable(${TARGET} main.cc)
message(STATUS ${GLEW_LIBRARIES})
target_link_libraries(${TARGET}
  imgui
  glfw
  ${GLEW_LIBRARIES}
  )
