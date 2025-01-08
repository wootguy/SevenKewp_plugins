namespace ContraBoyz
{
// 怪物Event
const int BOYZ_RANGEATTACK_EVENT = 3;
//怪物属性
const string BOYZ_CLASSNAME = "monster_contra_boyz";
const string BOYZ_DISPLAY_NAME = "Orc Boyz";
const string BOYZ_MODEL = "models/barney.mdl";
const string BOYZ_ATTACKSOUND = "sc_contrahdl/hdl_shot_2.wav";//HelloTimber changed, give him a sound when fire.
const string BOYZ_DEATHSOUND = "common/null.wav";//HelloTimber changed, no sound when dead.
const string BOYZ_ALERTSOUND = "AoMDC/monsters/ghost/slv_die.wav";//HelloTimber changed, no sound when see player.
const float BOYZ_BULLETVELOCITY = 512;
const float BOYZ_EYESIGHT_RANGE = 2048;
const float BOYZ_ATTACK_FREQUENCE = 0.3;
const float BOYZ_MOD_HEALTH = 30.0;
const float BOYZ_MOD_MOVESPEED = 325.0;
const int BOYZ_MOD_DMG_INIT = 10;
const float BOYZ_MOD_HEALTH_SURVIVAL = 30.0;
const float BOYZ_MOD_MOVESPEED_SURVIVAL = 400.0;
const int BOYZ_MOD_DMG_INIT_SURVIVAL = 10;

class CMonsterBoyz : ScriptBaseMonsterEntity
{
	private float m_flNextAttack = 0;
	private bool bSurvivalEnabled = g_engfuncs.pfnCVarGetFloat("mp_survival_starton") == 1 && g_engfuncs.pfnCVarGetFloat("mp_survival_supported") == 1;
	
	void Precache()
	{
		BaseClass.Precache();

		PRECACHE_MODEL(BOYZ_MODEL);

		PRECACHE_SOUND(BOYZ_ATTACKSOUND);
		PRECACHE_SOUND(BOYZ_DEATHSOUND);
		PRECACHE_SOUND(BOYZ_ALERTSOUND);
	}
	
	void Spawn()
	{
		Precache();

		if( !SetupModel() )
			SET_MODEL( self, BOYZ_MODEL );
			
		g_EntityFuncs.SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	
		pev->health = bSurvivalEnabled ? BOYZ_MOD_HEALTH_SURVIVAL : BOYZ_MOD_HEALTH;
		pev->solid = SOLID_SLIDEBOX;
		pev->movetype = MOVETYPE_STEP;
		m_bloodColor = BLOOD_COLOR_RED;
		//宽
		pev->view_ofs = Vector( 0, 0, 80 );
		m_flFieldOfView = 0.8;
		m_MonsterState = MONSTERSTATE_NONE;
		m_afCapability = bits_CAP_DOORS_GROUP;
		m_FormattedName = BOYZ_DISPLAY_NAME;

		MonsterInit();
	}

	int	Classify()
	{
		return GetClassification( CLASS_ALIEN_MONSTER );
	}
	
	void SetYawSpeed()
	{
		pev->yaw_speed = bSurvivalEnabled ? BOYZ_MOD_MOVESPEED_SURVIVAL : BOYZ_MOD_MOVESPEED;
	}
	
	void DeathSound()
	{
		EMIT_SOUND_DYN( edict(), CHAN_VOICE, BOYZ_DEATHSOUND, 1, ATTN_NORM, 0, PITCH_NORM );
	}
	
	void Killed(entvars_t* pevAttacker, int iGib)
	{
		BaseClass.Killed(pevAttacker, iGib);
	}
	
	void AlertSound()
	{	
		EMIT_SOUND_DYN( edict(), CHAN_VOICE, BOYZ_ALERTSOUND, 1, ATTN_NORM, 0, PITCH_NORM );
	
	}
	
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
	{	
		if(pevAttacker  == NULL )
			return 0;

		CBaseEntity* pAttacker = CBaseEntity::Instance( pevAttacker );
		if(CheckAttacker( pAttacker ))
			return 0;

		return BaseClass.TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}
	
	CBaseEntity* GetEnemy()
	{
		if(m_hEnemy.IsValid())
			return m_hEnemy.GetEntity().MyMonsterPointer();
		return NULL;
	}	
	
	bool CheckMeleeAttack1( float flDot, float flDist )
	{
		return false;
	}
	bool CheckMeleeAttack2( float flDot, float flDist )
	{
		return false;
	}
	
	bool CheckRangeAttack1(float flDot, float flDist)
	{
		if ( flDist <= BOYZ_EYESIGHT_RANGE && flDot >= 0.5 && NoFriendlyFire())
		{
			if(!m_hEnemy.IsValid())
				return false;
			CBaseMonster* pEnemy = m_hEnemy.GetEntity().MyMonsterPointer();
			if (pEnemy  == NULL )
				return false;
			Vector vecSrc = pev->origin;
			vecSrc.z += pev.size.z * 0.5;;
			Vector vecEnd = (pEnemy.BodyTarget( vecSrc ) - pEnemy.Center()) + m_vecEnemyLKP;
			TraceResult tr;
			TRACE_LINE( vecSrc, vecEnd, dont_ignore_monsters, edict(), tr );
			if ( tr.flFraction == 1.0 || tr.pHit is pEnemy->edict() )
				return true;
		}
		return false;
	}

	//Waaaaaaaagh!
	void ShootPlayer(CBasePlayer* pPlayer)
	{
		MAKE_VECTORS( pev->angles );
		Vector vecSrc = pev->origin;
		vecSrc.z += pev.size.z * 0.5;;
		Vector vecAim = ShootAtEnemy( vecSrc );
		Vector angDir = Math.VecToAngles( vecAim );
		SetBlending( 0, angDir.x );

		EMIT_SOUND_DYN( edict(), CHAN_WEAPON, BOYZ_ATTACKSOUND, 1, ATTN_NORM, 0, PITCH_NORM );
		CProjBullet* pBullet = ShootABullet(self, vecSrc, vecAim * BOYZ_BULLETVELOCITY);
		pBullet.pev->dmg = bSurvivalEnabled ? BOYZ_MOD_DMG_INIT_SURVIVAL : BOYZ_MOD_DMG_INIT;
	}

	void HandleAnimEvent(MonsterEvent* pEvent)
	{
		if(gpGlobals->time < m_flNextAttack)
			return;
		switch(pEvent.event)
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
			default: BaseClass.HandleAnimEvent(pEvent);break;
		}
	}
}
void Register()
{
	g_CustomEntityFuncs.RegisterCustomEntity( "ContraBoyz::CMonsterBoyz", BOYZ_CLASSNAME );
	g_Game.PrecacheOther(BOYZ_CLASSNAME);
}
}