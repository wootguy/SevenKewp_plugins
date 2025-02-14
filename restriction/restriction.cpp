#include "extdll.h"
#include "util.h"
#include "PluginManager.h"

void ControllerMapInit();
void npc_controller(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue);

HOOK_RETURN_DATA MapInit()
{
	//RegisterPointCheckPointEntity();
	ControllerMapInit();
	return HOOK_CONTINUE;
}

HLCOOP_PLUGIN_HOOKS g_hooks;

extern "C" int DLLEXPORT PluginInit() {
	g_hooks.pfnMapInit = MapInit;

	RegisterPluginEntCallback(npc_controller);

	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}