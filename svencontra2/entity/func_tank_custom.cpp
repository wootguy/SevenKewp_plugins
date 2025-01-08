#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "Scheduler.h"
#include "hlds_hooks.h"
#include "weapons.h"
#include "monsters.h"
#include "CSprite.h"
#include "CLaser.h"
#include "CFuncTank.h"
#include "explode.h"
#include "proj_bullet.h"
#include <vector>

using namespace std;

namespace CustomTank
{

const vector<Vector> gTankSpread =
{
    Vector( 0, 0, 0 ), // perfect
    Vector( 0.025, 0.025, 0.025 ),    // small cone
    Vector( 0.05, 0.05, 0.05 ),  // medium cone
    Vector( 0.1, 0.1, 0.1 ),    // large cone
    Vector( 0.25, 0.25, 0.25 ),    // extra-large cone
};

class CFuncTankCustom : public CBaseMonster {
public:
    EHANDLE m_pController;
    EHANDLE m_pControlEntity;
    float m_flNextAttack;
    Vector m_vecControllerUsePos;

    float m_yawCenter;    // "Center" yaw
    float m_yawRate; // Max turn rate to track targets
    float m_yawRange; // Range of turning motion (one-sided: 30 is +/- 30 degress from center)
    // Zero is full rotation
    float m_yawTolerance;    // Tolerance angle

    float m_pitchCenter;    // "Center" pitch
    float m_pitchRate;    // Max turn rate on pitch
    float m_pitchRange;    // Range of pitch motion as above
    float m_pitchTolerance;    // Tolerance angle

    float m_fireLast; // Last time I fired
    float m_fireRate; // How many rounds/second
    float m_lastSightTime;// Last time I saw target
    float m_persist; // Persistence of firing (how long do I shoot when I can't see)
    float m_minRange; // Minimum range to aim/track
    float m_maxRange; // Max range to aim/track

    Vector m_barrelPos;    // Length of the freakin barrel
    float m_spriteScale;    // Scale of any sprites we shoot
    string_t m_iszSpriteSmoke;
    string_t m_iszSpriteFlash;
    string_t m_iszRotateSound;
    int m_bulletType;                                      // Bullet type
    int m_iBulletDamage; // 0 means use Bullet type's default damage

    Vector m_sightOrigin;    // Last sight of target
    int m_spread; // firing spread
    string_t m_iszMaster;    // Master entity (game_team_master || multisource)

