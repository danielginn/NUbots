# We use NUClearModule
INCLUDE(NUClearModule)

# Remove any duplicate modules (used in more then one role)
SET(modules ${NUCLEAR_MODULES})
LIST(REMOVE_DUPLICATES modules)
SET(NUCLEAR_MODULES ${modules} CACHE INTERNAL "A list of the modules in use by the system" FORCE)

# Build each of our modules and their corresponding tests
FOREACH(module ${NUCLEAR_MODULES})
    STRING(REPLACE "::" "/" module_path "${module}")
    STRING(REPLACE "::" "" module_name "${module}")

    # Add the subdirectories
    ADD_SUBDIRECTORY("${NUCLEAR_MODULE_DIR}/${module_path}" "${CMAKE_CURRENT_BINARY_DIR}/${module_path}")
ENDFOREACH()
