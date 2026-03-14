cmake_minimum_required(VERSION 3.13)
project(bread_discovery_controller C)

option(CRUMBS_BUILD_IN_TREE "Add CRUMBS as a subdirectory" ON)
if(NOT DEFINED CRUMBS_PATH)
    set(CRUMBS_PATH ${CMAKE_SOURCE_DIR}/../../../CRUMBS)
endif()

if(CRUMBS_BUILD_IN_TREE)
    set(CRUMBS_ENABLE_LINUX_HAL ON CACHE BOOL "" FORCE)
    add_subdirectory(${CRUMBS_PATH} ${CMAKE_BINARY_DIR}/crumbs_subbuild)

    add_executable(controller_discovery main.c)
    target_link_libraries(controller_discovery PRIVATE crumbs)
    target_include_directories(controller_discovery PRIVATE
        ${CRUMBS_PATH}/src
        ${CMAKE_SOURCE_DIR}/../../include
    )
else()
    find_package(crumbs CONFIG REQUIRED)
    add_executable(controller_discovery main.c)
    target_link_libraries(controller_discovery PRIVATE crumbs::crumbs)
    target_include_directories(controller_discovery PRIVATE
        ${CMAKE_SOURCE_DIR}/../../include
    )
endif()

install(TARGETS controller_discovery DESTINATION bin)
