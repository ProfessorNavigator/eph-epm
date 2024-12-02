cmake_minimum_required(VERSION 3.13)

project(EphemerisEPM VERSION 3.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(USE_OPENMP "Build with openmp support" OFF)

find_package(PkgConfig REQUIRED)

find_package(Intl REQUIRED)
find_package(Gettext)

pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-i18n)
pkg_check_modules(GMPXX REQUIRED IMPORTED_TARGET gmpxx)

find_library(MGL mgl2 REQUIRED)

find_library(SOFA sofa_c REQUIRED)

if(USE_OPENMP)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENMP -fopenmp")
  add_link_options("-fopenmp")
endif()

add_executable(ephepm)

add_subdirectory(src)

target_include_directories(ephepm PRIVATE include)

target_link_libraries(ephepm PUBLIC
  PkgConfig::GTKMM
  PkgConfig::ICU
  PkgConfig::GMPXX
  ${MGL}
  ${SOFA})

include(GNUInstallDirs)

install(TARGETS ephepm
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(DIRECTORY EphEPM DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}")

if(CMAKE_SYSTEM_NAME MATCHES "Linux")

    install(FILES
    ru.mail.bobilev_yury.EphEPM.desktop DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")

    install(FILES
    ephepm.png DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps")

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")

    install(FILES
    ephepm.ico DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps")

endif()

if(Gettext_FOUND)
   GETTEXT_PROCESS_PO_FILES("ru" ALL
   INSTALL_DESTINATION "${CMAKE_INSTALL_LOCALEDIR}"
   PO_FILES po/ephepm.po
)
endif()

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


