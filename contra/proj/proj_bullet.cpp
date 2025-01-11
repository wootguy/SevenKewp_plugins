#include "proj_bullet.h"
#include <string>
#include "hlds_hooks.h"
#include "weapons.h"

void CProjBullet::Spawn()
{
    if (pev->owner == NULL)
        return;

    Precache();

    pev->movetype = MOVETYPE_FLYMISSILE;
    pev->solid = SOLID_SLIDEBOX;

    pev->framerate = 1.0f;
    //pev->rendermode = kRenderNormal;
    //pev->renderamt = 255;
    //pev->rendercolor = vecColor;

    pev->model = MAKE_STRING(szSprPath);
    pev->scale = flScale;
    pev->speed = flSpeed;
    pev->dmg = BULLET_DEFAULTDMG;

    g_engfuncs.pfnMakeVectors(pev->angles);
    //pev->velocity = gpGlobals->v_forward * pev->speed;
    vecVelocity = pev->velocity;

    SET_MODEL(edict(), STRING(pev->model));
    UTIL_SetSize(pev, vecHullMin, vecHullMax);
    UTIL_SetOrigin(pev, pev->origin);

    pev->nextthink = gpGlobals->time + 0.1f;

    SetTouch(&CProjBullet::Touch);
    SetThink(&CProjBullet::DefaultThink);
}

void CProjBullet::DefaultThink()
{
    pev->velocity = vecVelocity;
    pev->nextthink = gpGlobals->time + 0.1f;
}

void CProjBullet::SetAnim(int animIndex)
{
    pev->sequence = animIndex;
    pev->frame = 0;
    ResetSequenceInfo();
}

void CProjBullet::DelayTouch()
{
    pev->solid = SOLID_SLIDEBOX;
    SetThink(&CProjBullet::DefaultThink);
    pev->nextthink = gpGlobals->time + 0.1f;
}

void CProjBullet::Precache()
{
    CBaseAnimating::Precache();

    const char* szTemp = !pev->model ? szSprPath : STRING(pev->model);
    PRECACHE_MODEL(szTemp);

    PRECACHE_MODEL(BULLET_MDL1);
    PRECACHE_MODEL(BULLET_MDL2);

    PRECACHE_SOUND(szHitSound);
}

void CProjBullet::Touch(CBaseEntity* pOther)
{
    if (FClassnameIs(pev, STRING(pOther->pev->classname)) || pOther->edict() == pev->owner)
    {
        pev->velocity = gpGlobals->v_forward * pev->speed;
        return;
    }

    if (pOther->IsAlive())
        pOther->TakeDamage(pev, &pev->owner->v, pev->dmg, iDamageType);

    EMIT_SOUND(edict(), CHAN_AUTO, szHitSound, 1.0f, ATTN_NONE);
    UTIL_Remove(this);
}

LINK_ENTITY_TO_CLASS(contra_bullet, CProjBullet)

CProjBullet* ShootABullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity)
{
    CProjBullet* pBullet = (CProjBullet*)CBaseEntity::Create( BULLET_REGISTERNAME, g_vecZero, g_vecZero, false);

    UTIL_SetOrigin( pBullet->pev, vecOrigin );
    pBullet->pev->owner = pOwner->edict();

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = UTIL_VecToAngles( pBullet->pev->velocity );

    DispatchSpawn( pBullet->edict() );

    return pBullet;
}