cmake_minimum_required(VERSION 3.2)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../hlcoop.cmake)
project(pizza_ya_san)

add_library(${PROJECT_NAME} SHARED ${HLCOOP_HEADERS} 
	weapon_as_shotgun.cpp
)

hlcoop_sdk_init()
