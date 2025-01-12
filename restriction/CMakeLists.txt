project(restriction)

add_library(${PROJECT_NAME} SHARED
	controller.cpp
	restriction.cpp
)

hlcoop_setup_plugin("plugins/maps/")