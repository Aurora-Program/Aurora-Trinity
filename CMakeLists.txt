cmake_minimum_required(VERSION 3.16)
project(Trinity VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(trinity
    src/domain.c
    src/trigate.c
    src/ordering.c
    src/network.c
    src/operation.c
    src/so.c
    src/tensor.c
    src/window.c
    src/sequence.c
    src/executive.c
    src/dictionary.c
    src/translation.c
    src/bootstrap.c
    src/persistence.c
    src/circular.c
)
add_library(Trinity::trinity ALIAS trinity)
add_library(aurora_core ALIAS trinity)
set_target_properties(trinity PROPERTIES
    OUTPUT_NAME trinity
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_include_directories(trinity PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(MSVC)
    target_compile_options(trinity PRIVATE /W4)
else()
    target_compile_options(trinity PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(genesis
    genesis/src/tensor117.c
    genesis/src/random_origin.c
    genesis/src/benchmark.c
    genesis/src/education.c
    genesis/src/curriculum.c
    genesis/src/ablation.c
    genesis/src/self_organization.c
)
add_library(Trinity::genesis ALIAS genesis)
set_target_properties(genesis PROPERTIES OUTPUT_NAME genesis)
target_link_libraries(genesis PUBLIC Trinity::trinity)
target_include_directories(genesis PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/genesis/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if(MSVC)
    target_compile_options(genesis PRIVATE /W4)
else()
    target_compile_options(genesis PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS trinity genesis EXPORT TrinityTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/aurora include/trinity
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY genesis/include/genesis
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT TrinityTargets
    FILE TrinityTargets.cmake
    NAMESPACE Trinity::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Trinity
)
configure_package_config_file(
    cmake/TrinityConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/TrinityConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Trinity
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/TrinityConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/TrinityConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/TrinityConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Trinity
)

include(CTest)
add_executable(aurora_circular_census examples/circular_census.c)
target_link_libraries(aurora_circular_census PRIVATE Trinity::trinity)
if(BUILD_TESTING)
    add_executable(aurora_tests tests/test_aurora.c)
    target_link_libraries(aurora_tests PRIVATE Trinity::trinity)
    if(MSVC)
        target_compile_options(aurora_tests PRIVATE /W4)
    else()
        target_compile_options(aurora_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME trinity_exhaustive COMMAND aurora_tests)

    add_executable(genesis_tests genesis/tests/test_genesis.c)
    target_link_libraries(genesis_tests PRIVATE Trinity::genesis)
    if(MSVC)
        target_compile_options(genesis_tests PRIVATE /W4)
    else()
        target_compile_options(genesis_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME genesis_random_origin COMMAND genesis_tests)

    add_executable(genesis_education_tests genesis/tests/test_education.c)
    target_link_libraries(genesis_education_tests PRIVATE Trinity::genesis)
    add_test(NAME genesis_education COMMAND genesis_education_tests)

    add_executable(genesis_gr_context genesis/examples/gr_context.c)
    target_link_libraries(genesis_gr_context PRIVATE Trinity::genesis)

    add_executable(genesis_first_lesson genesis/examples/first_lesson.c)
    target_link_libraries(genesis_first_lesson PRIVATE Trinity::genesis)

    add_executable(genesis_educate_baby genesis/examples/educate_baby.c)
    target_link_libraries(genesis_educate_baby PRIVATE Trinity::genesis)

    add_executable(genesis_ablation genesis/examples/ablation.c)
    target_link_libraries(genesis_ablation PRIVATE Trinity::genesis)

    add_executable(genesis_self_organization
                   genesis/examples/self_organization.c)
    target_link_libraries(genesis_self_organization PRIVATE Trinity::genesis)
endif()
