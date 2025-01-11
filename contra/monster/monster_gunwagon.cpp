#include "proj_bullet.h"
#include "CBaseMonster.h"
#include "monsters.h"
#include "explode.h"
#include "weapons.h"

namespace ContraGunWagon
{
const int SENTY_TURNRATE = 45;//每0.1秒选择角度
const int SENTY_MAXWAIT = 15;
const float SENTY_FIRERATE = 0.2f;
const float SENTY_CHILDRANGE = 2048;
const char* SENTRY_CHILDMODEL = "models/turret.mdl";
const char* SENTRY_CHILFFIRESND = "sc_contrahdl/hdl_shot_2.wav";
const char* SENTRY_SHELLMODEL = "models/saw_shell.mdl";
const int SENTRY_HEALTH = 150;
const int SENTRY_DMG = 10;
const float SENTRY_BULLETSPEED = 512;
const float SENTRY_FIREANGLE = 0.996; //cos0.1°
const char* SENTRY_CLASSNAME = "monster_gunwagon";

enum TURRET_ANIM
{
	SENTY_ANIM_NONE = 0,
	SENTY_ANIM_FIRE,
	SENTY_ANIM_SPIN,
	SENTY_ANIM_DEPLOY,
	SENTY_ANIM_RETIRE,
	SENTY_ANIM_DIE,
};

class CCustomSentry : public CBaseMonster {
public:
    int iShell;
	int iMinPitch =  -60;

	bool bIsActived;
    bool bIsEnemyVisible;

	float flLastSight;
    float fTurnRate;
    float flShootTime;

    Vector vecLastSight;
	Vector vecCurAngles;
	Vector vecGoalAngles;
	
	void Spawn() override
	{
		Precache();
        MonsterInit();

		pev->movetype = MOVETYPE_FLY;
		pev->sequence = 0;
		pev->frame = 0;
		pev->solid = SOLID_SLIDEBOX;
		pev->takedamage = DAMAGE_YES;
        m_bloodColor = DONT_BLEED;

		pev->scale = 1;

		ResetSequenceInfo();
		SetBoneController(0, 0);
		SetBoneController(1, 0);
		m_flFieldOfView = VIEW_FIELD_FULL;
        m_flDistLook = SENTY_CHILDRANGE;
		
		SET_MODEL( edict(), SENTRY_CHILDMODEL);
		pev->health = SENTRY_HEALTH;
		pev->max_health = pev->health;
		m_HackedGunPos = Vector(0,0,48);
		pev->view_ofs.z = 48;

        if(!m_displayName)
			m_displayName = MAKE_STRING("Sentry");
            
		UTIL_SetSize( pev, Vector(-16, -16, 0), Vector(16, 16, 32) );

        vecGoalAngles.x = 0;

		flLastSight = gpGlobals->time + SENTY_MAXWAIT;

		SetThink( &CCustomSentry::AutoSearchThink );
		pev->nextthink = gpGlobals->time + 1; 
	}

    int	Classify() override
	{
		return CBaseMonster::Classify( CLASS_ALIEN_MONSTER );
	}

    float TrimAngle(float vecTrim)
    {
        if(vecTrim >= 360)
            vecTrim -= 360;
        if(vecTrim <= 0)
            vecTrim += 360;
        return vecTrim;
    }

	void Precache() override
	{  
        PRECACHE_MODEL( SENTRY_CHILDMODEL );

        PRECACHE_SOUND( SENTRY_CHILFFIRESND );

        iShell = PRECACHE_MODEL( SENTRY_SHELLMODEL );
	}

    int MoveTurret()
	{
		int state = 0;
		if( vecCurAngles.x != vecGoalAngles.x )
		{
			float flDir = vecGoalAngles.x > vecCurAngles.x ? 1 : -1 ;

			vecCurAngles.x += 0.1 * fTurnRate * flDir;

			if( flDir == 1 )
			{
				if( vecCurAngles.x > vecGoalAngles.x )
					vecCurAngles.x = vecGoalAngles.x;
			} 
			else
			{
				if( vecCurAngles.x < vecGoalAngles.x )
					vecCurAngles.x = vecGoalAngles.x;
			}

				SetBoneController( 1, -vecCurAngles.x );
			state = 1;
		}

		if( vecCurAngles.y != vecGoalAngles.y )
		{
			float flDir = vecGoalAngles.y > vecCurAngles.y ? 1 : -1 ;
			float flDist = abs( vecGoalAngles.y - vecCurAngles.y );
			
			if( flDist > 180 )
			{
				flDist = 360 - flDist;
				flDir = -flDir;
			}
			if( flDist > 30 )
			{
				if( fTurnRate < SENTY_TURNRATE * 10 )
				{
					fTurnRate += SENTY_TURNRATE;
				}
			}
			else if( fTurnRate > 45 )
			{
				fTurnRate -= SENTY_TURNRATE;
			}
			else
			{
				fTurnRate += SENTY_TURNRATE;
			}

			vecCurAngles.y += 0.1 * fTurnRate * flDir;

            vecCurAngles.y = TrimAngle(vecCurAngles.y);

			if( flDist < (0.05 * SENTY_TURNRATE) )
				vecCurAngles.y = vecGoalAngles.y;

				SetBoneController( 0, vecCurAngles.y - pev->angles.y );
			state = 1;
		}

		if( state == 0 )
			fTurnRate = SENTY_TURNRATE;

		return state;
	}
     
