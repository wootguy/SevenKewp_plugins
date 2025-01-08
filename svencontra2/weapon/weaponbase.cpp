#include "weaponbase.h"

using namespace std;

extern const char* WEAPONFLAG_REGISTERNAME;

void CBaseContraWeapon::Spawn(){
    Precache();
    SET_MODEL( edict(), szWModel);
    m_iDefaultAmmo = iDefaultAmmo;
    m_iId = wepinfo->iId;
    FallInit();
}

void CBaseContraWeapon::Precache(){
    m_defaultModelV = szVModel;
    m_defaultModelP = szPModel;
    m_defaultModelW = szWModel;
    CBasePlayerWeapon::Precache();

    if (!szPickUpSound) {
        szPickUpSound = "svencontra2/picked.wav";
    }

    PRECACHE_SOUND( "weapons/svencontra2/ar_reload.wav" );
    PRECACHE_SOUND( szPickUpSound );
    PRECACHE_SOUND( szFireSound );

    iShell = szShellModel ? PRECACHE_MODEL( szShellModel ) : -1;
    if(szFloatFlagModel){
        PRECACHE_MODEL( szFloatFlagModel );
    }
}

BOOL CBaseContraWeapon::GetItemInfo(ItemInfo* info) {
    if (!wepinfo) {
        return FALSE;
    }
    *info = *wepinfo;
    return TRUE;
}

BOOL CBaseContraWeapon::AddToPlayer( CBasePlayer* pPlayer ){
    if( !CBasePlayerWeapon::AddToPlayer( pPlayer ) )
        return FALSE;

    m_pPlayer = pPlayer;

    EMIT_SOUND_DYN( pPlayer->edict(), CHAN_WEAPON, szPickUpSound, 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG( 0, 10 ) );
    if(pFlagEntity)
        UTIL_Remove(pFlagEntity);
    return true;
}

void CBaseContraWeapon::Materialize(){
    if(szFloatFlagModel && !pFlagEntity){
        Vector vecOrigin = pev->origin;
        vecOrigin.z += flFlagHeight;
        CBaseEntity* pEntity = CBaseEntity::Create(WEAPONFLAG_REGISTERNAME, vecOrigin, pev->angles, true, edict());
        pEntity->pev->fov = flFlagHeight;
        SET_MODEL(pEntity->edict(), szFloatFlagModel);
        pFlagEntity = EHANDLE(pEntity->edict());
    }
    CBasePlayerWeapon::Materialize();
}

void CBaseContraWeapon::UpdateOnRemove(){
    if(pFlagEntity)
        UTIL_Remove(pFlagEntity);
}

void CBaseContraWeapon::Holster( int skiplocal /* = 0 */ ){
    SetThink( NULL );
    CBasePlayerWeapon::Holster();
}

BOOL CBaseContraWeapon::PlayEmptySound(){
    if( m_iPlayEmptySound ){
        m_iPlayEmptySound = FALSE;
        EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, "weapons/svencontra2/ar_reload.wav", 0.8, ATTN_NORM, 0, PITCH_NORM );
    }
    return FALSE;
}

BOOL CBaseContraWeapon::Deploy(){
    bool bResult = true;
    bResult = DefaultDeploy( GetModelV(), GetModelP(), iDeployAnime, szWeaponAnimeExt );
    m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flDeployTime;
    return bResult;
}

int CBaseContraWeapon::GetRandomAnime(vector<int>& ary){
    return ary[RANDOM_LONG(0, ary.size()-1)];
}

void CBaseContraWeapon::CreateProj(int pellet){
    //Dummy
}

void CBaseContraWeapon::Fire(int pellet){
    CreateProj(pellet);
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
    m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
    m_pPlayer->rgAmmo(m_iPrimaryAmmoType, m_pPlayer->rgAmmo(m_iPrimaryAmmoType)-1);
    EMIT_SOUND_DYN( m_pPlayer->edict(), CHAN_WEAPON, szFireSound, 1.0, ATTN_NORM, 0, 95 + RANDOM_LONG( 0, 10 ) );
    SendWeaponAnim(GetRandomAnime(aryFireAnime));
    m_pPlayer->pev->punchangle.x = RANDOM_FLOAT( vecPunchX.x, vecPunchX.y );
    m_pPlayer->pev->punchangle.y = RANDOM_FLOAT( vecPunchY.x, vecPunchY.y );
    if(iShell > -1)
        EjectBrass( 
            m_pPlayer->GetGunPosition() + gpGlobals->v_forward * vecEjectOffset.x + gpGlobals->v_right * vecEjectOffset.y + gpGlobals->v_up * vecEjectOffset.z, 
            m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_LONG(80,120),
            m_pPlayer->pev->angles[1], iShell, iShellBounce );
}

void CBaseContraWeapon::PrimaryAttack(){
    if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0){
        PlayEmptySound();
        m_flNextPrimaryAttack = WeaponTimeBase() + 0.15f;
        return;
    }
    Fire();
    if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0 )
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", false, 0 );
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = WeaponTimeBase() + flPrimeFireTime;
}

void CBaseContraWeapon::SecondaryAttack(){
    if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0){
        PlayEmptySound();
        m_flNextSecondaryAttack = WeaponTimeBase() + 0.15f;
        return;
    }
    Fire();
    if( m_pPlayer->rgAmmo( m_iPrimaryAmmoType ) <= 0 )
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", false, 0 );
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = WeaponTimeBase() + flSecconaryFireTime;
}

void CBaseContraWeapon::WeaponIdle(){
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
    if( m_flTimeWeaponIdle > WeaponTimeBase() )
        return;
    SendWeaponAnim(GetRandomAnime(aryIdleAnime));
    m_flTimeWeaponIdle = WeaponTimeBase() + RANDOM_FLOAT( 10, 15 );
}

float CBaseContraWeapon::WeaponTimeBase(){
    return gpGlobals->time;
}