# build_config.txt is the shared folder with BYAI
set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/build_config.txt")

# Check if configuration file exists and read it
if(EXISTS "${CONFIG_FILE}")
    file(READ "${CONFIG_FILE}" config_content)

    # Extract the BUILD_DIR variable from the file content
    string(REGEX MATCH "BUILD_DIR=([^\n]*)" MATCH_RESULT "${config_content}")
    set(CUSTOM_EXEC_PATH "${CMAKE_MATCH_1}")
    message(STATUS "Custom build path: ${CUSTOM_EXEC_PATH}")
else()
    message(FATAL_ERROR "Configuration file not found: ${CONFIG_FILE}. See the README.md for more information.")
endif()

# Ensure custom execution path exists
if(CUSTOM_EXEC_PATH)
    if(EXISTS "${CUSTOM_EXEC_PATH}")
        include_directories(include)
        file(GLOB MY_SOURCES "src/*.c")
        add_executable(purr_hibition ${MY_SOURCES})

        # Link libraries
        target_link_libraries(purr_hibition LINK_PRIVATE hal)

        # Copy the executable to the custom directory after build
        add_custom_command(TARGET purr_hibition POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory
                "${CUSTOM_EXEC_PATH}/purr-hibition"
            COMMAND "${CMAKE_COMMAND}" -E copy
                "$<TARGET_FILE:purr_hibition>"
                "${CUSTOM_EXEC_PATH}/purr-hibition"
            COMMENT "Copying ARM executable to custom directory"
        )

        # copy python files to the target
        add_custom_command(TARGET purr_hibition POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CMAKE_SOURCE_DIR}/python/target"
                "${CUSTOM_EXEC_PATH}/purr-hibition/python"
            COMMAND "${CMAKE_COMMAND}" -E remove_directory
                    "${CUSTOM_EXEC_PATH}/purr-hibition/python/__pycache__"
        COMMENT "Copying Python files to public NFS directory")

        # copy object detection files to the target
        add_custom_command(TARGET purr_hibition POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CMAKE_SOURCE_DIR}/python/MobileNetSSD"
                "${CUSTOM_EXEC_PATH}/purr-hibition/python/MobileNetSSD"
        COMMENT "Copying Python files to public NFS directory")


    else()
        message(FATAL_ERROR "Custom build directory does not exist: ${CUSTOM_EXEC_PATH}.")
    endif()
else()
    message(FATAL_ERROR "CUSTOM_EXEC_PATH is not set. Please ensure the build config file is correct.")
endif()
