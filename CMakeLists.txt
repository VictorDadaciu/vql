cmake_minimum_required(VERSION 3.21..4.4)

project(
    vql
    VERSION 0.0.1
    DESCRIPTION "very quick library"
    LANGUAGES C CXX
)

include(GNUInstallDirs)

if (PROJECT_IS_TOP_LEVEL)
    message("vql is the top-level project")
    set(VQL_IS_TOP_LEVEL ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_library(vql INTERFACE
    vql/db.h
    vql/tuple.h
    vql/utils.h
    vql/detail/db.inl
    vql/detail/utils.inl
)

set_property(TARGET vql
    PROPERTY CXX_STANDARD 26
)

target_compile_options(vql
    INTERFACE
        -std=c++26
        -freflection
)

target_include_directories(vql
    INTERFACE
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/vql>
)

option(VQL_INSTALL "Install vql" ${VQL_IS_TOP_LEVEL})
if (${VQL_INSTALL})
    message("Configuring vql install...")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/vqlConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/vqlConfig.cmake
        COPYONLY
    )
    install(TARGETS vql
        EXPORT vqlTargets
    )
    install(EXPORT vqlTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/vql/cmake
        NAMESPACE vql::
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/vqlConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/vql/cmake
    )
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vql"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vql
    )
    export(TARGETS vql
        NAMESPACE vql::
        FILE vqlTargets.cmake
    )
endif()

if (${VQL_IS_TOP_LEVEL})
    message("Installing vql...")
    add_custom_target(
        vql_forced_install ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file
        COMMENT "Installing vql to ~/.local"
        COMMAND ./do_install.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

option(VQL_TESTS "Build tests" ON)
if (${VQL_TESTS})
    message("Configuring vql tests...")
    add_subdirectory(tests)
endif()

