#include "extdll.h"
#include "util.h"
#include "PluginHooks.h"
#include "Scheduler.h"
#include "proj_bullet.h"
#include "weaponbase.h"

HLCOOP_PLUGIN_HOOKS g_hooks;

void PrecacheAllMonsterDeath();
void SearchAndDestoryMonster();
void InitMonsterList();
HOOK_RETURN_DATA ClientPutInServerHook(CBasePlayer* pPlayer);
HOOK_RETURN_DATA EntityCreatedHook(CBaseEntity* pEntity);

HOOK_RETURN_DATA MapInit(){
    UTIL_PrecacheOther(BULLET_REGISTERNAME);
    PrecacheAllMonsterDeath();
    
    g_Scheduler.SetInterval(SearchAndDestoryMonster, 0.01f, g_Scheduler.REPEAT_INFINITE_TIMES);

    g_wepinfo_sc2ar = UTIL_RegisterWeapon("weapon_sc2ar");
    g_wepinfo_sc2fg = UTIL_RegisterWeapon("weapon_sc2fg");
    g_wepinfo_sc2mg = UTIL_RegisterWeapon("weapon_sc2mg");
    g_wepinfo_sc2sg = UTIL_RegisterWeapon("weapon_sc2sg");
    g_wepinfo_sc2lg = UTIL_RegisterWeapon("weapon_sc2lg");

    UTIL_PrecacheOther("weapon_sc2ar");
    UTIL_PrecacheOther("weapon_sc2fg");
    UTIL_PrecacheOther("weapon_sc2mg");
    UTIL_PrecacheOther("weapon_sc2sg");
    UTIL_PrecacheOther("weapon_sc2lg");

    //g_SurvivalMode.EnableMapSupport();

    return HOOK_CONTINUE;
}

HOOK_RETURN_DATA MapStart(){
    InitMonsterList();
    return HOOK_CONTINUE;
}

extern "C" int DLLEXPORT PluginInit() {
    g_hooks.pfnMapInit = MapInit;
    g_hooks.pfnServerActivate = MapStart;
    g_hooks.pfnEntityCreated = EntityCreatedHook;
    g_hooks.pfnClientPutInServer = ClientPutInServerHook;

    return RegisterPlugin(&g_hooks);
}
