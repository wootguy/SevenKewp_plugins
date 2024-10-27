project(pizza_ya_san)

include(${MOD_DIR}/scripts/hlcoop.cmake)

add_library(${PROJECT_NAME} SHARED ${HLCOOP_HEADERS} 
	weapon_as_shotgun.cpp
	weapon_as_soflam.cpp
	weapon_as_jetpack.cpp
	pizza_ya_san.cpp
)

hlcoop_setup_plugin("plugins/maps/pizza_ya_san/")