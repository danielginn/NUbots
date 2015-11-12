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

    FOREACH(header ${headers})

        # Get the absolute file, and the file without the extension
        GET_FILENAME_COMPONENT(abs_file ${header} ABSOLUTE)
        GET_FILENAME_COMPONENT(file_we ${header} NAME_WE)

        # Calculate the Output Directory
        FILE(RELATIVE_PATH relpath ${CMAKE_CURRENT_SOURCE_DIR} ${header})
        GET_FILENAME_COMPONENT(outputpath ${relpath} PATH)
        SET(outputpath "${CMAKE_CURRENT_BINARY_DIR}/${outputpath}")

        # If we have a .i file for this we can use it instead

        # Otherwise just generate based off the header

        #ADD_CUSTOM_COMMAND(
        #    OUTPUT "${outputpath}/${file_we}_wrap.cpp"
        #    COMMAND ${SWIG_EXECUTABLE}
        #    ARGS -c++ -python -builtin -importall -module "messages" -o "${outputpath}/${file_we}_wrap.cpp" "-I${CMAKE_SOURCE_DIR}/shared" "${header}"
        #    DEPENDS "${header}"
        #)
    ENDFOREACH(header)

    # Build a library from these files
    ADD_LIBRARY(swig_wrapper ${src})

    # Put it in an IDE group for shared
    SET_PROPERTY(TARGET utility PROPERTY FOLDER "shared/")

ENDIF(SWIG_FOUND)
