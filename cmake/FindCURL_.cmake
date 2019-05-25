
find_package(CURL REQUIRED)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CURL_
        REQUIRED_VARS CURL_INCLUDE_DIR CURL_LIBRARY)

if (CURL_FOUND)

    if(NOT TARGET CURL::libcurl)
        add_library(CURL::libcurl UNKNOWN IMPORTED)
        set_target_properties(CURL::libcurl PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")

        if(EXISTS "${CURL_LIBRARY}")
            set_target_properties(CURL::libcurl PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                    IMPORTED_LOCATION "${CURL_LIBRARY}")
        endif()
        if(CURL_LIBRARY_RELEASE)
            set_property(TARGET CURL::libcurl APPEND PROPERTY
                    IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(CURL::libcurl PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                    IMPORTED_LOCATION_RELEASE "${CURL_LIBRARY_RELEASE}")
        endif()
        if(CURL_LIBRARY_DEBUG)
            set_property(TARGET CURL::libcurl APPEND PROPERTY
                    IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(CURL::libcurl PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                    IMPORTED_LOCATION_DEBUG "${CURL_LIBRARY_DEBUG}")
        endif()
    endif()


endif()