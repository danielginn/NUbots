# Build our messages (including protocol buffers)
ADD_SUBDIRECTORY(messages)

# This will build our Utilities library (common functionality)
ADD_SUBDIRECTORY(utility)

# Build our SWIG wrappers for the shared and utility classes
FIND_PACKAGE(SWIG)
IF(SWIG_FOUND)
    FIND_PACKAGE(PythonInterp)
    FIND_PACKAGE(PythonLibs)

    # We need our python headers
    INCLUDE_DIRECTORIES(SYSTEM ${PYTHON_INCLUDE_DIRS})

    # This gets all of our headers for utilities and messages
    FILE(GLOB_RECURSE headers "**.h" "**.hpp")

    FOREACH(header ${headers})

        # Get the file without the extension
        GET_FILENAME_COMPONENT(file_we ${header} NAME_WE)

        # Calculate the Output Directory
        FILE(RELATIVE_PATH relative_file ${CMAKE_CURRENT_SOURCE_DIR} ${header})
        GET_FILENAME_COMPONENT(relative_dir ${relative_file} PATH)
        SET(output_path "${CMAKE_CURRENT_BINARY_DIR}/${relative_dir}")

        # If we have a .i file here we can use that instead of generating one from the header
        IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${relative_dir}/${file_we}.i")
            # Copy the interface file over
            ADD_CUSTOM_COMMAND(
                OUTPUT "${output_path}/${file_we}.i"
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${relative_dir}/${file_we}.i" "${output_path}/${file_we}.i"
                DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${relative_dir}/${file_we}.i"
                COMMENT "Copying SWIG interface file for ${file_we}"
            )
        ELSE()
            # Generate an interface file
            ADD_CUSTOM_COMMAND(
                OUTPUT "${output_path}/${file_we}.i"
                COMMAND ${PYTHON_EXECUTABLE}
                ARGS "${CMAKE_SOURCE_DIR}/tools/scripts/generate_swig.py" "${header}" "${output_path}/${file_we}.i"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                DEPENDS ${header} "${CMAKE_SOURCE_DIR}/tools/scripts/generate_swig.py"
                COMMENT "Generating the SWIG interface file for ${file_we}"
            )
        ENDIF()

        SET(interface_files ${interface_files} "${output_path}/${file_we}.i")
    ENDFOREACH(header)

    # Loop again, as we might require any one of the generated interface files
    FOREACH(interface_file ${interface_files})

        # Get our filename without an extension and our directory
        GET_FILENAME_COMPONENT(file_we ${interface_file} NAME_WE)
        GET_FILENAME_COMPONENT(outputpath ${interface_file} DIRECTORY)

        # Build our SWIG code for this
        ADD_CUSTOM_COMMAND(
            OUTPUT "${outputpath}/${file_we}_wrap.cpp"
            COMMAND ${SWIG_EXECUTABLE}
            ARGS -c++ -python -builtin -o "${outputpath}/${file_we}_wrap.cpp" "-I${CMAKE_SOURCE_DIR}/shared" "-I${CMAKE_BINARY_DIR}/shared" "${interface_file}"
            DEPENDS ${interface_files}
            COMMENT "Generating the SWIG wrapper code for ${file_we}"
        )

        # Add our output to our source
        SET(src ${src} "${outputpath}/${file_we}_wrap.cpp")

    ENDFOREACH(interface_file)

    # Build a library from these files
    ADD_LIBRARY(swig_wrapper ${src})

    # Put it in an IDE group for shared
    SET_PROPERTY(TARGET utility PROPERTY FOLDER "shared/")

ENDIF(SWIG_FOUND)
