#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "ammobase.h"
#include "proj_bullet.h"

class CContraWeapon : public CBasePlayerWeapon {
public:
	CBasePlayer* m_pPlayer = NULL;
	int m_iShell;
	float m_flRechargeTime;
	bool bInReload = false;
	CAmmoType* m_iAmmoType;

	void Spawn();
	void Precache();
	BOOL GetItemInfo(ItemInfo* info) override;
	BOOL AddToPlayer(CBasePlayer* pPlayer) override;
	void Holster(int skiplocal /* = 0 */) override;
	BOOL PlayEmptySound() override;
	BOOL Deploy() override;
	float WeaponTimeBase();
	void CancelReload();
	void PrimaryAttack();
	void RechargeThink();
	void Reload() override;
	void WeaponIdle() override;
};

extern ItemInfo g_contra_wep_info;