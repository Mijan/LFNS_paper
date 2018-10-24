# - Find DP-GMM
# Find the native DP-GMM includes and library
#
#  DPGMM_INCLUDE_DIR    - where to find flint.h
#  DPGMM_LIBRARIES   - List of libraries when using FLINT.
#  DPGMM_FOUND       - True if FLINT found.

include(FindPackageHandleStandardArgs)

if (DPGMM_INCLUDE_DIR)
    # Already in cache, be silent
    set(DPGMM_FIND_QUIETLY TRUE)
endif (DPGMM_INCLUDE_DIR)

find_path(DPGMM_INCLUDE_DIR
        NAMES DPGMMEstimator.h
        HINTS /usr/local/
        ENV CPATH
        ENV MUPARSER_ROOT
        ENV DPGMM_ROOT
        PATH_SUFFIXES include
        DOC "Directory containing DPGMM header files"
        )

find_library(DPGMM_LIBRARIES NAMES libDPGMM.so
        HINTS /usr/local
        ENV DPGMM_ROOT
        ENV LD_LIBRARY_PATH
        ENV LIBRARY_PATH
        PATH_SUFFIXES lib
        DOC "Path to DPGMM library"
        )

# handle the QUIETLY and REQUIRED arguments and set FLINT_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(DPGMM "DEFAULT_MSG" DPGMM_LIBRARIES DPGMM_INCLUDE_DIR)

mark_as_advanced(DPGMM_LIBRARIES DPGMM_INCLUDE_DIR)

