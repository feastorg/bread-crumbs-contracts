cmake_minimum_required(VERSION 3.14)
project(bread_crumbs_contracts VERSION 0.10.0 LANGUAGES C)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(BREAD_CONTRACTS_TOP_LEVEL ON)
else()
    set(BREAD_CONTRACTS_TOP_LEVEL OFF)
endif()

option(BREAD_CONTRACTS_BUILD_TESTS "Build bread-crumbs-contracts test targets" ${BREAD_CONTRACTS_TOP_LEVEL})

if(NOT TARGET crumbs AND NOT TARGET crumbs::crumbs)
    find_package(crumbs 0.16 CONFIG REQUIRED)
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

    # One round-trip test per family: each SET through its wrapper and the
    # Slice-side unpack, each GET reply through the Slice-side pack and the
    # getter, with the bytes on the wire pinned.
    foreach(family rlht dcmt)
        add_executable(bread_contracts_${family}_roundtrip tests/payload_roundtrip/${family}_roundtrip.c)
        target_link_libraries(bread_contracts_${family}_roundtrip PRIVATE bread::contracts)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(bread_contracts_${family}_roundtrip PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
        if(BUILD_TESTING)
            add_test(NAME bread_contracts_${family}_roundtrip COMMAND bread_contracts_${family}_roundtrip)
        endif()
    endforeach()

    # The Slice firmware and the C++ controllers include these headers as
    # C++11; build the smoke test that way too when a C++ compiler exists.
    include(CheckLanguage)
    check_language(CXX)
    if(CMAKE_CXX_COMPILER)
        enable_language(CXX)
        add_executable(bread_contracts_compile_smoke_cxx tests/compile_smoke/smoke_cxx.cpp)
        target_link_libraries(bread_contracts_compile_smoke_cxx PRIVATE bread::contracts)
        set_target_properties(bread_contracts_compile_smoke_cxx PROPERTIES
            CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(bread_contracts_compile_smoke_cxx PRIVATE -Wall -Wextra -Werror)
        endif()
        if(BUILD_TESTING)
            add_test(NAME bread_contracts_compile_smoke_cxx COMMAND bread_contracts_compile_smoke_cxx)
        endif()
    endif()

    # BREAD_CHECK_FAMILY_OPS: a family opcode equal to any bread-common
    # opcode must fail to compile with the message on the source's first
    # line. The targets are built only by their tests, and a build that
    # fails for any other reason fails the test.
    file(STRINGS tests/compile_fail/common_opcode_clash.c common_opcode_clash_line LIMIT_COUNT 1)
    if(NOT common_opcode_clash_line MATCHES "^/\\* Must fail to compile with: (.+) \\*/$")
        message(FATAL_ERROR "tests/compile_fail/common_opcode_clash.c: first line must name the expected message")
    endif()
    set(common_opcode_clash_message "${CMAKE_MATCH_1}")
    foreach(opcode 0x7C 0x7D 0x7E 0x7F)
        add_library(compile_fail_common_opcode_${opcode} OBJECT EXCLUDE_FROM_ALL tests/compile_fail/common_opcode_clash.c)
        target_link_libraries(compile_fail_common_opcode_${opcode} PRIVATE bread::contracts)
        target_compile_definitions(compile_fail_common_opcode_${opcode} PRIVATE CLASH_OPCODE=${opcode})
        if(BUILD_TESTING)
            add_test(NAME bread_contracts_compile_fail_common_opcode_${opcode}
                     COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target compile_fail_common_opcode_${opcode})
            set_tests_properties(bread_contracts_compile_fail_common_opcode_${opcode} PROPERTIES
                PASS_REGULAR_EXPRESSION "${common_opcode_clash_message}"
                ENVIRONMENT "LC_ALL=C")
        endif()
    endforeach()

    # Writes tests/golden_vectors/vectors.json from the real headers; CI
    # regenerates the file and fails on any difference.
    add_executable(bread_contracts_gen_vectors tests/golden_vectors/gen_vectors.c)
    target_link_libraries(bread_contracts_gen_vectors PRIVATE bread::contracts)
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
