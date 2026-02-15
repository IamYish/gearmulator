cmake_minimum_required(VERSION 3.15)
project(mpcSamplerLib VERSION ${CMAKE_PROJECT_VERSION})

set(SOURCES
	samplerDevice.cpp samplerDevice.h
	samplerEngine.cpp samplerEngine.h
	samplerVoice.cpp samplerVoice.h
)

add_library(mpcSamplerLib STATIC ${SOURCES})

target_include_directories(mpcSamplerLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(mpcSamplerLib PUBLIC synthLib)

source_group("source" FILES ${SOURCES})
