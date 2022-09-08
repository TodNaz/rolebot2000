# RoleBot CMake configuration

cmake_minimum_required(VERSION 3.13)
project(rolebot)

include(FindPkgConfig)
pkg_check_modules(CURLPP REQUIRED curlpp)

link_directories(/usr/lib)

add_executable(rolebot
    main.cpp
        iSerealize.cc iSerealize.hh)

target_link_libraries(rolebot dpp)
target_link_libraries(rolebot ${CURLPP_LDFLAGS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)