cmake_minimum_required(VERSION 3.19)

set(CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

file(READ ${CMAKE_SOURCE_DIR}/../script/deps.json DEPENDENCIES)
string(JSON win32Include GET ${DEPENDENCIES} win32nt Include)
string(JSON win32Lib GET ${DEPENDENCIES} win32nt Lib)
string(JSON vulkanInclude GET ${DEPENDENCIES} vulkan Include)
string(JSON vulkanLib GET ${DEPENDENCIES} vulkan Lib)

add_compile_definitions(WIN32_NTAPI_INCLUDEDIR=${win32Include})
add_compile_definitions(VK_INCLUDEDIR=${vulkanInclude})



project(sjt4)

add_executable(${PROJECT_NAME} WIN32 
    main.cxx
)

target_include_directories(${PROJECT_NAME} PUBLIC ${win32Include} PUBLIC ${vulkanInclude})
target_link_directories(${PROJECT_NAME} PUBLIC ${win32Lib} PUBLIC ${vulkanLib})

target_link_libraries(${PROJECT_NAME} vulkan-1 User32)

set_target_properties(${PROJECT_NAME} 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../app"    
)

#target_link_directories()
#target_include_directories(. )

#add_executable (${PROJECT_NAME} WIN32_EXECUTABLE *.c *.cpp *.hpp *.h)
#target_link_libraries(${PROJECT_NAME} vulkan-1 User32 ntdll)