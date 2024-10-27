cmake_minimum_required(VERSION 3.2)

include(${CMAKE_SOURCE_DIR}/SevenKewp/scripts/hlcoop.cmake)
project(pizza_ya_san)

add_library(${PROJECT_NAME} SHARED ${HLCOOP_HEADERS} 
	weapon_as_shotgun.cpp
	weapon_as_soflam.cpp
	weapon_as_jetpack.cpp
	pizza_ya_san.cpp
)

hlcoop_sdk_init()
