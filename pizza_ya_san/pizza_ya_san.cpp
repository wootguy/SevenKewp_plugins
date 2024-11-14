#include "extdll.h"
#include "util.h"
#include "weapons.h"
#include "PluginHooks.h"

extern ItemInfo g_soflam_info;
extern ItemInfo g_shotgun_info;
extern ItemInfo g_jetpack_info;

HLCOOP_PLUGIN_HOOKS g_hooks;

HOOK_RETURN_DATA MapInit() {
	g_shotgun_info = UTIL_RegisterWeapon("weapon_as_shotgun");
	g_soflam_info = UTIL_RegisterWeapon("weapon_as_soflam");
	g_jetpack_info = UTIL_RegisterWeapon("weapon_as_jetpack");

	return HOOK_CONTINUE;
}

extern "C" int DLLEXPORT PluginInit() {
	g_hooks.pfnMapInit = MapInit;

	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}