cmake_minimum_required(VERSION 3.25)

set(CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../dist)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_DEBUG_POSTFIX d)

# To get rid of a bunch of unused dlls being linked into the program
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
    -nolibc
    $<$<CONFIG:RELEASE>:-Ofast>
    $<$<CONFIG:RELEASE>:-nodgb>
)

add_link_options(
    -GR-
    -Oi
    -MT
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

add_executable(${PROJECT_NAME}
    main.cxx
    app.hpp         app.cxx
    windowing.hpp   windowing.cxx
)

target_include_directories(${PROJECT_NAME} PUBLIC $<$<PLATFORM_ID:Windows>:${win32Include}>)
target_link_directories   (${PROJECT_NAME} PUBLIC $<$<PLATFORM_ID:Windows>:${win32Lib}>)
target_include_directories(${PROJECT_NAME} PUBLIC ${vulkanInclude})
target_link_directories   (${PROJECT_NAME} PUBLIC ${vulkanLib})

target_link_libraries(${PROJECT_NAME} vulkan-1 $<$<PLATFORM_ID:Windows>: User32 ntdll >)

#target_link_options(${PROJECT_NAME} 
    #PRIVATE $<$<PLATFORM_ID:Windows>:/ENTRY:WinMainCRTStartup> 
    #PRIVATE "/NODEFAULTLIB"
    #PUBLIC $<$<PLATFORM_ID:Windows>:--subsystem,Windows>
#)
#target_compile_options(${PROJECT_NAME}
    

#)

set_target_properties(${PROJECT_NAME} 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../dist"
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    $<IF$<PLATFORM_ID:Windows>:WIN32_EXECUTABLE, >
)