#include "proj/proj_bullet"
#include "weapon/weapon_contra"
#include "weapon/ammobase"
#include "weapon/ammomethod"
#include "monster/monster_boyz"
#include "monster/monster_gunwagon"

#include "point_checkpoint"

void PluginInit()
{
    g_Module.ScriptInfo.SetAuthor( "DrAbc" );
    g_Module.ScriptInfo.SetContactInfo( "Not yet" );
}

void MapInit()
{
    RegisteAmmo("N", 0.35f, *ShootNormalBullet);
    RegisteAmmo("M", 0.15f, *ShootMBullet);
    RegisteAmmo("S", 0.5f, *ShootSBullet);
    RegisteAmmo("L", 0.5f, *ShootLBullet);

    for(uint i = 0; i < aryAmmoType.length(); i++)
    {
        g_CustomEntityFuncs.RegisterCustomEntity( aryAmmoType[i].Name + "Ammo", aryAmmoType[i].Name + "Ammo" );
    }

    g_CustomEntityFuncs.RegisterCustomEntity( "CProjBullet", BULLET_REGISTERNAME );
    g_CustomEntityFuncs.RegisterCustomEntity( "CContraWeapon", "weapon_contra" );
	UTIL_RegisterWeapon( "weapon_contra", "hl_weapons", "weapon_contra" );

    ContraBoyz::Register();
    ContraGunWagon::Register();
}