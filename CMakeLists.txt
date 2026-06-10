cmake_minimum_required(VERSION 3.16)

project(FirewallDefense VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT SFML_DIR)
    if(EXISTS "${CMAKE_SOURCE_DIR}/libs/SFML-2.6.2/lib/cmake/SFML")
        set(SFML_DIR "${CMAKE_SOURCE_DIR}/libs/SFML-2.6.2/lib/cmake/SFML")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/SFML-2.6.2/lib/cmake/SFML")
        set(SFML_DIR "${CMAKE_SOURCE_DIR}/SFML-2.6.2/lib/cmake/SFML")
    else()
        message(FATAL_ERROR "nie ma sfmla")
    endif()
endif()

find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-graphics>
        $<TARGET_FILE:sfml-window>
        $<TARGET_FILE:sfml-system>
        $<TARGET_FILE:sfml-audio>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

include(GNUInstallDirs)
install(TARGETS FirewallDefense
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)