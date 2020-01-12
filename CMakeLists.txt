cmake_minimum_required(VERSION 3.16)
Project(BackgroundSaver C CXX)
find_package(PkgConfig REQUIRED)

set(SAVER_NAME saver_bg)

pkg_check_modules(x11 REQUIRED IMPORTED_TARGET x11)
pkg_check_modules(jpeglib REQUIRED IMPORTED_TARGET libjpeg)

add_executable(${SAVER_NAME} saver.c)
target_link_libraries(${SAVER_NAME} PkgConfig::x11 PkgConfig::jpeglib)