    virtual	BOOL IsNormalMonster(void) { return FALSE; }
    Vector UpdateTargetPosition( CBaseEntity* pTarget )
    {
        return pTarget->BodyTarget( pev->origin );
    }
    int ObjectCaps()
    {
        return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;
    }
    int Classify()
    {
        if (m_pController)
            return m_pController->Classify();
        return m_Classify;
    }
    bool IsActive()
    {
        return (pev->spawnflags & SF_TANK_ACTIVE) != 0;
    }
    void TankActivate()
    {
        pev->spawnflags |= SF_TANK_ACTIVE;
        pev->nextthink = pev->ltime + 0.1;
        m_fireLast = 0;
    }
    void TankDeactivate()
    {
        pev->spawnflags &= ~SF_TANK_ACTIVE;
        m_fireLast = 0;
        StopRotSound();
    }
    bool CanFire()
    {
        return (gpGlobals->time - m_lastSightTime) < m_persist;
    }
    Vector BarrelPosition()
    {
        MAKE_VECTORS(pev->angles);
        return pev->origin + gpGlobals->v_forward * m_barrelPos.x + gpGlobals->v_right * m_barrelPos.y + gpGlobals->v_up * m_barrelPos.z;
    }
    virtual void Precache()
    {
        CBaseMonster::Precache();
        if ( m_iszSpriteSmoke )
            PRECACHE_MODEL( STRING(m_iszSpriteSmoke) );
        if ( m_iszSpriteFlash )
            PRECACHE_MODEL( STRING(m_iszSpriteFlash) );
        if ( m_iszRotateSound )
            PRECACHE_SOUND( STRING(m_iszRotateSound) );
    }
    virtual void Spawn()
    {
        CBaseMonster::Spawn();

        Precache();
        pev->movetype = MOVETYPE_PUSH;  // so it doesn't get pushed by anything
        pev->solid = SOLID_BSP;

        SET_MODEL( edict(), STRING(pev->model));

        m_yawCenter = pev->angles.y;
        m_pitchCenter = pev->angles.x;
        m_sightOrigin = BarrelPosition(); // Point at the end of the barrel

        if (IsActive())
            pev->nextthink = pev->ltime + 1.0;
        
        if (m_fireRate <= 0)
            m_fireRate = 1;
        if ( m_spread >= int(gTankSpread.size()) )
            m_spread = 0;
        pev->oldorigin = pev->origin;
    }
    virtual void KeyValue(KeyValueData* pkvd)
    {
        if (FStrEq(pkvd->szKeyName, "yawrate"))
        {
            m_yawRate = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "yawrange"))
        {
            m_yawRange = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "yawtolerance"))
        {
            m_yawTolerance = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "pitchrange"))
        {
            m_pitchRange = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "pitchrate"))
        {
            m_pitchRate = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "pitchtolerance"))
        {
            m_pitchTolerance = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "firerate"))
        {
            m_fireRate = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "barrel"))
        {
            m_barrelPos.x = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "barrely"))
        {
            m_barrelPos.y = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "barrelz"))
        {
            m_barrelPos.z = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "spritescale"))
        {
            m_spriteScale = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "spritesmoke"))
        {
            m_iszSpriteSmoke = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "spriteflash"))
        {
            m_iszSpriteFlash = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "rotatesound"))
        {
            pev->noise = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "persistence"))
        {
            m_persist = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "bullet"))
        {
            m_bulletType = (TANKBULLET)atoi(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "bullet_damage"))
        {
            m_iBulletDamage = atoi(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "firespread"))
        {
            m_spread = atoi(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "minRange"))
        {
            m_minRange = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "maxRange"))
        {
            m_maxRange = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "master"))
        {
            m_iszMaster = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else
            CBaseMonster::KeyValue(pkvd);
    }
    bool StartControl( CBasePlayer* pController )
    {
        if ( m_pController )
            return false;
        // Team only || disabled?
        if ( m_iszMaster && !UTIL_IsMasterTriggered( m_iszMaster, pController))
            return false;
        
        //Useless but blow ur game up
        //pController.m_hTargetTank = EHANDLE(self);
        pev->owner = pController->edict();
        edict_t* pControllEntity = FIND_ENTITY_BY_STRING(NULL, "target", STRING(pev->targetname));
        m_pControlEntity = EHANDLE(pControllEntity);

        m_pController = EHANDLE(pController->edict());
        if ( pController->m_pActiveItem )
        {
            ((CBasePlayerWeapon*)pController->m_pActiveItem.GetEntity())->Holster();
            pController->pev->weaponmodel = 0;
            pController->pev->viewmodel = 0;
            pController->m_flNextAttack = gpGlobals->time + 9999;
        }

        pController->m_iHideHUD |= HIDEHUD_WEAPONS;
        m_vecControllerUsePos = pController->pev->origin;
        
        pev->nextthink = pev->ltime + 0.1;
        
        return true;
    }
    void StopControl()
    {
        if ( !m_pController )
            return;
        CBasePlayer* pPlayer = (CBasePlayer*)(m_pController.GetEntity());
        if ( pPlayer->m_pActiveItem )
            ((CBasePlayerWeapon*)pPlayer->m_pActiveItem.GetEntity())->Deploy();
        
        //Useless but blow ur game up
        //pPlayer.m_hTargetTank = EHANDLE(NULL);

        pev->owner = NULL;
        pPlayer->m_iHideHUD &= ~HIDEHUD_WEAPONS;
        pPlayer->m_flNextAttack = 0;

        pev->nextthink = 0;
        m_pController = EHANDLE(NULL);
        m_pControlEntity = EHANDLE(NULL);

        if ( IsActive() )
            pev->nextthink = pev->ltime + 1.0;
    }
    bool IsInControllerEntity(CBaseEntity* pPlayer)
    {
        //AABB
        CBaseEntity* pEntity = m_pControlEntity.GetEntity();
        // 被包含
        CBaseEntity* pA = NULL;
        // 包含
        CBaseEntity* pB = NULL;
        if (pPlayer->pev->size.Length() > pEntity->pev->size.Length()) {
            pA = pEntity;
            pB = pPlayer;
        }
        else{
            pA = pPlayer;
            pB = pEntity;
        }
        if (pA->pev->absmin.x > pB->pev->absmax.x)
            return false;
        else if (pA->pev->absmax.x < pB->pev->absmin.x)
            return false;
        else if (pA->pev->absmin.y > pB->pev->absmax.y)
            return false;
        else if (pA->pev->absmax.y < pB->pev->absmin.y)
            return false;
        else if (pA->pev->absmin.z > pB->pev->absmax.z)
            return false;
        else if (pA->pev->absmax.z < pB->pev->absmin.z)
            return false;
        return true;
    }
    // Called each frame by the player's ItemPostFrame
    void ControllerPostFrame()
    {
        if(!m_pController)
            return;
        if(!m_pControlEntity)
            return;

        if ( gpGlobals->time < m_flNextAttack )
            return;
        
        CBasePlayer* pController = (CBasePlayer*)(m_pController.GetEntity());
        if ( pController->pev->button & IN_ATTACK )
        {
            m_fireLast = gpGlobals->time - (1/m_fireRate) - 0.01;
            MAKE_VECTORS(pev->angles);
            Fire( BarrelPosition(), gpGlobals->v_forward, pController->pev );
            if ( pController  && pController->IsPlayer() )
                pController->m_iWeaponVolume = LOUD_GUN_VOLUME;
            pController->m_flNextAttack = gpGlobals->time + 9999;
            m_flNextAttack = gpGlobals->time + (1/m_fireRate);
        }
    }
    void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
    {
        if ( pev->spawnflags & SF_TANK_CANCONTROL )
        {
            // player controlled turret
            if ( !pActivator->IsPlayer() )
                return;
            else if (!m_pController && useType != USE_OFF )
                StartControl((CBasePlayer*)(pActivator));
            else
                StopControl();
        }
        else
        {
            if ( !ShouldToggle( useType, IsActive() ) )
                return;
            if ( IsActive() )
                TankDeactivate();
            else
                TankActivate();
        }
        //BaseClass.Use(*pActivator, *pCaller, useType, value);
    }
    bool InRange( float range )
    {
        if ( range < m_minRange )
            return false;
        if ( m_maxRange > 0 && range > m_maxRange )
            return false;
        return true;
    }
    virtual void Think()
    {
        pev->avelocity = g_vecZero;
        TrackTarget();
        if ( fabs(pev->avelocity.x) > 1 || fabs(pev->avelocity.y) > 1 )
            StartRotSound();
        else
            StopRotSound();
    }
    float fabs(float inNum)
    {
        return inNum < 0 ? -inNum : inNum;
    }
    CBaseEntity* FindClosestEnemy( const char* szClassname, float fRadius)
    {
        edict_t* pent = NULL;
        CBaseEntity* enemy = NULL;
        float iNearest = fRadius;
        do
        {
            pent = FIND_ENTITY_IN_SPHERE(pent, pev->origin, fRadius );

            if (!FClassnameIs(&pent->v, szClassname)) {
                continue;
            }

            CBaseEntity* ent = CBaseEntity::Instance(pent);

            if ( ent  == NULL  || !ent->IsAlive() )
                continue;

            if ( ent->entindex() == entindex() )
                continue;

            if ( ent->edict() == pev->owner )
                continue;

            int rel = IRelationship(ent);
            if ( rel == R_AL || rel == R_NO )
                continue;

            float iDist = ( ent->pev->origin - pev->origin ).Length();
            if ( iDist < m_minRange)
                continue;
            if ( iDist < iNearest )
            {
                iNearest = iDist;
                enemy = ent;
            }
        }
        while ( !FNullEnt(pent)  );
        return enemy;
    }
    void TrackTarget()
    {
        TraceResult tr;
        bool updateTime = false;
        bool lineOfSight;
        Vector angles, direction, targetPosition, barrelEnd;
        CBaseEntity* pTarget;
        // Get a position to aim for
        if (m_pController)
        {
            // Tanks attempt to mirror the player's angles
            angles = m_pController.GetEntity()->pev->v_angle;
            angles[0] = -angles[0];
            if(!IsInControllerEntity(m_pController.GetEntity()))
                StopControl();
            ControllerPostFrame();
            pev->nextthink = pev->ltime + 0.05;
        }
        else
        {
            if ( IsActive() )
                pev->nextthink = pev->ltime + 0.1;
            else
                return;

            float flSearchRange = m_maxRange > 0 ? m_maxRange : 4096;
            CBaseEntity* pEnemy = NULL;
            pEnemy = FindClosestEnemy("player",  flSearchRange);
            if ( pEnemy  == NULL  )
                pEnemy = FindClosestEnemy("monster_*",  flSearchRange);
            if ( pEnemy  == NULL  )
            {
                if ( IsActive() )
                    pev->nextthink = pev->ltime + 2;    // Wait 2 secs
                return;
            }
            pTarget = pEnemy;
            // Calculate angle needed to aim at target
            barrelEnd = BarrelPosition();
            targetPosition = pTarget->pev->origin + pTarget->pev->view_ofs;

            float range = (targetPosition - barrelEnd).Length();
            if ( !InRange( range ) )
                return;

            if ( FVisible(pTarget))
            {
                lineOfSight = true;
                if ( InRange( range ) && pTarget->IsAlive() )
                {
                    updateTime = true;
                    m_sightOrigin = UpdateTargetPosition( pTarget );
                }
            }
            // Track sight origin
            // !!! I'm not sure what i changed
            direction = m_sightOrigin - pev->origin;
            // direction = m_sightOrigin - barrelEnd;
            g_engfuncs.pfnVecToAngles(direction, angles);
            // Calculate the additional rotation to point the end of the barrel at the target (not the gun's center)
            angles = AdjustAnglesForBarrel( angles, direction.Length() );
        }
        angles.x = -angles.x;
        // Force the angles to be relative to the center position
        angles.y = m_yawCenter + UTIL_AngleDistance( angles.y, m_yawCenter );
        angles.x = m_pitchCenter + UTIL_AngleDistance( angles.x, m_pitchCenter );
        // Limit against range in y
        if ( angles.y > m_yawCenter + m_yawRange )
        {
            angles.y = m_yawCenter + m_yawRange;
            updateTime = false;    // Don't update if you saw the player, but out of range
        }
        else if ( angles.y < (m_yawCenter - m_yawRange) )
        {
             angles.y = (m_yawCenter - m_yawRange);
             updateTime = false; // Don't update if you saw the player, but out of range
        }
        if ( updateTime )
             m_lastSightTime = gpGlobals->time;
        // Move toward target at rate || less
        float distY = UTIL_AngleDistance( angles.y, pev->angles.y );
            pev->avelocity.y = distY * 10;
        if ( pev->avelocity.y > m_yawRate )
            pev->avelocity.y = m_yawRate;
        else if ( pev->avelocity.y < -m_yawRate )
            pev->avelocity.y = -m_yawRate;
        // Limit against range in x
        if ( angles.x > m_pitchCenter + m_pitchRange )
             angles.x = m_pitchCenter + m_pitchRange;
        else if ( angles.x < m_pitchCenter - m_pitchRange )
             angles.x = m_pitchCenter - m_pitchRange;
        // Move toward target at rate || less
        float distX = UTIL_AngleDistance( angles.x, pev->angles.x );
        pev->avelocity.x = distX  * 10;
        if ( pev->avelocity.x > m_pitchRate )
             pev->avelocity.x = m_pitchRate;
        else if ( pev->avelocity.x < -m_pitchRate )
             pev->avelocity.x = -m_pitchRate;


        if (!m_pController){
            if ( CanFire() && ( (fabs(distX) < m_pitchTolerance && fabs(distY) < m_yawTolerance) || (pev->spawnflags & SF_TANK_LINEOFSIGHT) != 0 ) )
            {
                bool fire = false;
                if ( pev->spawnflags & SF_TANK_LINEOFSIGHT )
                {
                    float length = direction.Length();
                    MAKE_VECTORS(pev->angles);
                    TRACE_LINE( barrelEnd, barrelEnd + gpGlobals->v_forward * length, dont_ignore_monsters, edict(), &tr );
                    if ( tr.pHit == pTarget->edict() )
                        fire = true;
                }
                else
                    fire = true;
                if ( fire )
                {
                    Fire( BarrelPosition(), gpGlobals->v_forward, pev );
                }
                else
                    m_fireLast = 0;
            }
            else
                m_fireLast = 0;
        }
    }
    // If barrel is offset, add in additional rotation
    Vector AdjustAnglesForBarrel( Vector angles, float distance )
    {
        float r2, d2;
        if ( m_barrelPos.y != 0 || m_barrelPos.z != 0 )
        {
            distance -= m_barrelPos.z;
            d2 = distance * distance;
            if ( m_barrelPos.y != 0 )
            {
                r2 = m_barrelPos.y * m_barrelPos.y;
                angles.y += (180.0 / M_PI) * atan2( m_barrelPos.y, sqrt( d2 - r2 ) );
            }
            if ( m_barrelPos.z != 0 )
            {
                r2 = m_barrelPos.z * m_barrelPos.z;
                angles.x += (180.0 / M_PI) * atan2( -m_barrelPos.z, sqrt( d2 - r2 ) );
            }
        }
        return angles;
    }
    virtual void Fire( const Vector barrelEnd, const Vector forward, entvars_t* pevAttacker )
    {
        //Dummy
        FireEffect( barrelEnd, forward, pevAttacker );
    }
    // Fire targets && spawn sprites
    virtual void FireEffect( const Vector barrelEnd, const Vector forward, entvars_t* pevAttacker )
    {
        if ( m_fireLast != 0 )
        {
            if ( m_iszSpriteSmoke )
            {
                CSprite* pSprite = CSprite::SpriteCreate( STRING(m_iszSpriteSmoke), barrelEnd, true );
                pSprite->AnimateAndDie( RANDOM_FLOAT( 15.0, 20.0 ) );
                pSprite->SetTransparency( kRenderTransAlpha, int(pev->rendercolor.x), int(pev->rendercolor.y), int(pev->rendercolor.z), 255, kRenderFxNone );
                pSprite->pev->velocity.z = RANDOM_FLOAT(40, 80);
                pSprite->SetScale( m_spriteScale );
            }
            if ( m_iszSpriteFlash )
            {
                CSprite* pSprite = CSprite::SpriteCreate( STRING(m_iszSpriteFlash), barrelEnd, true );
                pSprite->AnimateAndDie( 60 );
                pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation );
                pSprite->SetScale( m_spriteScale );
                // Hack Hack, make it stick around for at least 100 ms.
                pSprite->pev->nextthink += 0.1;
            }
            SUB_UseTargets( this, USE_TOGGLE, 0 );
        }
        m_fireLast = gpGlobals->time;
    }
    void TankTrace( const Vector vecStart, const Vector vecForward, const Vector vecSpread, TraceResult& tr )
    {
        // get circular gaussian spread
        float x, y, z;
        do
        {
            x = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
            y = RANDOM_FLOAT(-0.5,0.5) + RANDOM_FLOAT(-0.5,0.5);
            z = x*x+y*y;
        }
        while (z > 1);
      // push in engine
        MAKE_VECTORS(pev->angles);
        Vector vecDir = vecForward +
            x * vecSpread.x * gpGlobals->v_right +
            y * vecSpread.y * gpGlobals->v_up;
        Vector vecEnd = vecStart + vecDir * 4096;
        TRACE_LINE( vecStart, vecEnd, dont_ignore_monsters, edict(), &tr );
    }
    void StartRotSound()
    {
        if ( !m_iszRotateSound || (pev->spawnflags & SF_TANK_SOUNDON) != 0 )
            return;
        pev->spawnflags |= SF_TANK_SOUNDON;
        EMIT_SOUND( edict(), CHAN_STATIC, STRING(m_iszRotateSound), 0.85, ATTN_NORM);
    }
    void StopRotSound()
    {
        if ( (pev->spawnflags & SF_TANK_SOUNDON) && m_iszRotateSound )
            STOP_SOUND( edict(), CHAN_STATIC, STRING(m_iszRotateSound) );
        pev->spawnflags &= ~SF_TANK_SOUNDON;
    }
};

class CFuncTankAthena : public CFuncTankCustom {
public:
    CBeam* m_pLaser;
    float m_laserTime;

    CBaseEntity* pRocket;

    void Fire(const Vector barrelEnd, const Vector forward, entvars_t* pevAttacker)
    {
        if (m_fireLast != 0)
        {
            // FireBullets needs gpGlobals->v_up, etc.
            UTIL_MakeAimVectors(pev->angles);
            int bulletCount = int((gpGlobals->time - m_fireLast) * m_fireRate);

            if (bulletCount > 0)
            {
                if (m_pLaser)
                    UTIL_Remove(m_pLaser);

                TraceResult tr;
                TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);
                TRACE_LINE(tr.vecEndPos, gpGlobals->v_up * 8102, dont_ignore_monsters, edict(), &tr);

                pRocket = ShootMortar(ENT(pevAttacker), tr.vecEndPos, Vector(0, 0, 0));

                /**
                *m_pLaser = g_EntityFuncs.CreateBeam("sprites/laserbeam.spr", 255);
                m_pLaser.SetColor(255, 255, 0);
                m_pLaser.SetBrightness(255);

                TraceResult tr;
                TankTrace( barrelEnd, forward, gTankSpread[m_spread], tr );
                m_pLaser.SetStartPos( tr.vecEndPos );
                g_EntityFuncs.CreateExplosion(tr.vecEndPos, g_vecZero, pevAttacker.get_pContainingEntity(), 5, true);

                TRACE_LINE( tr.vecEndPos, gpGlobals->v_up * 8102, dont_ignore_monsters, edict(), tr );
                m_pLaser.SetEndPos( tr.vecEndPos );

                    m_laserTime = gpGlobals->time;
                    m_pLaser->pev->dmgtime = gpGlobals->time - 1.0;
                    m_pLaser->pev->nextthink = 0;
                **/
                CFuncTankCustom::Fire(barrelEnd, forward, pevAttacker);
            }
        }
        else
            CFuncTankCustom::Fire(barrelEnd, forward, pevAttacker);
    }
};