    void SetSearch()
    {
        m_hEnemy = NULL;
		flLastSight = gpGlobals->time + SENTY_MAXWAIT;
		SetThink( &CCustomSentry::SearchThink );
    }

    int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override
	{	
		if(pevAttacker  == NULL )
			return 0;

        UTIL_Ricochet(Center(), 1);
		return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}

    void Killed(entvars_t*pevAtttacker, int iGibbed) override
    {
        pev->solid = SOLID_NOT;
		pev->takedamage = DAMAGE_NO;

        SetThink( &CCustomSentry::KillThink );
        pev->nextthink = gpGlobals->time;
    }

    void KillThink()
	{
        SetThink( &CCustomSentry::Explosion );
        SetTurretAnim( SENTY_ANIM_DIE );
        pev->nextthink = gpGlobals->time + 3;
    }

    void Explosion()
    {
        ExplosionCreate(Center(), pev->angles, edict(), 25, true);
        UTIL_Remove(this);
    }
    
	void ActiveThink()
	{
		bool fAttack = false;
		Vector vecDirToEnemy;

		pev->nextthink = gpGlobals->time + 0.1;
		StudioFrameAdvance();

		if( !bIsActived || m_hEnemy.GetEntity()  == NULL  )
		{
			SetSearch();
			return;
		}

		if( !m_hEnemy.GetEntity()->IsAlive() )
		{
			if( flLastSight <= 0.0 )
				flLastSight = gpGlobals->time;
			else
			{
				if( gpGlobals->time > flLastSight )
				{ 
					SetSearch();
				    return;
				}
			}
		}

		Vector vecMid = pev->origin + pev->view_ofs;
		Vector vecMidEnemy = m_hEnemy->BodyTarget( vecMid );

		bIsEnemyVisible = m_hEnemy->FVisible( this, true );
		vecDirToEnemy = vecMidEnemy - vecMid;
		float flDistToEnemy = vecDirToEnemy.Length();
		Vector vec = UTIL_VecToAngles( vecMidEnemy - vecMid );

		if( !bIsEnemyVisible || flDistToEnemy > SENTY_CHILDRANGE )
		{
			if( flLastSight <= 0.0 )
				flLastSight = gpGlobals->time;
			else
			{
				if( gpGlobals->time > flLastSight )
				{
					SetSearch();
					return;
				}
			}
			bIsEnemyVisible = false;
		}
		else
			vecLastSight = vecMidEnemy;

		UTIL_MakeAimVectors( vecCurAngles );
		vecCurAngles.x *= -1;

		Vector vecLOS = vecDirToEnemy;
		vecLOS = vecLOS.Normalize();

		if( DotProduct(vecLOS, gpGlobals->v_forward) <= SENTRY_FIREANGLE )
			fAttack = false;
		else
			fAttack = true;

		if( fAttack )
		{
			Vector vecSrc, vecAng;
			GetAttachment( 0, vecSrc, vecAng );
			Shoot( vecSrc, gpGlobals->v_forward );
		}
		else
			SetTurretAnim( SENTY_ANIM_SPIN );

		if( bIsEnemyVisible )
		{
            vec.y = TrimAngle(vec.y);

			if( vec.x < -180 )
				vec.x += 360;

			if( vec.x > 180 )
				vec.x -= 360;

				if( vec.x > 90 )
					vec.x = 90;
				else if( vec.x < iMinPitch )
					vec.x = iMinPitch;

			vecGoalAngles.y = vec.y;
			vecGoalAngles.x = vec.x;
		}
		MoveTurret();
	}

