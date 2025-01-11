#pragma once
#include "weapons.h"
#include <string>
#include <vector>

#define AMMO_SACCURANCY VECTOR_CONE_20DEGREES

typedef void (*FireMethod)(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity);

class CAmmoType {
public:
    std::string Name;
    float FireInter;
    std::string FireSnd;
    Vector Accurency;
    FireMethod Method;

    CAmmoType(std::string _Name, float _FireInter, FireMethod _Method)
    {
        Name = _Name;
        FireInter = _FireInter;
        Method = _Method;
    }
};

extern std::vector<CAmmoType> aryAmmoType;

void RegisteAmmo(std::string _Name, float _FireInter, FireMethod _Method);

void ShootNormalBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity);
void ShootMBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity);
void ShootSBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity);
void ShootLBullet(CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity);