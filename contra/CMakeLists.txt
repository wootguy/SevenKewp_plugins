project(contra)

add_library(${PROJECT_NAME} SHARED
	monster/monster_boyz.cpp
	monster/monster_gunwagon.cpp
	
	proj/proj_bullet.h proj/proj_bullet.cpp
	
	weapon/ammobase.h weapon/ammobase.cpp
	weapon/ammomethod.cpp
	weapon/weapon_contra.h weapon/weapon_contra.cpp
	
	contra.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE .)
target_include_directories(${PROJECT_NAME} PRIVATE proj)
target_include_directories(${PROJECT_NAME} PRIVATE weapon)

hlcoop_setup_plugin("plugins/maps/")