class CFuncTankGun : public CFuncTankCustom {
public:
    void Fire(const Vector barrelEnd, const Vector forward, entvars_t* pevAttacker)
    {
        int i;
        if (m_fireLast != 0)
        {
            // FireBullets needs gpGlobals->v_up, etc.
            UTIL_MakeAimVectors(pev->angles);

            int bulletCount = int((gpGlobals->time - m_fireLast) * m_fireRate);
            if (bulletCount > 0)
            {
                for (i = 0; i < bulletCount; i++)
                {
                    switch (m_bulletType)
                    {
                    case TANK_BULLET_9MM:
                        FireBullets(1, barrelEnd, forward, gTankSpread[m_spread], 4096, BULLET_MONSTER_9MM, 1, m_iBulletDamage, pevAttacker);
                        break;

                    case TANK_BULLET_MP5:
                        FireBullets(1, barrelEnd, forward, gTankSpread[m_spread], 4096, BULLET_MONSTER_MP5, 1, m_iBulletDamage, pevAttacker);
                        break;

                    case TANK_BULLET_12MM:
                        FireBullets(1, barrelEnd, forward, gTankSpread[m_spread], 4096, BULLET_MONSTER_12MM, 1, m_iBulletDamage, pevAttacker);
                        break;
                    case TANK_BULLET_NONE: break;
                    default: break;
                    }
                }
                CFuncTankCustom::Fire(barrelEnd, forward, pev);
            }
        }
        else
            CFuncTankCustom::Fire(barrelEnd, forward, pev);
    }
};

