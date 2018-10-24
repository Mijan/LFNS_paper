# - Find DP-GMM
# Find the native DP-GMM includes and library
#
#  MUPARSER_INCLUDE_DIR    - where to find muParser.h
#  MUPARSER_LIBRARIES   - List of libraries when using muParser.
#  MUPARSER_FOUND       - True if FLINT found.

include(FindPackageHandleStandardArgs)

if (MUPARSER_INCLUDE_DIR)
    # Already in cache, be silent
    set(muparser_FIND_QUIETLY TRUE)
endif (MUPARSER_INCLUDE_DIR)

find_path(MUPARSER_INCLUDE_DIR
        NAMES muParser.h
        HINTS ENV CPATH
        ENV MUPARSER_ROOT
        PATH_SUFFIXES include
        DOC "Directory containing muparser header files"
        )

find_library(MUPARSER_LIBRARIES NAMES libmuparser.so
        HINTS ENV LD_LIBRARY_PATH
        ENV LIBRARY_PATH
        ENV MUPARSER_ROOT
        PATH_SUFFIXES lib
        DOC "Path to muparser library"
        )

# handle the QUIETLY and REQUIRED arguments and set FLINT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(MUPARSER "DEFAULT_MSG" MUPARSER_LIBRARIES MUPARSER_INCLUDE_DIR)

mark_as_advanced(MUPARSER_LIBRARIES MUPARSER_INCLUDE_DIR)

