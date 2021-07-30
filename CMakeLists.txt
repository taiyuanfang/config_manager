cmake_minimum_required (VERSION 2.8.12)

project (config_manager)

set (BuildType STATIC)


set (TARGET_NAME "config_manager")
add_library (${TARGET_NAME} ${BuildType}
    src/config_manager.c
)
target_include_directories (${TARGET_NAME} PUBLIC
    include
    src
)


file (GLOB_RECURSE JPATH_FILES
    src/jpath/*.c
)
set (TARGET_NAME "jpath")
add_library (${TARGET_NAME} ${BuildType}
    ${JPATH_FILES}
)


file (GLOB_RECURSE CMS_FILES
    src/cms/*.c
)
set (TARGET_NAME "cms")
add_executable (${TARGET_NAME}
    ${CMS_FILES}
)
target_include_directories (${TARGET_NAME} PRIVATE
    src
    src/jpath
)
target_link_libraries (${TARGET_NAME}
    config_manager
    jpath
)
install_target(${TARGET_NAME} ${DIR_NFS}/bin)


set(TARGET_NAME "cmc")
add_executable(${TARGET_NAME}
    src/cmc.c
)
target_link_libraries (${TARGET_NAME}
    config_manager
)
install_target(${TARGET_NAME} ${DIR_NFS}/bin)


set(TARGET_NAME "example")
add_executable(${TARGET_NAME}
    example/main.c
)
target_link_libraries (${TARGET_NAME}
    config_manager
)
