cmake_minimum_required(VERSION 3.13)

project(EphemerisEPM VERSION 2.3 LANGUAGES CXX)

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

find_library(MGL mgl2)
if(NOT MGL)
  find_library(MGL mgl REQUIRED)
endif()

find_library(SOFA sofa_c REQUIRED)

find_package(OpenMP REQUIRED)
if(NOT OpenMP_CXX_FOUND)
  message(FATAL_ERROR "Cannot find OpenMP for C++")
else()
  try_compile(OMP_TEST "${CMAKE_BINARY_DIR}/omp_test_nested" "${PROJECT_SOURCE_DIR}/omp_test_nested" OmpTestNested)
  if(NOT OMP_TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEPH_OPENMP_OLD")
  endif()
  try_compile(OMP_TASK_TEST "${CMAKE_BINARY_DIR}/omp_task_test" "${PROJECT_SOURCE_DIR}/omp_task_test" OmpTaskTest)
  if(OMP_TASK_TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEPH_OMP_TASK")
  else()
    find_package(Threads QUIET)
  endif()
endif()

add_executable(ephepm)

if(GTKMM_VERSION VERSION_LESS "4.10")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEPH_GTK_OLD")
endif()

add_subdirectory(include)
add_subdirectory(src)

target_compile_options(ephepm
  PRIVATE ${GTKMM_CFLAGS_OTHER}
  PRIVATE ${OpenMP_CXX_FLAGS}
)

target_include_directories(ephepm
  PRIVATE include
  PRIVATE ${GTKMM_INCLUDE_DIRS}
  PRIVATE ${OpenMP_CXX_INCLUDE_DIRS}
)

target_link_options(ephepm
  PRIVATE ${GTKMM_LDFLAGS_OTHER}
)

target_link_libraries(ephepm
  PRIVATE ${GTKMM_LINK_LIBRARIES}
  PRIVATE ${SOFA}
  PRIVATE ${MGL}
  PRIVATE ${OpenMP_CXX_LIBRARIES}
)

if(GMPXX_FOUND)
    target_compile_options(ephepm
        PRIVATE ${GMPXX_CFLAGS_OTHER}
    )

    target_include_directories(ephepm
        PRIVATE ${GMPXX_INCLUDE_DIRS}
    )

    target_link_options(ephepm
        PRIVATE ${GMPXX_LDFLAGS_OTHER}
    )

    target_link_libraries(ephepm
      PRIVATE ${GMPXX_LINK_LIBRARIES}
    )
else()
    target_link_libraries(ephepm
        PRIVATE ${GMPXXlib}
        PRIVATE ${GMPlib}
    )
endif()

if(Threads_FOUND)
  target_link_libraries(ephepm PRIVATE ${CMAKE_THREAD_LIBS_INIT})
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


