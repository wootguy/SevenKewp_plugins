#include "proj_bullet.h"
#include "weapon_contra.h"
#include "ammobase.h"

HLCOOP_PLUGIN_HOOKS g_hooks;

namespace ContraBoyz { extern const char* BOYZ_CLASSNAME; }
namespace ContraGunWagon { extern const char* SENTRY_CLASSNAME; }

HOOK_RETURN_DATA MapInit()
{
    RegisteAmmo("N", 0.35f, ShootNormalBullet);
    RegisteAmmo("M", 0.15f, ShootMBullet);
    RegisteAmmo("S", 0.5f, ShootSBullet);
    RegisteAmmo("L", 0.5f, ShootLBullet);

    g_contra_wep_info = UTIL_RegisterWeapon( "weapon_contra" );

    UTIL_PrecacheOther(ContraBoyz::BOYZ_CLASSNAME);
    UTIL_PrecacheOther(ContraGunWagon::SENTRY_CLASSNAME);

    return HOOK_CONTINUE;
}

extern "C" int DLLEXPORT PluginInit() {
    g_hooks.pfnMapInit = MapInit;

    //g_Module.ScriptInfo.SetAuthor("DrAbc");
    //g_Module.ScriptInfo.SetContactInfo("Not yet");

    return RegisterPlugin(&g_hooks);
}