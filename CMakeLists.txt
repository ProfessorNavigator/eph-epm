cmake_minimum_required(VERSION 3.13)

project(EphemerisEPM VERSION 2.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

find_package(Intl REQUIRED)
find_package(Gettext)

pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-i18n)
pkg_check_modules(ICUUC REQUIRED IMPORTED_TARGET icu-uc)
pkg_check_modules(GMPXX REQUIRED IMPORTED_TARGET gmpxx)

find_package(MathGL2)

find_library(SOFA sofa_c REQUIRED)

find_package(OpenMP REQUIRED)
if(NOT OpenMP_CXX_FOUND)
  message(FATAL_ERROR "Cannot find OpenMP for C++")
endif()

add_executable(ephepm)

if(GTKMM_VERSION VERSION_LESS "4.10")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEPH_GTK_OLD")
endif()

add_subdirectory(include)
add_subdirectory(src)

target_compile_options(ephepm
  PRIVATE ${GTKMM_CFLAGS_OTHER}
  PRIVATE ${ICU_CFLAGS_OTHER}
  PRIVATE ${ICUUC_CFLAGS_OTHER}
  PRIVATE ${GMPXX_CFLAGS_OTHER}
  PRIVATE ${OpenMP_CXX_FLAGS}
)

target_include_directories(ephepm
  PRIVATE include
  PRIVATE ${GTKMM_INCLUDE_DIRS}
  PRIVATE ${ICU_INCLUDE_DIRS}
  PRIVATE ${ICUUC_INCLUDE_DIRS}
  PRIVATE ${GMPXX_INCLUDE_DIRS}
  PRIVATE ${OpenMP_CXX_INCLUDE_DIRS}
)

target_link_options(ephepm
  PRIVATE ${GTKMM_LDFLAGS_OTHER}
  PRIVATE ${ICU_LDFLAGS_OTHER}
  PRIVATE ${ICUUC_LDFLAGS_OTHER}
  PRIVATE ${GMPXX_LDFLAGS_OTHER}
)

target_link_libraries(ephepm
  PRIVATE ${GTKMM_LINK_LIBRARIES}
  PRIVATE ${ICU_LINK_LIBRARIES}
  PRIVATE ${ICUUC_LINK_LIBRARIES}
  PRIVATE ${GMPXX_LINK_LIBRARIES}
  PRIVATE ${SOFA}
  PRIVATE ${OpenMP_CXX_LIBRARIES}
)

if(MathGL2_FOUND)
  message(NOTICE "-- MathGL2 package found")
  target_link_libraries(ephepm PRIVATE mgl)
else()
  message(NOTICE "-- MathGL2 package not found")
  find_library(MGL mgl2)
  if(NOT MGL)
    find_library(MGL mgl REQUIRED)
  endif()
endif()

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


