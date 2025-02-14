project(disconator)

add_library(${PROJECT_NAME} SHARED
	hlsp_disco_boss.cpp
	cubemath/disco_breakable.cpp
	cubemath/disco_disconator.cpp
	cubemath/disco_drone.cpp
	cubemath/disco_floor_system.cpp
)

hlcoop_setup_plugin("plugins/maps/")