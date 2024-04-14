cmake_minimum_required(VERSION 3.25)

set (CXX_STANDARD 23)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../dist)
set (CMAKE_CXX_COMPILER clang++)
set (CMAKE_C_COMPILER   clang)

set (CMAKE_DEBUG_POSTFIX d)

set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "")
set(CMAKE_C_STANDARD_LIBRARIES_INIT "")
set(CMAKE_C_STANDARD_LIBRARIES "")
set(CMAKE_CXX_STANDARD_LIBRARIES "")

add_compile_options(
    $<$<CONFIG:DEBUG>:-g3>
    $<$<CONFIG:DEBUG>:-Og>
    -Wall
    -Wextra
    -Wconversion
    -Wsign-conversion
    -fuse-ld=lld
    -fms-extensions
#   -nolibc
    -entry,mainCRTStartup
    $<$<CONFIG:RELEASE>:-Ofast>
    $<$<CONFIG:RELEASE>:-nodgb>
)

add_link_options(
    -GR-
    -Oi
    -MT
    -subsystem,windows
)

# Reading include and lib paths from the .json
file(READ ${CMAKE_SOURCE_DIR}/../script/deps.json DEPENDENCIES)
if(${WIN32})
string(JSON win32Include GET ${DEPENDENCIES} win32nt Include)
string(JSON win32Lib     GET ${DEPENDENCIES} win32nt Lib)    
cmake_path(CONVERT ${win32Include} TO_CMAKE_PATH_LIST ${win32Include})
cmake_path(CONVERT ${win32Lib}     TO_CMAKE_PATH_LIST ${win32Lib})
endif()
string(JSON vulkanInclude GET ${DEPENDENCIES} vulkan Include)
string(JSON vulkanLib     GET ${DEPENDENCIES} vulkan Lib)
cmake_path(CONVERT ${vulkanInclude} TO_CMAKE_PATH_LIST ${vulkanInclude})
cmake_path(CONVERT ${vulkanLib}     TO_CMAKE_PATH_LIST ${vulkanLib})

if(WIN32)
add_compile_definitions(WIN32_NTAPI_INCLUDEDIR=${win32Include})
endif()
add_compile_definitions(VK_INCLUDEDIR=${vulkanInclude})
add_compile_definitions($<$<CONFIG:DEBUG>:DEBUG>)



project(sjt4)

#add_custom_command(COMMAND rc.exe -I ${CMAKE_CURRENT_SOURCE_DIR}/shaders -fo ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.res ${CMAKE_CURRENT_SOURCE_DIR}resources.rc OUTPUT resources.res DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vert.spv ${CMAKE_CURRENT_SOURCE_DIR}/shaders/frag.spv)
add_executable ( ${PROJECT_NAME}
    main.cxx
    app.cxx app.hpp
    renderer.cxx renderer.hpp
    resources/resourcesInclude.s

    app.hpp renderer.hpp resources/resourcesInclude.h
)

target_include_directories(${PROJECT_NAME} PUBLIC $<$<PLATFORM_ID:Windows>:${win32Include}>)
target_link_directories   (${PROJECT_NAME} PUBLIC $<$<PLATFORM_ID:Windows>:${win32Lib}>)
target_include_directories(${PROJECT_NAME} PUBLIC ${vulkanInclude})
target_link_directories   (${PROJECT_NAME} PUBLIC ${vulkanLib})

target_link_libraries(${PROJECT_NAME} vulkan-1 $<$<PLATFORM_ID:Windows>: User32 ntdll >)

set_target_properties(${PROJECT_NAME} 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../dist"
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    $<IF$<PLATFORM_ID:Windows>:WIN32_EXECUTABLE, >
)