#include "proj_bullet.h"

using namespace std;

namespace ProjBulletTouch {
    void DefaultTouch(CProjBullet* pThis, CBaseEntity* pOther);
}

typedef void (*BulletTouchCallback)(CProjBullet*, CBaseEntity*);

void CProjBullet::SetExpVar(const char* _s, const char* _es, int _sc, int _r, float _d) {
    szExpSpr = _s;
    szExpSound = _es;
    iExpSclae = _sc;
    iExpRadius = _r;
    flExpDmg = _d;
}

void CProjBullet::Spawn() {
    if (pev->owner == NULL)
        return;
    Precache();
    pev->movetype = MOVETYPE_FLYMISSILE;
    pev->solid = SOLID_TRIGGER;
    pev->framerate = 1.0f;
    if (!pev->model)
        pev->model = ALLOC_STRING(szSprPath);
    if (pev->speed <= 0)
        pev->speed = flSpeed;
    if (pev->dmg <= 0)
        pev->dmg = 30;
    if (pev->scale <= 0)
        pev->scale = flScale;
    if (pTouchFunc == NULL)
        pTouchFunc = ProjBulletTouch::DefaultTouch;
    pev->rendermode = kRenderTransAdd;
    pev->renderamt = 255;
    pev->rendercolor = Vector(255, 255, 255);
    pev->groupinfo = 114514;
    SET_MODEL(edict(), STRING(pev->model));
    UTIL_SetSize(pev, vecHullMin, vecHullMax);
    UTIL_SetOrigin(pev, pev->origin);

    SetTouch(&CProjBullet::Touch);

    // parametric interpolation makes bullets appear sooner and not disappear too early -wootguy
    if (UTIL_PointInBox(pev->origin, Vector(-4096, -4096, -4096), Vector(4096, 4096, 4096))) {
        SetThink(&CProjBullet::Interpolate);
        pev->nextthink = gpGlobals->time;
    }
}

void CProjBullet::SetAnim(int animIndex) {
    pev->sequence = animIndex;
    pev->frame = 0;
    ResetSequenceInfo();
}

void CProjBullet::Precache() {
    CBaseAnimating::Precache();

    const char* szTemp = pev->model ? szSprPath : STRING(pev->model);
    PRECACHE_MODEL(szTemp);
    PRECACHE_MODEL(szExpSpr);
    PRECACHE_MODEL(szSprPath);

    PRECACHE_SOUND(szHitSound);
    PRECACHE_SOUND(szExpSound);
}
void CProjBullet::Touch(CBaseEntity* pOther) {
    if (FClassnameIs(pOther->pev, STRING(pev->classname)) || pOther->edict() == pev->owner)
        return;

    if (pOther->pev->solid == SOLID_TRIGGER) {
        return; // pass through trigger_hurt -w00tguy
    }

    pTouchFunc(this, pOther);
}

void CProjBullet::Interpolate() {
    ParametricInterpolation(0.1f);
    pev->nextthink = gpGlobals->time + 0.1f;
}

const char* CProjBullet::GetDeathNoticeWeapon() {
    CBaseEntity* owner = CBaseEntity::Instance(pev->owner);
    return owner ? owner->GetDeathNoticeWeapon() : "skull";
}

LINK_ENTITY_TO_CLASS(contra_bullet, CProjBullet)

CProjBullet* ShootABullet(edict_t* pOwner, Vector vecOrigin, Vector vecVelocity){
    CProjBullet* pBullet = (CProjBullet*)(CBaseEntity::Create( BULLET_REGISTERNAME, g_vecZero, g_vecZero, false));

    UTIL_SetOrigin( pBullet->pev, vecOrigin );
    pBullet->pev->owner = pOwner;

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = UTIL_VecToAngles( pBullet->pev->velocity );

    DispatchSpawn( pBullet->edict() );

    return pBullet;
}

CProjBullet* ShootABullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity){
    CProjBullet* pBullet = (CProjBullet*)(CBaseEntity::Create(BULLET_REGISTERNAME, g_vecZero, g_vecZero, false));

    UTIL_SetOrigin( pBullet->pev, vecOrigin );
    pBullet->pev->owner = pOwner->edict();

    pBullet->pev->velocity = vecVelocity;
    pBullet->pev->angles = UTIL_VecToAngles( pBullet->pev->velocity );

    DispatchSpawn( pBullet->edict() );

    return pBullet;
}

namespace ProjBulletTouch {
    void DefaultDirectTouch(CProjBullet* pThis, CBaseEntity* pOther) {
        if (pOther->IsAlive()) {
            pOther->DamageDecal(pThis->iDamageType);
            SpawnBlood(pThis->pev->origin, pOther->BloodColor(), pThis->pev->dmg);
            pOther->TakeDamage(pThis->pev, &pThis->pev->owner->v, pThis->pev->dmg, pThis->iDamageType);
        }
    }
    void DefaultPostTouch(CProjBullet* pThis, CBaseEntity* pOther) {
        EMIT_SOUND(pThis->edict(), CHAN_AUTO, pThis->szHitSound, 1.0f, ATTN_NONE);
        UTIL_Remove(pThis);
    }
    void DefaultTouch(CProjBullet* pThis, CBaseEntity* pOther) {
        ProjBulletTouch::DefaultDirectTouch(pThis, pOther);
        ProjBulletTouch::DefaultPostTouch(pThis, pOther);
    }
    void ExplodeTouch(CProjBullet* pThis, CBaseEntity* pOther) {
        ProjBulletTouch::DefaultDirectTouch(pThis, pOther);
        ::RadiusDamage(pThis->pev->origin, pThis->pev, &pThis->pev->owner->v, pThis->flExpDmg, pThis->iExpRadius, -1, pThis->iDamageType);
        MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
        WRITE_BYTE(TE_EXPLOSION);
        WRITE_COORD(pThis->pev->origin.x);
        WRITE_COORD(pThis->pev->origin.y);
        WRITE_COORD(pThis->pev->origin.z);
        WRITE_SHORT(g_engfuncs.pfnModelIndex(pThis->szExpSpr));
        WRITE_BYTE(pThis->iExpSclae);
        WRITE_BYTE(15);
        WRITE_BYTE(0);
        MESSAGE_END();
        ProjBulletTouch::DefaultPostTouch(pThis, pOther);
    }
}