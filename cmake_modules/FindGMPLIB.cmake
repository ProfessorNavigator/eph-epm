find_path(GMPXXLIB_INCLUDE_DIR
    NAMES gmpxx.h
)

find_path(GMPCLIB_INCLUDE_DIR
    NAMES gmp.h
)

find_library(GMPXXLIB_LIBRARY
    NAMES gmpxx
    REQUIRED
)

find_library(GMPCLIB_LIBRARY
    NAMES gmp
    REQUIRED
)

include(FindPackageHandleStandardArgs)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_file(GMPXXLIB_DLL
        NAMES libgmpxx.dll
    )

    find_file(GMPCLIB_DLL
        NAMES libgmp.dll
    )

    if(GMPXXLIB_INCLUDE_DIR AND GMPXXLIB_LIBRARY AND GMPXXLIB_DLL)
        add_library(gmpxx SHARED IMPORTED GLOBAL)
        set_target_properties(gmpxx PROPERTIES
            IMPORTED_LOCATION "${GMPXXLIB_DLL}"
            IMPORTED_IMPLIB "${GMPXXLIB_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GMPXXLIB_INCLUDE_DIR}"
        )
    endif()

    if(GMPCLIB_INCLUDE_DIR AND GMPCLIB_LIBRARY AND GMPCLIB_DLL)
        add_library(gmp SHARED IMPORTED GLOBAL)
        set_target_properties(gmp PROPERTIES
            IMPORTED_LOCATION "${GMPCLIB_DLL}"
            IMPORTED_IMPLIB "${GMPCLIB_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GMPCLIB_INCLUDE_DIR}"
        )
    endif()

    find_package_handle_standard_args(GMPLIB
        FOUND_VAR GMPLIB_FOUND
        REQUIRED_VARS GMPXXLIB_INCLUDE_DIR GMPXXLIB_LIBRARY GMPXXLIB_DLL
            GMPCLIB_INCLUDE_DIR GMPCLIB_LIBRARY GMPCLIB_DLL
    )
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(GMPXXLIB_INCLUDE_DIR AND GMPXXLIB_LIBRARY)
        add_library(gmpxx SHARED IMPORTED GLOBAL)
        set_target_properties(gmpxx PROPERTIES
            IMPORTED_IMPLIB "${GMPXXLIB_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GMPXXLIB_INCLUDE_DIR}"
        )
    endif()

    if(GMPCLIB_INCLUDE_DIR AND GMPCLIB_LIBRARY)
        add_library(gmp SHARED IMPORTED GLOBAL)
        set_target_properties(gmp PROPERTIES
            IMPORTED_IMPLIB "${GMPCLIB_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GMPCLIB_INCLUDE_DIR}"
        )
    endif()

    find_package_handle_standard_args(GMPLIB
        FOUND_VAR GMPLIB_FOUND
        REQUIRED_VARS GMPXXLIB_INCLUDE_DIR GMPXXLIB_LIBRARY
            GMPCLIB_INCLUDE_DIR GMPCLIB_LIBRARY
    )
endif()
