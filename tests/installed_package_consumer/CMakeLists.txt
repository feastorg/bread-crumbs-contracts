cmake_minimum_required(VERSION 3.14)
project(bread_contracts_installed_package_consumer C)

find_package(breadCrumbsContracts 0.4 CONFIG REQUIRED)

add_executable(bread_contracts_installed_package_consumer main.c)
target_link_libraries(bread_contracts_installed_package_consumer PRIVATE bread::contracts)

enable_testing()
add_test(
    NAME bread_contracts_installed_package_consumer
    COMMAND bread_contracts_installed_package_consumer
)
