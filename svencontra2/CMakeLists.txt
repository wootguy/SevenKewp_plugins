project(svencontra2)

add_library(${PROJECT_NAME} SHARED
	entity/func_noprojclip.cpp
	entity/func_tank_custom.cpp
	entity/info_weaponflag.cpp
	entity/weaponballoon.cpp
	
	proj/proj_bullet.h proj/proj_bullet.cpp
	
	weapon/weapon_sc2ar.cpp
	weapon/weapon_sc2fg.cpp
	weapon/weapon_sc2lg.cpp
	weapon/weapon_sc2mg.cpp
	weapon/weapon_sc2sg.cpp
	weapon/weaponbase.h weapon/weaponbase.cpp
	
	contra2.cpp
	dynamicdifficult.h dynamicdifficult.cpp
	hook.cpp
	monsterdeath.cpp
	point_checkpoint.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE .)
target_include_directories(${PROJECT_NAME} PRIVATE proj)
target_include_directories(${PROJECT_NAME} PRIVATE weapon)

hlcoop_setup_plugin("plugins/maps/")