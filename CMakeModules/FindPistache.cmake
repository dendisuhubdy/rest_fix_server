# - Check for the presence of PISTACHE
#
# The following variables are set when PISTACHE is found:
#  PISTACHE_FOUND      = Set to true, if all components of PISTACHE have been
#                         found.
#  PISTACHE_INCLUDES   = Include path for the header files of PISTACHE
#  PISTACHE_LIBRARIES  = Link these to use PISTACHE
#  PISTACHE_LFLAGS     = Linker flags (optional)

if (NOT PISTACHE_FOUND)

    if (NOT PISTACHE_ROOT_DIR)
        set (PISTACHE_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
    endif (NOT PISTACHE_ROOT_DIR)

    ##____________________________________________________________________________
    ## Check for the header files

    find_path (PISTACHE_INCLUDES
            NAMES pistache.h
            HINTS ${PISTACHE_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
            PATH_SUFFIXES include
            )

    ##____________________________________________________________________________
    ## Actions taken when all components have been found

    find_package_handle_standard_args (PISTACHE DEFAULT_MSG PISTACHE_INCLUDES)

    if (PISTACHE_FOUND)
        ## Update
        get_filename_component (PISTACHE_ROOT_DIR ${PISTACHE_INCLUDES} PATH)
        ## Feedback
        if (NOT PISTACHE_FIND_QUIETLY)
            message (STATUS "Found components for PISTACHE")
            message (STATUS "PISTACHE_ROOT_DIR  = ${PISTACHE_ROOT_DIR}")
            message (STATUS "PISTACHE_INCLUDES  = ${PISTACHE_INCLUDES}")
        endif (NOT PISTACHE_FIND_QUIETLY)
    else (PISTACHE_FOUND)
        if (PISTACHE_FIND_REQUIRED)
            message (FATAL_ERROR "Could not find PISTACHE!")
        endif (PISTACHE_FIND_REQUIRED)
    endif (PISTACHE_FOUND)

    ##____________________________________________________________________________
    ## Mark advanced variables

    mark_as_advanced (
            PISTACHE_ROOT_DIR
            PISTACHE_INCLUDES
    )

endif (NOT PISTACHE_FOUND)