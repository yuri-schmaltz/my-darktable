# - Try to find GTK 4
# Once done, this will define
#
#  GTK4_FOUND - system has GTK 4.
#  GTK4_INCLUDE_DIRS - the GTK 4 include directories
#  GTK4_LIBRARIES - link these to use GTK 4.
#
find_package(PkgConfig)
pkg_check_modules(GTK4 gtk4)

# Mac needs library names to include full path
if(APPLE)
	foreach(i ${GTK4_LIBRARIES})
		find_library(_gtk4_LIBRARY NAMES ${i} HINTS ${GTK4_LIBRARY_DIRS})
		LIST(APPEND GTK4_LIBRARY ${_gtk4_LIBRARY})
		unset(_gtk4_LIBRARY CACHE)
	endforeach(i)
	set(GTK4_LIBRARIES ${GTK4_LIBRARY})
	unset(GTK4_LIBRARY CACHE)
endif(APPLE)

set(VERSION_OK TRUE)
if (GTK4_VERSION)
    if (GTK4_FIND_VERSION_EXACT)
        if (NOT("${GTK4_FIND_VERSION}" VERSION_EQUAL "${GTK4_VERSION}"))
            set(VERSION_OK FALSE)
        endif ()
    else ()
        if ("${GTK4_VERSION}" VERSION_LESS "${GTK4_FIND_VERSION}")
            set(VERSION_OK FALSE)
        endif ()
    endif ()
endif ()
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK4 DEFAULT_MSG GTK4_INCLUDE_DIRS GTK4_LIBRARIES VERSION_OK)
