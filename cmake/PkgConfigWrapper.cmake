find_package(PkgConfig)

function(PkgConfig_Find_Module module_name)
    pkg_check_modules(${module_name}_TO_FIND REQUIRED ${module_name})

    if (${ARGC} EQUAL "1")
        set(target_name ${module_name})
    elseif(${ARGC} EQUAL "2")
        set(target_name ${ARGV1})
    endif()

    list(LENGTH ${module_name}_TO_FIND_LIBRARY_DIRS include_dir_length)

    if (${include_dir_length} GREATER 2)
        message(STATUS "We've encountered a pkg-config library (${module_name}) with more than a debug and release include directory.")
        #foreach(dir ${${module_name}_TO_FIND_LIBRARY_DIRS})
        #    message(STATUS "\t${dir}")
        #endforeach()
    endif()

    # STATIC should be variable here, should detect it somehow.
    add_library(PkgConfigWrapper::${target_name} INTERFACE IMPORTED)
    

    set(${module_name}_TO_FIND_LIBRARY_DIRS_DEBUG ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib)
    set(${module_name}_TO_FIND_LIBRARY_DIRS_RELEASE ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)

    foreach(library ${${module_name}_TO_FIND_LIBRARIES})
        if (TARGET PkgConfigWrapperSubLib::${library})
            continue()
        endif()

        # Get the release library
        unset(${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND)
        find_library(${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND NAMES ${library} PATHS ${${module_name}_TO_FIND_LIBRARY_DIRS_RELEASE} NO_CACHE NO_DEFAULT_PATH)

        # Get the debug library
        unset(${module_name}_TO_FIND_${library}_DEBUG_LIBRARY_FOUND)
        find_library(${module_name}_TO_FIND_${library}_DEBUG_LIBRARY_FOUND NAMES ${library} PATHS ${${module_name}_TO_FIND_LIBRARY_DIRS_DEBUG} NO_CACHE NO_DEFAULT_PATH)
        
        if (${${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND} STREQUAL ${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND-NOTFOUND)
            message(STATUS "Giving up and just linking ${library}")
            set_property(TARGET PkgConfigWrapper::${target_name} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${library})
        else()
            add_library(PkgConfigWrapperSubLib::${library} UNKNOWN IMPORTED)
            set_target_properties(PkgConfigWrapperSubLib::${library} PROPERTIES
                IMPORTED_LOCATION ${${module_name}_TO_FIND_${library}_RELEASE_LIBRARY_FOUND}
                IMPORTED_LOCATION_DEBUG ${${module_name}_TO_FIND_${library}_DEBUG_LIBRARY_FOUND}
            )
        
            set_property(TARGET PkgConfigWrapper::${target_name} APPEND PROPERTY INTERFACE_LINK_LIBRARIES PkgConfigWrapperSubLib::${library})
        endif()
    endforeach()

    foreach(include_dir ${${module_name}_TO_FIND_INCLUDE_DIRS})
        if (EXISTS ${include_dir})
            set_property(TARGET PkgConfigWrapper::${target_name} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${include_dir})
        endif()
    endforeach()
    
    foreach(cflag ${${module_name}_TO_FIND_CFLAGS_OTHER})
        set_property(TARGET PkgConfigWrapper::${target_name} APPEND PROPERTY INTERFACE_COMPILE_OPTIONS ${cflag})
    endforeach()
endfunction()