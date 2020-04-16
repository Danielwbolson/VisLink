project(VisLinkProject)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_FRAMEWORK_DIR ${CMAKE_SOURCE_DIR}/cmake/framework)

if(NOT EXISTS ${CMAKE_FRAMEWORK_DIR})
execute_process(
		#COMMAND git clone ${PROJECT_SOURCE_DIR}/.. ${CMAKE_FRAMEWORK_DIR}
		COMMAND git clone https://github.com/dtorban/cmake_fw.git ${CMAKE_FRAMEWORK_DIR}
)
endif()

include(${CMAKE_FRAMEWORK_DIR}/project/MainProject.cmake)

add_subdirectory(src/VisLink)
add_subdirectory(app)
add_subdirectory(examples/MiniCave)
add_subdirectory(examples/NoSockets)
add_subdirectory(examples/VLOpenGLConnector)
add_subdirectory(examples/VLUnityConnector)
add_subdirectory(examples/Filter)
add_subdirectory(microservices/SimpleService)
add_subdirectory(microservices/SimpleClient)
add_subdirectory(microservices/MicroService)
add_subdirectory(microservices/ExampleClient)
