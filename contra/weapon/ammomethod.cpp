#include "extdll.h"
#include "util.h"
#include "weapon_contra.h"
#include "proj_bullet.h"
#include "Scheduler.h"
#include "hlds_hooks.h"

void ShootNormalBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    CProjBullet* pBullet = (CProjBullet*)CBaseEntity::Create( BULLET_REGISTERNAME, g_vecZero, g_vecZero, false);

    UTIL_SetOrigin( pBullet->pev, vecOrigin );
    pBullet->pev->owner = pOwner->edict();

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = UTIL_VecToAngles( pBullet->pev->velocity );
    pBullet->szSprPath = BULLET_MDL1;

    DispatchSpawn( pBullet->edict() );
}

void ShootMBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    CProjBullet* pBullet = (CProjBullet*)CBaseEntity::Create(BULLET_REGISTERNAME, g_vecZero, g_vecZero, false);

    UTIL_SetOrigin(pBullet->pev, vecOrigin);
    pBullet->pev->owner = pOwner->edict();

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = UTIL_VecToAngles( pBullet->pev->velocity );

    DispatchSpawn(pBullet->edict());
}

void ShootSBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    for(int i = 0; i <= 4; i++)
    {
        CProjBullet* pBullet = (CProjBullet*)CBaseEntity::Create(BULLET_REGISTERNAME, g_vecZero, g_vecZero, false);
        UTIL_SetOrigin(pBullet->pev, vecOrigin);
        pBullet->pev->owner = pOwner->edict();

        float x, y;
        GetCircularGaussianSpread( x, y );
        MAKE_VECTORS( pOwner->pev->v_angle + pOwner->pev->punchangle );
        Vector vecAngles = gpGlobals->v_forward * vecVelocity.Length() + 
                        (x * 1000 * AMMO_SACCURANCY.x * gpGlobals->v_right + 
                            y * 1000 * AMMO_SACCURANCY.y * gpGlobals->v_up);
        pBullet->pev->velocity = vecAngles;

        pBullet->pev->nextthink = gpGlobals->time + 1.0f;
        DispatchSpawn(pBullet->edict());
        pBullet->pev->solid = SOLID_NOT;

        pBullet->m_pfnThink = static_cast<void (CBaseEntity::*)(void)>(&CProjBullet::DelayTouch);
    }
}

void ShootLBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    for(int i = 0; i <= 4; i++)
    {
        g_Scheduler.SetTimeout(ShootMBullet, 0.02 * i, pOwner, vecOrigin, vecVelocity);
    }
}