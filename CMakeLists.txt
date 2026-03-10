cmake_minimum_required(VERSION 3.14)
project(bread_crumbs_contracts LANGUAGES C)

add_library(bread_crumbs_contracts INTERFACE)
add_library(bread::contracts ALIAS bread_crumbs_contracts)

target_include_directories(bread_crumbs_contracts INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

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
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/breadCrumbsContractsConfig.cmake
    DESTINATION lib/cmake/breadCrumbsContracts
)
