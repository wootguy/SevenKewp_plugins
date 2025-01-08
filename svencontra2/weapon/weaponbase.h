#pragma once
#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "CBasePlayer.h"
#include <string>
#include "dynamicdifficult.h"

#define CONTRA_WEP_FLAGS (ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_NOAUTOSWITCHEMPTY)
#define CONTRA_WEP_WEIGHT 998

extern ItemInfo g_wepinfo_sc2ar;
extern ItemInfo g_wepinfo_sc2fg;
extern ItemInfo g_wepinfo_sc2mg;
extern ItemInfo g_wepinfo_sc2sg;
extern ItemInfo g_wepinfo_sc2lg;

class CBaseContraWeapon : public CBasePlayerWeapon {
protected:
    CBasePlayer* m_pPlayer = NULL;
    int iShell = -1;
    
    const char* szWModel;
    const char* szPModel;
    const char* szVModel;
    const char* szShellModel;
    const char* szPickUpSound;
    const char* szFloatFlagModel;

    ItemInfo* wepinfo;
    int iDefaultAmmo;

    float flDeployTime;
    float flPrimeFireTime;
    float flSecconaryFireTime;

    const char* szWeaponAnimeExt;
    int iDeployAnime;
    int iReloadAnime;
    std::vector<int> aryFireAnime;
    std::vector<int> aryIdleAnime;

    const char* szFireSound;

    Vector2D vecPunchX;
    Vector2D vecPunchY;
    float flBulletSpeed;
    float flDamage;
    int iShellBounce = TE_BOUNCE_SHELL;

    Vector vecEjectOffset;

    EHANDLE pFlagEntity = NULL;
    float flFlagHeight = 24;

    virtual void Spawn() override;
    virtual void Precache() override;
    BOOL GetItemInfo(ItemInfo* info) override;
    virtual BOOL AddToPlayer(CBasePlayer* pPlayer) override;
    virtual void Materialize(void) override;
    virtual void UpdateOnRemove() override;
    virtual void Holster(int skiplocal /* = 0 */) override;
    virtual BOOL PlayEmptySound() override;
    virtual BOOL Deploy() override;
    int GetRandomAnime(std::vector<int>& ary);
    virtual void CreateProj(int pellet = 1);
    virtual void Fire(int pellet = 1);
    virtual void PrimaryAttack() override;
    virtual void SecondaryAttack() override;
    virtual void WeaponIdle() override;
    virtual float WeaponTimeBase();
};