cmake_minimum_required(VERSION 3.14)
project(bread_crumbs_contracts VERSION 0.4.1 LANGUAGES C)

find_package(crumbs CONFIG REQUIRED)

add_library(bread_crumbs_contracts INTERFACE)
add_library(bread::contracts ALIAS bread_crumbs_contracts)

target_include_directories(bread_crumbs_contracts INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(bread_crumbs_contracts INTERFACE crumbs::crumbs)

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
