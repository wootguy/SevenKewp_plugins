#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include <string>
#include <vector>
#include "ammobase.h"
#include "weapon_contra.h"

using namespace std;

std::vector<CAmmoType> aryAmmoType;

CAmmoType* GetAmmo(string Name)
{
    for(unsigned int i = 0; i < aryAmmoType.size(); i++)
    {
        if(aryAmmoType[i].Name == Name)
            return &aryAmmoType[i];
    }
    return NULL;
}

void RegisteAmmo (string _Name, float _FireInter, FireMethod _Method)
{
    aryAmmoType.push_back(CAmmoType(_Name, _FireInter, _Method));
}

class CBaseAmmoEntity : public CBaseAnimating
{
public:
    const char* szMdlPath = "sprites/contra/r.spr";
    const char* szPickUpPath = "items/gunpickup2.wav";
    float flSize = 6;
    CAmmoType* m_iType;

    void Spawn()
    {
        Precache();
        if (!pev->model)
            SET_MODEL(edict(), szMdlPath);
        else
            SET_MODEL(edict(), STRING(pev->model));

        pev->movetype = MOVETYPE_NONE;
        pev->solid = SOLID_TRIGGER;

        UTIL_SetSize(pev, Vector(-flSize, -flSize, -flSize), Vector(flSize, flSize, flSize));
        //SetTouch(TouchFunction(this->Touch));

        CBaseAnimating::Spawn();
    }

    void Precache()
    {
        CBaseAnimating::Precache();

        const char* szTemp = !pev->model ? szMdlPath : STRING(pev->model);
        PRECACHE_MODEL(szTemp);
        PRECACHE_SOUND(szPickUpPath);
    }

    void SetAnim(int animIndex)
    {
        pev->sequence = animIndex;
        pev->frame = 0;
        ResetSequenceInfo();
    }

    void Touch(CBaseEntity* pOther)
    {
        if (pOther->IsAlive() && pOther->IsPlayer() && pOther->IsNetClient())
        {
            CBasePlayer* pPlayer = (CBasePlayer*)(pOther);
            if (pPlayer && IsValidPlayer(pPlayer->edict()))
            {
                CContraWeapon* pWeapon = (CContraWeapon*)pPlayer->GetNamedPlayerItem("weapon_contra");
                if (pWeapon == NULL)
                    return;

                pWeapon->m_iAmmoType = m_iType;
                UTIL_Remove(this);
            }
        }
    }
};

class CNAmmo : public CBaseAmmoEntity {
public:
    CNAmmo()
    {
        szMdlPath = "sprites/contra/n.spr";
        m_iType = GetAmmo("N");
    }
};

class CMAmmo : public CBaseAmmoEntity {
public:
    CMAmmo()
    {
        szMdlPath = "sprites/contra/m.spr";
        m_iType = GetAmmo("M");
    }
};

class CSAmmo : public CBaseAmmoEntity {
public:
    CSAmmo()
    {
        szMdlPath = "sprites/contra/s.spr";
        m_iType = GetAmmo("S");
    }
};

class CLAmmo : public CBaseAmmoEntity {
public:
    CLAmmo()
    {
        szMdlPath = "sprites/contra/l.spr";
        m_iType = GetAmmo("L");
    }
};

LINK_ENTITY_TO_CLASS(NAmmo, CNAmmo)
LINK_ENTITY_TO_CLASS(MAmmo, CMAmmo)
LINK_ENTITY_TO_CLASS(SAmmo, CSAmmo)
LINK_ENTITY_TO_CLASS(LAmmo, CLAmmo)