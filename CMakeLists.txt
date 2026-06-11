cmake_minimum_required(VERSION 3.14)
project(bread_crumbs_contracts VERSION 0.4.3 LANGUAGES C)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(BREAD_CONTRACTS_TOP_LEVEL ON)
else()
    set(BREAD_CONTRACTS_TOP_LEVEL OFF)
endif()

option(BREAD_CONTRACTS_BUILD_TESTS "Build bread-crumbs-contracts test targets" ${BREAD_CONTRACTS_TOP_LEVEL})

if(NOT TARGET crumbs AND NOT TARGET crumbs::crumbs)
    find_package(crumbs CONFIG REQUIRED)
endif()

add_library(bread_crumbs_contracts INTERFACE)
add_library(bread::contracts ALIAS bread_crumbs_contracts)

target_include_directories(bread_crumbs_contracts INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(bread_crumbs_contracts INTERFACE crumbs::crumbs)

if(BREAD_CONTRACTS_BUILD_TESTS)
    include(CTest)

    add_executable(bread_contracts_compile_smoke tests/compile_smoke/smoke.c)
    target_link_libraries(bread_contracts_compile_smoke PRIVATE bread::contracts)

    if(BUILD_TESTING)
        add_test(NAME bread_contracts_compile_smoke COMMAND bread_contracts_compile_smoke)
    endif()
endif()

install(DIRECTORY include/ DESTINATION include)
install(TARGETS bread_crumbs_contracts EXPORT breadCrumbsContractsTargets)

install(EXPORT breadCrumbsContractsTargets
    NAMESPACE bread::
    DESTINATION lib/cmake/breadCrumbsContracts
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/breadCrumbsContractsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/breadCrumbsContractsConfig.cmake
    INSTALL_DESTINATION lib/cmake/breadCrumbsContracts
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/breadCrumbsContractsConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/breadCrumbsContractsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/breadCrumbsContractsConfigVersion.cmake
    DESTINATION lib/cmake/breadCrumbsContracts
)
