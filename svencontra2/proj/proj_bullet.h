#include "extdll.h"
#include "util.h"
#include "CBaseAnimating.h"
#include <string>
#include "hlds_hooks.h"
#include "weapons.h"

#define BULLET_REGISTERNAME "contra_bullet"
#define BULLET_HITSOUND "common/null.wav"

class CProjBullet;

typedef void (*BulletTouchCallback)(CProjBullet*, CBaseEntity*);

class CProjBullet : public CBaseAnimating {
public:
    const char* szSprPath = "sprites/svencontra2/bullet_mg.spr";
    const char* szHitSound = BULLET_HITSOUND;
    float flSpeed = 800;
    float flScale = 0.5f;
    int iDamageType = DMG_BULLET;
    Vector vecHullMin = Vector(-4, -4, -4);
    Vector vecHullMax = Vector(4, 4, 4);
    //Exp vars
    const char* szExpSpr = "sprites/svencontra2/bullet_fghit.spr";
    const char* szExpSound = "weapons/svencontra2/shot_fghit.wav";
    int iExpSclae;
    int iExpRadius;
    float flExpDmg;

    BulletTouchCallback pTouchFunc = NULL;

    void SetExpVar(const char* _s, const char* _es, int _sc, int _r, float _d);
    void Spawn();
    void SetAnim(int animIndex);
    void Precache();
    void Touch(CBaseEntity* pOther);
    void Interpolate();
    const char* GetDeathNoticeWeapon() override;
};

namespace ProjBulletTouch {
    void DefaultDirectTouch(CProjBullet* pThis, CBaseEntity* pOther);
    void DefaultPostTouch(CProjBullet* pThis, CBaseEntity* pOther);
    void DefaultTouch(CProjBullet* pThis, CBaseEntity* pOther);
    void ExplodeTouch(CProjBullet* pThis, CBaseEntity* pOther);
}

CProjBullet* ShootABullet(edict_t* pOwner, Vector vecOrigin, Vector vecVelocity);
CProjBullet* ShootABullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity);
