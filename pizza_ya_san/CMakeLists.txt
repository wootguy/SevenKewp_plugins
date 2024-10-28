project(pizza_ya_san)

add_library(${PROJECT_NAME} SHARED
	weapon_as_shotgun.cpp
	weapon_as_soflam.cpp
	weapon_as_jetpack.cpp
	pizza_ya_san.cpp
)

hlcoop_setup_plugin("plugins/maps/")