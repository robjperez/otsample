cmake_minimum_required(VERSION 3.0)
project(imgui)

file(GLOB imgui_src "*.h" "*.cpp")
add_library(imgui ${imgui_src})

target_include_directories(imgui PUBLIC .)
