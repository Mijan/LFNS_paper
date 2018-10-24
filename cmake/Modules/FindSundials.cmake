# - Find Sundials
# Find the native Sundials includes and library
#
#  SUNDIALS_INCLUDE_DIR    - where to find flint.h
#  SUNDIALS_LIBRARIES   - List of libraries when using FLINT.
#  SUNDIALS_FOUND       - True if FLINT found.

include(FindPackageHandleStandardArgs)

if (Sundials_INCLUDE_DIR)
    # Already in cache, be silent
    set(SUNDIALS_FIND_QUIETLY TRUE)
endif (Sundials_INCLUDE_DIR)


macro(_Sundials_FIND_LIBRARY var)
    find_library(${var} ${ARGN})

    # If we found the first library save Sundials_LIBRARY_DIR.
    if (${var} AND NOT Sundials_LIBRARY_DIR)
        get_filename_component(_dir "${${var}}" PATH)
        set(SUNDIALS_LIBRARY_DIR "${_dir}" CACHE PATH "Sundials library directory" FORCE)
    endif ()

    # If Sundials_LIBRARY_DIR is known then search only there.
    if (Sundials_LIBRARY_DIR)
        set(_SUNDIALS_LIBRARY_SEARCH_DIRS ${Sundials_LIBRARY_DIR} NO_DEFAULT_PATH)
    endif ()
endmacro()

find_path(SUNDIALS_INCLUDE_DIR
        NAMES sundials/sundials_math.h
        HINTS ENV SUNDIALS_ROOT
        ENV LD_LIBRARY_PATH
        ENV LIBRARY_PATH
        ENV CPATH
        PATH_SUFFIXES include
        DOC "Directory containing Sundials header files"
        )

foreach (COMPONENT ${Sundials_FIND_COMPONENTS})
    string(TOUPPER ${COMPONENT} UPPERCOMPONENT)
    _Sundials_FIND_LIBRARY(SUNDIALS_${UPPERCOMPONENT}_LIBRARY
            NAMES libsundials_${COMPONENT}.so
            HINTS ENV SUNDIALS_ROOT
            ENV LD_LIBRARY_PATH
            PATH_SUFFIXES lib
            DOC "Path to Sundials component library"
            )
    list(APPEND SUNDIALS_LIBRARIES ${SUNDIALS_${UPPERCOMPONENT}_LIBRARY})
endforeach ()

# handle the QUIETLY and REQUIRED arguments and set FLINT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(SUNDIALS "DEFAULT_MSG" SUNDIALS_LIBRARIES SUNDIALS_INCLUDE_DIR)

mark_as_advanced(DPGMM_LIBRARIES DPGMM_INCLUDE_DIR)