class CFuncTankLaser : public CFuncTankCustom
{
    CLaser* m_pLaser;
    float    m_laserTime;

    void Activate()
    {
        if (GetLaser() == NULL)
        {
            UTIL_Remove(this);
            //ALERT( at_error, "Laser tank with no env_laser!\n" );
        }
        else
            m_pLaser->TurnOff();
    }
    void KeyValue(KeyValueData* pkvd)
    {
        if (FStrEq(pkvd->szKeyName, "laserentity"))
        {
            pev->message = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else
            CFuncTankCustom::KeyValue(pkvd);
    }
    CLaser* GetLaser()
    {
        if (m_pLaser)
            return m_pLaser;

        edict_t* pentLaser = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->message));
        while (pentLaser == NULL)
        {
            // Found the landmark
            if (FIND_ENTITY_BY_CLASSNAME(pentLaser, "env_laser"))
            {
                m_pLaser = (CLaser*)CBaseEntity::Instance(pentLaser);
                break;
            }
            else
                pentLaser = FIND_ENTITY_BY_TARGETNAME(pentLaser, STRING(pev->message));
        }
        return m_pLaser;
    }

    void Think()
    {
        if (m_pLaser && (gpGlobals->time > m_laserTime))
            m_pLaser->TurnOff();
        CFuncTankCustom::Think();
    }

    void Fire(const Vector barrelEnd, const Vector forward, entvars_t* pevAttacker)
    {
        int i;
        TraceResult tr;
        if (m_fireLast != 0 && GetLaser())
        {
            // TankTrace needs gpGlobals->v_up, etc.
            UTIL_MakeAimVectors(pev->angles);
            int bulletCount = int((gpGlobals->time - m_fireLast) * m_fireRate);
            if (bulletCount > 0)
            {
                for (i = 0; i < bulletCount; i++)
                {
                    m_pLaser->pev->origin = barrelEnd;
                    TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);

                    m_laserTime = gpGlobals->time;
                    m_pLaser->TurnOn();
                    m_pLaser->pev->dmgtime = gpGlobals->time - 1.0;
                    m_pLaser->FireAtPoint(tr);
                    m_pLaser->pev->nextthink = 0;
                }
                CFuncTankCustom::Fire(barrelEnd, forward, pev);
            }
        }
        else
            CFuncTankCustom::Fire(barrelEnd, forward, pev);
    }
};

