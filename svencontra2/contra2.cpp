#include "utility"
#include "monsterdeath"
#include "hook"
#include "dynamicdifficult"

#include "entity/info_weaponflag"
#include "entity/func_noprojclip"
#include "entity/weaponballoon"
#include "entity/func_tank_custom"
#include "entity/trigger_changesky2"

#include "proj/proj_bullet"

#include "weapon/weaponbase"
#include "weapon/weapon_sc2ar"
#include "weapon/weapon_sc2fg"
#include "weapon/weapon_sc2mg"
#include "weapon/weapon_sc2sg"
#include "weapon/weapon_sc2lg"

#include "point_checkpoint"

void PluginInit(){
    g_Module.ScriptInfo.SetAuthor( "❤Dr.Abc Official❤" );
    g_Module.ScriptInfo.SetContactInfo( "❤Love you❤" );
}

void MapInit(){
    g_CustomEntityFuncs.RegisterCustomEntity( "CChangeSky", "trigger_changesky2" );
    g_CustomEntityFuncs.RegisterCustomEntity( "CNoProjClip", "func_noprojclip" );
    g_CustomEntityFuncs.RegisterCustomEntity( "CWeaponFlag", WEAPONFLAG_REGISTERNAME );
    g_CustomEntityFuncs.RegisterCustomEntity( "CProjBullet", BULLET_REGISTERNAME );
    g_CustomEntityFuncs.RegisterCustomEntity( "CWeaponBalloon", "weaponballoon" );
    g_Game.PrecacheOther(BULLET_REGISTERNAME);
    g_CustomEntityFuncs.RegisterCustomEntity( "CustomTank::CFuncTankProj", "func_tankcontra" );
    RegisterPointCheckPointEntity();

    PrecacheAllMonsterDeath();

    g_Hooks.RegisterHook( Hooks::Game::EntityCreated, *EntityCreated );
    g_Hooks.RegisterHook( Hooks::Player::ClientPutInServer, *ClientPutInServer );
    g_Scheduler.SetInterval("SearchAndDestoryMonster", 0.01f, g_Scheduler.REPEAT_INFINITE_TIMES);
    
    g_CustomEntityFuncs.RegisterCustomEntity( "weapon_sc2ar", "weapon_sc2ar" );
    UTIL_RegisterWeapon( "weapon_sc2ar", "svencontra2", "9mm", "ARgrenades" );
    g_CustomEntityFuncs.RegisterCustomEntity( "weapon_sc2fg", "weapon_sc2fg" );
    UTIL_RegisterWeapon( "weapon_sc2fg", "svencontra2", "rockets");
    g_CustomEntityFuncs.RegisterCustomEntity( "weapon_sc2mg", "weapon_sc2mg" );
    UTIL_RegisterWeapon( "weapon_sc2mg", "svencontra2", "556");
    g_CustomEntityFuncs.RegisterCustomEntity( "weapon_sc2sg", "weapon_sc2sg" );
    UTIL_RegisterWeapon( "weapon_sc2sg", "svencontra2", "buckshot");
    g_CustomEntityFuncs.RegisterCustomEntity( "weapon_sc2lg", "weapon_sc2lg" );
    UTIL_RegisterWeapon( "weapon_sc2lg", "svencontra2", "uranium");

    g_SurvivalMode.EnableMapSupport();
}

void MapStart(){
    InitMonsterList();
}
