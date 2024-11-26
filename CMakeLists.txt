cmake_minimum_required(VERSION 3.15)
project(abstractions CXX)




add_library(abstractions src/abstractions.cpp)
target_include_directories(abstractions PUBLIC include)



set_target_properties(abstractions PROPERTIES PUBLIC_HEADER "include/abstractions.h")
install(TARGETS abstractions)
