cmake_minimum_required(VERSION 3.13)

project(EphemerisEPM VERSION 2.3.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

find_package(Intl REQUIRED)
find_package(Gettext)

pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_check_modules(GMPXX IMPORTED_TARGET gmpxx)

if(NOT GMPXX_FOUND)
    find_library(GMPXXlib NAMES gmpxx REQUIRED)
    find_library(GMPlib NAMES gmp REQUIRED)
endif()

find_package(MathGL2)

if(NOT MathGL2_FOUND)
    find_library(MGL mgl2)
    if(NOT MGL)
        find_library(MGL mgl REQUIRED)
    endif()
endif()

find_library(SOFA sofa_c REQUIRED)

find_package(OpenMP REQUIRED)

try_compile(OMP_TEST_NESTED "${CMAKE_BINARY_DIR}/omp_test_nested"
    "${PROJECT_SOURCE_DIR}/omp_test_nested" OmpTestNested
)

find_package(Threads QUIET)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_executable(ephepm)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_executable(ephepm WIN32)
endif()

add_subdirectory(include)
add_subdirectory(src)

target_include_directories(ephepm
  PRIVATE include
)

target_link_libraries(ephepm
  PRIVATE PkgConfig::GTKMM
  PRIVATE ${SOFA}  
  PRIVATE OpenMP::OpenMP_CXX
)

target_link_options(ephepm
  PRIVATE ${OpenMP_CXX_FLAGS}
)

if(MathGL2_FOUND)
    target_include_directories(ephepm
        PRIVATE ${MathGL2_INCLUDE_DIRS}
    )
    target_link_libraries(ephepm
        PRIVATE mgl
    )
else()
    target_link_libraries(ephepm
        PRIVATE ${MGL}
    )
endif()

if(GTKMM_VERSION VERSION_LESS "4.10")
    target_compile_options(ephepm
        PRIVATE -DEPH_GTK_OLD
    )
endif()

if(GMPXX_FOUND)
    target_link_libraries(ephepm
      PRIVATE PkgConfig::GMPXX
    )
else()
    target_link_libraries(ephepm
        PRIVATE ${GMPXXlib}
        PRIVATE ${GMPlib}
    )
endif()

if(Threads_FOUND)
  target_link_libraries(ephepm PRIVATE Threads::Threads)
endif()

if(NOT OMP_TEST_NESTED)
    target_compile_options(ephepm
        PRIVATE -DOLD_OPENMP
    )
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


