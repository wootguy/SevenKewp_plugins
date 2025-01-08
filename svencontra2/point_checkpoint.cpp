/*
* point_checkpoint
* This point entity represents a point in the world where players can trigger a checkpoint
* Dead players are revived
*/
#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "CSprite.h"
#include "hlds_hooks.h"
#include "gamerules.h"

enum PointCheckpointFlags
{
    SF_CHECKPOINT_REUSABLE = 1 << 0,    //This checkpoint is reusable
};

class CCheckpoint : public CBaseAnimating {
public:
    CSprite* m_pSprite;
    int m_iNextPlayerToRevive = 1;
    
    // How much time between being triggered && starting the revival of dead players
    float m_flDelayBeforeStart             = 3;
    
    // Time between player revive
    float m_flDelayBetweenRevive         = 1;
    
    // How much time before this checkpoint becomes active again, if SF_CHECKPOINT_REUSABLE is set
    float m_flDelayBeforeReactivation     = 60;     
    
    // When we started a respawn
    float m_flRespawnStartTime;                    
    
    // Show Xenmaker-like effect when the checkpoint is spawned?
    bool m_fSpawnEffect                    = false; 
    
    void KeyValue(KeyValueData* pkvd)
    {
        if (FStrEq(pkvd->szKeyName, "m_flDelayBeforeStart"))
        {
            m_flDelayBeforeStart = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "m_flDelayBetweenRevive"))
        {
            m_flDelayBetweenRevive = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "m_flDelayBeforeReactivation"))
        {
            m_flDelayBeforeReactivation = atof(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "minhullsize"))
        {
            UTIL_StringToVector(pev->vuser1, pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "maxhullsize"))
        {
            UTIL_StringToVector(pev->vuser2, pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else if (FStrEq(pkvd->szKeyName, "m_fSpawnEffect"))
        {
            m_fSpawnEffect = atoi(pkvd->szValue);
            pkvd->fHandled = TRUE;
        }
        else
            CBaseEntity::KeyValue(pkvd);
    }
    
    // If youre gonna use this in your script, make sure you don't try
    // to access invalid animations. -zode
    void SetAnim( int animIndex ) 
    {
        pev->sequence = animIndex;
        pev->frame = 0;
        ResetSequenceInfo();
    }
    
    void Precache()
    {
        CBaseAnimating::Precache();
        
        // Allow for custom models
        if( !pev->model )
            PRECACHE_MODEL( "models/common/lambda.mdl" );
        else
            PRECACHE_MODEL( STRING(pev->model) );
        
        PRECACHE_MODEL( "sprites/exit1.spr" );
        
        PRECACHE_SOUND( "../sound/svencontra2/1up.wav" );
        PRECACHE_SOUND( "debris/beamstart8.wav" );
        PRECACHE_SOUND( "common/null.wav" );
        
        if( !pev->message )
            pev->message = MAKE_STRING("items/gunpickup2.wav");
            
        PRECACHE_SOUND( STRING(pev->message) );
    }
    
    void Spawn()
    {
        Precache();
        
        pev->movetype         = MOVETYPE_NONE;
        pev->solid             = SOLID_TRIGGER;
        
        pev->framerate         = 1.0f;
        
        // Enabled by default
        pev->health            = 1.0f;
        
        // Not activated by default
        pev->frags            = 0.0f;
        
        // Allow for custom models
        if( !pev->model )
            SET_MODEL( edict(), "models/common/lambda.mdl");
        else
            SET_MODEL(edict(), STRING(pev->model) );
        
        UTIL_SetOrigin( pev, pev->origin );
        
        // Custom hull size
        if( pev->vuser1 != g_vecZero && pev->vuser2 != g_vecZero )
            UTIL_SetSize( pev, pev->vuser1, pev->vuser2 );
        else
            UTIL_SetSize( pev, Vector( -64, -64, -36 ), Vector( 64, 64, 36 ) );
            
        SetAnim( 0 ); // set sequence to 0 aka idle
            
        // If the map supports survival mode but survival is not active yet,
        // spawn disabled checkpoint
        //if ( pGameRules-> g_SurvivalMode.MapSupportEnabled() && !g_SurvivalMode.IsActive() )
        if (!g_pGameRules->SurvivalModeEnabled())
            SetEnabled( false );
        else
            SetEnabled( true );
        
        {
            // Fire netname entity on spawn (if specified && checkpoint is enabled)
            if ( pev->netname )
        if ( IsEnabled() )
                FireTargets( STRING(pev->netname), this, this, USE_TOGGLE, 0.0f );
                
            // Create Xenmaker-like effect
            if ( m_fSpawnEffect )
                CreateSpawnEffect();
        }
        
        SetThink( &CCheckpoint::IdleThink );
        pev->nextthink = gpGlobals->time + 0.1f;
    }
    
    void CreateSpawnEffect()
    {
        int iBeamCount = 8;
        Vector vBeamColor = Vector(217,226,146);
        int iBeamAlpha = 128;
        float flBeamRadius = 256;

        Vector vLightColor = Vector(39,209,137);
        float flLightRadius = 160;

        Vector vStartSpriteColor = Vector(65,209,61);
        float flStartSpriteScale = 1.0f;
        float flStartSpriteFramerate = 12;
        int iStartSpriteAlpha = 255;

        Vector vEndSpriteColor = Vector(159,240,214);
        float flEndSpriteScale = 1.0f;
        float flEndSpriteFramerate = 12;
        int iEndSpriteAlpha = 255;

        // create the clientside effect
        /*
        MESSAGE_BEGIN( MSG_PVS, NetworkMessages::TE_CUSTOM, pev.origin );
            WRITE_BYTE( 2 ); // TE_C_XEN_PORTAL
            WRITE_COORD_VECTOR( pev->origin );
            // for the beams
            WRITE_BYTE( iBeamCount );
            WRITE_COORD_VECTOR( vBeamColor );
            WRITE_BYTE( iBeamAlpha );
            WRITE_COORD( flBeamRadius );
            // for the dlight
            WRITE_COORD_VECTOR( vLightColor );
            WRITE_COORD( flLightRadius );
            // for the sprites
            WRITE_COORD_VECTOR( vStartSpriteColor );
            WRITE_BYTE( int( flStartSpriteScale*10 ) );
            WRITE_BYTE( int( flStartSpriteFramerate ) );
            WRITE_BYTE( iStartSpriteAlpha );
            
            WRITE_COORD_VECTOR( vEndSpriteColor );
            WRITE_BYTE( int( flEndSpriteScale*10 ) );
            WRITE_BYTE( int( flEndSpriteFramerate ) );
            WRITE_BYTE( iEndSpriteAlpha );
        MESSAGE_END();
        */
    }
    
    void Touch( CBaseEntity* pOther )
    {
        if( !IsEnabled() || IsActivated() || !pOther->IsPlayer() )
            return;
        
        // Set activated
        pev->frags = 1.0f;
        
        EMIT_SOUND( edict(), CHAN_STATIC, "../sound/svencontra2/1up.wav", 1.0f, ATTN_NONE );

        pev->rendermode        = kRenderTransTexture;
        pev->renderamt        = 255;
        
        SetThink( &CCheckpoint::FadeThink );
        pev->nextthink = gpGlobals->time + 0.1f;
        
        // Trigger targets
        SUB_UseTargets( pOther, USE_TOGGLE, 0 );
    }
    
    bool IsEnabled() const { return pev->health != 0.0f; }
    
    bool IsActivated() const { return pev->frags != 0.0f; }
    
    void SetEnabled( const bool bEnabled )
    {
        if( bEnabled == IsEnabled() )
            return;
        
        if ( bEnabled && !IsActivated() )
            pev->effects &= ~EF_NODRAW;
        else
            pev->effects |= EF_NODRAW;
        
        pev->health = bEnabled ? 1.0f : 0.0f;
    }
    
    // GeckoN: Idle Think - just to make sure the animation gets updated properly.
    // Should fix the "checkpoint jitter" issue.
    void IdleThink()
    {
        StudioFrameAdvance();
        pev->nextthink = gpGlobals->time + 0.1;
    }
    
    void FadeThink()
    {
        if ( pev->renderamt > 0 )
        {
            StudioFrameAdvance();
            
            pev->renderamt -= 30;
            if ( pev->renderamt < 0 )
                pev->renderamt = 0;
            
            pev->nextthink = gpGlobals->time + 0.1f;
        }
        else
        {
            SetThink( &CCheckpoint::RespawnStartThink );
            pev->nextthink = gpGlobals->time + m_flDelayBeforeStart;
            
            m_flRespawnStartTime = gpGlobals->time;
        
            // Make this entity invisible
            pev->effects |= EF_NODRAW;
            
            pev->renderamt = 255;
        }
    }
    
    void RespawnStartThink()
    {
        //Clean up the old sprite if needed
        if( m_pSprite  )
            UTIL_Remove( m_pSprite );
        
        m_iNextPlayerToRevive = 1;
        
        m_pSprite = CSprite::SpriteCreate( "sprites/exit1.spr", pev->origin, true );
        m_pSprite->pev->framerate = 10;
        m_pSprite->TurnOn();
        m_pSprite->pev->rendermode = kRenderTransAdd;
        m_pSprite->pev->renderamt = 128;
    
        EMIT_SOUND( edict(), CHAN_STATIC, "debris/beamstart8.wav", 1.0f, ATTN_NORM );
        
        SetThink( &CCheckpoint::RespawnThink );
        pev->nextthink = gpGlobals->time + 0.1f;
    }
    
    //Revives 1 player every m_flDelayBetweenRevive seconds, if any players need reviving.
    void RespawnThink()
    {
        CBasePlayer* pPlayer;
        
        for( ; m_iNextPlayerToRevive <= gpGlobals->maxClients; ++m_iNextPlayerToRevive )
        {
            pPlayer = UTIL_PlayerByIndex( m_iNextPlayerToRevive );
            
            //Only respawn if the player died before this checkpoint was activated
            //Prevents exploitation
            if( pPlayer  && !pPlayer->IsAlive() && pPlayer->m_fDeadTime < m_flRespawnStartTime )
            {
                //Revive player && move to this checkpoint
                //pPlayer.GetObserver().RemoveDeadBody();
                UTIL_SetOrigin(pPlayer->pev, pev->origin);
                pPlayer->Revive();
                
                //Call player equip
                //Only disable default giving if there are game_player_equip entities in give mode
                edict_t *pEquipEntity = NULL;
                while ((pEquipEntity = FIND_ENTITY_BY_CLASSNAME(pEquipEntity, "game_player_equip")))
                    DispatchTouch(pEquipEntity, pPlayer->edict());
                    
                //Congratulations, && celebrations, YOU'RE ALIVE!
                EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, STRING(pev->message), 1.0f, ATTN_NORM );
                
                ++m_iNextPlayerToRevive; //Make sure to increment this to avoid unneeded loop
                break;
            }
        }
        
        //All players have been checked, close portal after 5 seconds.
        if( m_iNextPlayerToRevive > gpGlobals->maxClients )
        {
            SetThink( &CCheckpoint::StartKillSpriteThink );
            
            pev->nextthink = gpGlobals->time + 5.0f;
        }
        //Another player could require reviving
        else
        {
            pev->nextthink = gpGlobals->time + m_flDelayBetweenRevive;
        }
    }
    
    void StartKillSpriteThink()
    {
        EMIT_SOUND( edict(), CHAN_STATIC, "common/null.wav", 1.0f, ATTN_NORM );
        
        SetThink( &CCheckpoint::KillSpriteThink );
        pev->nextthink = gpGlobals->time + 3.0f;
    }
    
    void CheckReusable()
    {
        if( pev->spawnflags & SF_CHECKPOINT_REUSABLE )
        {
            SetThink( &CCheckpoint::ReenableThink );
            pev->nextthink = gpGlobals->time + m_flDelayBeforeReactivation;
        }
        else
            SetThink( NULL );
    }
    
    void KillSpriteThink()
    {
        if( m_pSprite  )
        {
            UTIL_Remove( m_pSprite );
            m_pSprite = NULL;
        }
        
        CheckReusable();
    }
    
    void ReenableThink()
    {
        if ( IsEnabled() )
        {
            //Make visible again
            pev->effects &= ~EF_NODRAW;
        }
        
        pev->frags = 0.0f;
        
        SetThink( &CCheckpoint::RespawnThink );
        pev->nextthink = gpGlobals->time + 0.1f;
    }
};

LINK_ENTITY_TO_CLASS(point_checkpoint, CCheckpoint)