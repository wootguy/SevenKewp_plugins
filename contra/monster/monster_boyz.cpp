#include "proj_bullet.h"
#include "CBaseMonster.h"
#include "monsters.h"

namespace ContraBoyz
{
// 怪物Event
const int BOYZ_RANGEATTACK_EVENT = 3;
//怪物属性
const char* BOYZ_CLASSNAME = "monster_contra_boyz";
const char* BOYZ_DISPLAY_NAME = "Orc Boyz";
const char* BOYZ_MODEL = "models/barney.mdl";
const char* BOYZ_ATTACKSOUND = "sc_contrahdl/hdl_shot_2.wav";//HelloTimber changed, give him a sound when fire.
const char* BOYZ_DEATHSOUND = "common/null.wav";//HelloTimber changed, no sound when dead.
const char* BOYZ_ALERTSOUND = "AoMDC/monsters/ghost/slv_die.wav";//HelloTimber changed, no sound when see player.
const float BOYZ_BULLETVELOCITY = 512;
const float BOYZ_EYESIGHT_RANGE = 2048;
const float BOYZ_ATTACK_FREQUENCE = 0.3;
const float BOYZ_MOD_HEALTH = 30.0;
const float BOYZ_MOD_MOVESPEED = 325.0;
const int BOYZ_MOD_DMG_INIT = 10;
const float BOYZ_MOD_HEALTH_SURVIVAL = 30.0;
const float BOYZ_MOD_MOVESPEED_SURVIVAL = 400.0;
const int BOYZ_MOD_DMG_INIT_SURVIVAL = 10;

class CMonsterBoyz : public CBaseMonster {
public:
	float m_flNextAttack = 0;
	bool bSurvivalEnabled = g_engfuncs.pfnCVarGetFloat("mp_survival_starton") == 1 && g_engfuncs.pfnCVarGetFloat("mp_survival_supported") == 1;
	
	void Precache() override
	{
		CBaseMonster::Precache();

		PRECACHE_MODEL(BOYZ_MODEL);

		PRECACHE_SOUND(BOYZ_ATTACKSOUND);
		PRECACHE_SOUND(BOYZ_DEATHSOUND);
		PRECACHE_SOUND(BOYZ_ALERTSOUND);
	}
	
	void Spawn() override
	{
		Precache();

		SET_MODEL( edict(), BOYZ_MODEL);
			
		UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	
		pev->health = bSurvivalEnabled ? BOYZ_MOD_HEALTH_SURVIVAL : BOYZ_MOD_HEALTH;
		pev->solid = SOLID_SLIDEBOX;
		pev->movetype = MOVETYPE_STEP;
		m_bloodColor = BLOOD_COLOR_RED;
		//宽
		pev->view_ofs = Vector( 0, 0, 80 );
		m_flFieldOfView = 0.8;
		m_MonsterState = MONSTERSTATE_NONE;
		m_afCapability = bits_CAP_DOORS_GROUP;
		m_displayName = MAKE_STRING(BOYZ_DISPLAY_NAME);

		MonsterInit();
	}

	int	Classify() override
	{
		return CBaseMonster::Classify( CLASS_ALIEN_MONSTER );
	}
	
	void SetYawSpeed() override
	{
		pev->yaw_speed = bSurvivalEnabled ? BOYZ_MOD_MOVESPEED_SURVIVAL : BOYZ_MOD_MOVESPEED;
	}
	
	void DeathSound() override
	{
		EMIT_SOUND_DYN( edict(), CHAN_VOICE, BOYZ_DEATHSOUND, 1, ATTN_NORM, 0, PITCH_NORM );
	}
	
	void Killed(entvars_t* pevAttacker, int iGib)
	{
		CBaseMonster::Killed(pevAttacker, iGib);
	}
	
	void AlertSound() override
	{	
		EMIT_SOUND_DYN( edict(), CHAN_VOICE, BOYZ_ALERTSOUND, 1, ATTN_NORM, 0, PITCH_NORM );
	
	}
	
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override
	{	
		if(pevAttacker  == NULL )
			return 0;

		CBaseEntity* pAttacker = CBaseEntity::Instance( pevAttacker );
		if(IRelationship( pAttacker ) < R_NO)
			return 0;

		return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}
	
	CBaseEntity* GetEnemy()
	{
		if(m_hEnemy)
			return m_hEnemy->MyMonsterPointer();
		return NULL;
	}	
	
	BOOL CheckMeleeAttack1( float flDot, float flDist ) override
	{
		return FALSE;
	}
	BOOL CheckMeleeAttack2( float flDot, float flDist ) override
	{
		return FALSE;
	}
	
	BOOL CheckRangeAttack1(float flDot, float flDist) override
	{
		if ( flDist <= BOYZ_EYESIGHT_RANGE && flDot >= 0.5 && NoFriendlyFire())
		{
			if(!m_hEnemy)
				return false;
			CBaseMonster* pEnemy = m_hEnemy->MyMonsterPointer();
			if (pEnemy  == NULL )
				return false;
			Vector vecSrc = pev->origin;
			vecSrc.z += pev->size.z * 0.5;;
			Vector vecEnd = (pEnemy->BodyTarget( vecSrc ) - pEnemy->Center()) + m_vecEnemyLKP;
			TraceResult tr;
			TRACE_LINE( vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr );
			if ( tr.flFraction == 1.0 || tr.pHit == pEnemy->edict() )
				return true;
		}
		return false;
	}

	//Waaaaaaaagh!
	void ShootPlayer(CBasePlayer* pPlayer)
	{
		MAKE_VECTORS( pev->angles );
		Vector vecSrc = pev->origin;
		vecSrc.z += pev->size.z * 0.5;;
		Vector vecAim = ShootAtEnemy( vecSrc );
		Vector angDir = UTIL_VecToAngles( vecAim );
		SetBlending( 0, angDir.x );

		EMIT_SOUND_DYN( edict(), CHAN_WEAPON, BOYZ_ATTACKSOUND, 1, ATTN_NORM, 0, PITCH_NORM );
		CProjBullet* pBullet = ShootABullet(this, vecSrc, vecAim * BOYZ_BULLETVELOCITY);
		pBullet->pev->dmg = bSurvivalEnabled ? BOYZ_MOD_DMG_INIT_SURVIVAL : BOYZ_MOD_DMG_INIT;
	}

	void HandleAnimEvent(MonsterEvent_t* pEvent)
	{
		if(gpGlobals->time < m_flNextAttack)
			return;
		switch(pEvent->event)
		{
			case BOYZ_RANGEATTACK_EVENT:
			{
				//我的剑，只砍玩家
				CBasePlayer* pPlayer = (CBasePlayer*)(GetEnemy());
				if (pPlayer )
				{
					ShootPlayer(pPlayer);
					m_flNextAttack = gpGlobals->time + BOYZ_ATTACK_FREQUENCE;
				}
				break;
			}
			default: CBaseMonster::HandleAnimEvent(pEvent);break;
		}
	}
};
}

LINK_ENTITY_TO_CLASS(monster_contra_boyz, ContraBoyz::CMonsterBoyz)