	void Deploy()
	{
		pev->nextthink = gpGlobals->time + 0.1;
		StudioFrameAdvance();

		if( pev->sequence != SENTY_ANIM_DEPLOY )
		{
			bIsActived = true;
			SetTurretAnim( SENTY_ANIM_DEPLOY );
			SUB_UseTargets( this, USE_ON, 0 );
        }

		if( m_fSequenceFinished )
		{
			vecCurAngles.x = 0;

			SetTurretAnim( SENTY_ANIM_SPIN );
			pev->framerate = 0;
			SetThink( &CCustomSentry::SearchThink );
		}
		flLastSight = gpGlobals->time + SENTY_MAXWAIT;
	}

	void SetTurretAnim( int anim )
	{
		if( pev->sequence != anim )
		{
			switch( anim )
			{
                case SENTY_ANIM_FIRE:break;
                case SENTY_ANIM_SPIN:
                    if( pev->sequence != SENTY_ANIM_FIRE && pev->sequence != SENTY_ANIM_SPIN )
                        pev->frame = 0;
                    break;
                default:pev->frame = 0;break;
			}

			pev->sequence = anim;
			ResetSequenceInfo();

			switch( anim )
			{
			    case SENTY_ANIM_RETIRE:pev->frame = 255;pev->framerate = -1.0; break;
                case SENTY_ANIM_DIE:pev->framerate = 1.0;break;
			}
		}
	}

    CBaseEntity* FindClosestEnemy( float fRadius )
	{
		CBaseEntity* ent = NULL;
		CBaseEntity* enemy = NULL;
        float iNearest = fRadius;
		do
		{
			ent = UTIL_FindEntityInSphere( ent, pev->origin, fRadius ); 

			if ( ent  == NULL  || !ent->IsAlive() )
				continue;

			if (!FClassnameIs(ent->pev, "player"))
				continue;
	
			if ( ent->entindex() == entindex() )
				continue;
				
			if ( ent->edict() == pev->owner )
				continue;
				
			int rel = IRelationship(ent);
			if ( rel == R_AL || rel == R_NO )
				continue;

            if(!ent->FVisible(this, true))
                continue;

			float iDist = ( ent->pev->origin - pev->origin ).Length();
			if ( iDist < iNearest )
			{
				iNearest = iDist;
				enemy = ent;
			}
		}
		while ( ent  );
		return enemy;
	}

	void Shoot( Vector&vecSrc, Vector&vecDirToEnemy )
	{
		if( gpGlobals->time >= flShootTime )
		{
			MAKE_VECTORS( vecCurAngles );
            CProjBullet* pBullet = ShootABullet(this, vecSrc, vecDirToEnemy * SENTRY_BULLETSPEED);
            pBullet->pev->dmg = SENTRY_DMG;

            EMIT_SOUND_DYN( edict(), CHAN_WEAPON, SENTRY_CHILFFIRESND, 1, ATTN_NORM, 0, 94 + RANDOM_LONG( 0, 0xF ) );

            EjectBrass( 
			vecSrc, 
			gpGlobals->v_right * RANDOM_LONG(80,160) + gpGlobals->v_forward * RANDOM_LONG(-20,80) + pev->velocity, 
			vecCurAngles.y, 
			iShell, 
			TE_BOUNCE_SHELL );

			SetTurretAnim(SENTY_ANIM_FIRE);

			flShootTime = gpGlobals->time + SENTY_FIRERATE;
		}
	}

    void CheckValidEnemy()
    {
        if( m_hEnemy.GetEntity()  )
		{
			if( !m_hEnemy.GetEntity()->IsAlive() )
				m_hEnemy = NULL;
		}

		if( m_hEnemy.GetEntity()  == NULL  )
		{
			Look( SENTY_CHILDRANGE );
			m_hEnemy = FindClosestEnemy(SENTY_CHILDRANGE);
		}
    }

	void SearchThink()
	{
		SetTurretAnim( SENTY_ANIM_SPIN );
		StudioFrameAdvance();
		
		CheckValidEnemy();

		if( m_hEnemy.GetEntity()  )
		{
			flLastSight = 0;
			SetThink( &CCustomSentry::ActiveThink );
		}
		else
		{
			vecGoalAngles.y = (vecGoalAngles.y + 0.1 * fTurnRate);
            vecGoalAngles.y = TrimAngle(vecGoalAngles.y);
			MoveTurret();
		}

        pev->nextthink = gpGlobals->time + 0.1;
	}

	void AutoSearchThink()
	{
		StudioFrameAdvance();
		
		CheckValidEnemy();

		if( m_hEnemy.GetEntity()  )
			SetThink( &CCustomSentry::Deploy );

        pev->nextthink = gpGlobals->time + 0.3;
	}
};
}

LINK_ENTITY_TO_CLASS(monster_gunwagon, ContraGunWagon::CCustomSentry)