class CFuncTankMortar : public CFuncTankCustom {
public:
    void KeyValue(KeyValueData* pkvd)
    {
        if (FStrEq(pkvd->szKeyName, "iMagnitude"))
        {
            pev->impulse = atoi(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else
            CFuncTankCustom::KeyValue(pkvd);
    }

    void Fire(const Vector barrelEnd, const Vector forward, entvars_t* pevAttacker)
    {
        if (m_fireLast != 0)
        {
            int bulletCount = int(float(gpGlobals->time - m_fireLast) * m_fireRate);
            // Only create 1 explosion
            if (bulletCount > 0)
            {
                TraceResult tr;
                // TankTrace needs gpGlobals->v_up, etc.
                UTIL_MakeAimVectors(pev->angles);
                TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);
                ExplosionCreate(tr.vecEndPos, pev->angles, edict(), pev->impulse, true);
                CFuncTankCustom::Fire(barrelEnd, forward, pev);
            }
        }
        else
            CFuncTankCustom::Fire(barrelEnd, forward, pev);
    }
};

class CFuncTankRocket : public CFuncTankCustom {
public:
    void Precache()
    {
        UTIL_PrecacheOther("rpg_rocket");
        CFuncTankCustom::Precache();
    }
    void Fire(const Vector barrelEnd, const Vector forward, entvars_t* pevAttacker)
    {
        if (m_fireLast != 0)
        {
            int bulletCount = int((gpGlobals->time - m_fireLast) * m_fireRate);
            if (bulletCount > 0)
            {
                for (int i = 0; i < bulletCount; i++)
                {
                    CBaseEntity::Create("rpg_rocket", barrelEnd, pev->angles, true, edict());
                }
                CFuncTankCustom::Fire(barrelEnd, forward, pev);
            }
        }
        else
            CFuncTankCustom::Fire(barrelEnd, forward, pev);
    }
};

class CFuncTankProj : public CFuncTankCustom {
    string_t szSprPath;
    float flSprSpeed = 400;
    float flSprScale;

