#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseAnimating.h"

#define BULLET_REGISTERNAME "contra_bullet"
#define BULLET_DEFAULT_SPEED 800
#define BULLET_MDL1 "sprites/contra/contra_bullet1.spr"
#define BULLET_MDL2 "sprites/contra/contra_bullet2.spr"
#define BULLET_DEFAULTDMG 30.0f

class CProjBullet : public CBaseAnimating
{
public:
    const char* szSprPath = BULLET_MDL2;
    const char* szHitSound = "common/null.wav";
    float flSpeed = BULLET_DEFAULT_SPEED;
    float flScale = 0.5f;
    int iDamageType = DMG_BULLET;

    Vector vecHullMin = Vector(-4, -4, -4);
    Vector vecHullMax = Vector(4, 4, 4);

    Vector vecVelocity = Vector(0, 0, 0);
    Vector vecColor = Vector(255, 255, 255);

    void Spawn();
    void DefaultThink();
    void SetAnim(int animIndex);
    void DelayTouch();
    void Precache();
    void Touch(CBaseEntity* pOther);
};

CProjBullet* ShootABullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity);