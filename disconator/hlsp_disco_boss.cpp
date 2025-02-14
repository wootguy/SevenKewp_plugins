#include "extdll.h"
#include "util.h"

HOOK_RETURN_DATA MapInit()
{
	CVAR_SET_FLOAT( "mp_hevsuit_voice", 1 );
	//ClassicModeMapInit();
	return HOOK_CONTINUE;
}

HLCOOP_PLUGIN_HOOKS g_hooks;

extern "C" int DLLEXPORT PluginInit() {
	g_hooks.pfnMapInit = MapInit;
	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}
