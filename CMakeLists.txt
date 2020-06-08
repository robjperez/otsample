cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -stdlib=libc++")

project(otsample)

add_subdirectory(imgui/)
add_subdirectory(glfw/)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLEW REQUIRED glew)

pkg_check_modules(OPENTOK REQUIRED libopentok)

set(TARGET sample)

add_executable(${TARGET} main.cc renderer.cc)

target_link_libraries(${TARGET}
  imgui
  glfw
  ${GLEW_LIBRARIES}
  ${OPENTOK_LIBRARIES}
  )
