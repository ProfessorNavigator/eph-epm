find_path(MGL_INCLUDE_DIR
    NAMES mgl.h
    PATH_SUFFIXES "mgl" "mgl2"
)

find_library(MGL_LIBRARY
    NAMES mgl mgl2
    REQUIRED
)

include(FindPackageHandleStandardArgs)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_file(MGL_DLL
        NAMES libmgl.dll libmgl2.dll
    )

    if(MGL_INCLUDE_DIR AND MGL_LIBRARY AND MGL_DLL)
        add_library(mgl SHARED IMPORTED GLOBAL)
        set_target_properties(mgl PROPERTIES
            IMPORTED_LOCATION "${MGL_DLL}"
            IMPORTED_IMPLIB "${MGL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${MGL_INCLUDE_DIR}"
        )
    endif()

    find_package_handle_standard_args(MGL
        FOUND_VAR MGL_FOUND
        REQUIRED_VARS MGL_INCLUDE_DIR MGL_LIBRARY MGL_DLL
    )
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(MGL_INCLUDE_DIR AND MGL_LIBRARY)
        add_library(mgl SHARED IMPORTED GLOBAL)
        set_target_properties(mgl PROPERTIES
            IMPORTED_IMPLIB "${MGL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${MGL_INCLUDE_DIR}"
        )
    endif()

    find_package_handle_standard_args(MGL
        FOUND_VAR MGL_FOUND
        REQUIRED_VARS MGL_INCLUDE_DIR MGL_LIBRARY
    )
endif()