    void KeyValue(KeyValueData* pkvd)
    {
        if (FStrEq(pkvd->szKeyName, "sprpath"))
        {
            szSprPath = ALLOC_STRING(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "sprspeed"))
        {
            flSprSpeed = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "sprscale"))
        {
            flSprScale = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else
            CFuncTankCustom::KeyValue(pkvd);
    }
    void Precache()
    {
        PRECACHE_MODEL(STRING(szSprPath));
        CFuncTankCustom::Precache();
    }
    void Fire(const Vector barrelEnd, const Vector forward, entvars_t* pevAttacker)
    {
        if (m_fireLast != 0)
        {
            int bulletCount = int((gpGlobals->time - m_fireLast) * m_fireRate);
            if (bulletCount > 0)
            {
                Vector vecVelocity = forward * flSprSpeed;
                for (int i = 0; i < bulletCount; i++)
                {
                    CProjBullet* pProj = ShootABullet(this, barrelEnd, vecVelocity);
                    pProj->pev->scale = flSprScale;
                    SET_MODEL(pProj->edict(), STRING(szSprPath));
                }
                CFuncTankCustom::Fire(barrelEnd, forward, pev);
            }
        }
        else
            CFuncTankCustom::Fire(barrelEnd, forward, pev);
    }
};
//END
}

LINK_ENTITY_TO_CLASS(func_tankcontra, CustomTank::CFuncTankProj)