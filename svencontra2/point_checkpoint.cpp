/*
* point_checkpoint
* This point entity represents a point in the world where players can trigger a checkpoint
* Dead players are revived
*/

enum PointCheckpointFlags
{
    SF_CHECKPOINT_REUSABLE         = 1 << 0,    //This checkpoint is reusable
}

class point_checkpoint : ScriptBaseAnimating
{
    private CSprite* m_pSprite;
    private int m_iNextPlayerToRevive = 1;
    
    // How much time between being triggered && starting the revival of dead players
    private float m_flDelayBeforeStart             = 3;
    
    // Time between player revive
    private float m_flDelayBetweenRevive         = 1;
    
    // How much time before this checkpoint becomes active again, if SF_CHECKPOINT_REUSABLE is set
    private float m_flDelayBeforeReactivation     = 60;     
    
    // When we started a respawn
    private float m_flRespawnStartTime;                    
    
    // Show Xenmaker-like effect when the checkpoint is spawned?
    private bool m_fSpawnEffect                    = false; 
    
    bool KeyValue( const string&szKey, const string&szValue )
    {
        if( szKey == "m_flDelayBeforeStart" )
        {
            m_flDelayBeforeStart = atof( szValue );
            return true;
        }
        else if( szKey == "m_flDelayBetweenRevive" )
        {
            m_flDelayBetweenRevive = atof( szValue );
            return true;
        }
        else if( szKey == "m_flDelayBeforeReactivation" )
        {
            m_flDelayBeforeReactivation = atof( szValue );
            return true;
        }
        else if( szKey == "minhullsize" )
        {
            g_Utility.StringToVector( pev->vuser1, szValue );
            return true;
        }
        else if( szKey == "maxhullsize" )
        {
            g_Utility.StringToVector( pev->vuser2, szValue );
            return true;
        }
        else if( szKey == "m_fSpawnEffect" )
        {
            m_fSpawnEffect = atoi( szValue ) != 0;
            return true;
        }
        else
            return BaseClass.KeyValue( szKey, szValue );
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
        BaseClass.Precache();
        
        // Allow for custom models
        if( string( pev->model ).IsEmpty() )
            PRECACHE_MODEL( "models/common/lambda.mdl" );
        else
            PRECACHE_MODEL( pev->model );
        
        PRECACHE_MODEL( "sprites/exit1.spr" );
        
        PRECACHE_SOUND( "../sound/svencontra2/1up.wav" );
        PRECACHE_SOUND( "debris/beamstart8.wav" );
        PRECACHE_SOUND( "common/null.wav" );
        
        if( string( pev->message ).IsEmpty() )
            pev->message = "items/gunpickup2.wav";
            
        PRECACHE_SOUND( pev->message );
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
        if( string( pev->model ).IsEmpty() )
            SET_MODEL( self, "models/common/lambda.mdl" );
        else
            SET_MODEL( self, pev->model );
        
        g_EntityFuncs.SetOrigin( self, pev->origin );
        
        // Custom hull size
        if( pev->vuser1 != g_vecZero && pev->vuser2 != g_vecZero )
            g_EntityFuncs.SetSize( pev, pev->vuser1, pev->vuser2 );
        else
            g_EntityFuncs.SetSize( pev, Vector( -64, -64, -36 ), Vector( 64, 64, 36 ) );
            
        SetAnim( 0 ); // set sequence to 0 aka idle
            
        // If the map supports survival mode but survival is not active yet,
        // spawn disabled checkpoint
        if ( g_SurvivalMode.MapSupportEnabled() && !g_SurvivalMode.IsActive() )
            SetEnabled( false );
        else
            SetEnabled( true );
        
        if ( IsEnabled() )
        {
            // Fire netname entity on spawn (if specified && checkpoint is enabled)
            if ( !string( pev->netname ).IsEmpty() )
                g_EntityFuncs.FireTargets( pev->netname, self, self, USE_TOGGLE );
                
            // Create Xenmaker-like effect
            if ( m_fSpawnEffect )
                CreateSpawnEffect();
        }
        
        SetThink( ThinkFunction( this->IdleThink ) );
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

        MESSAGE_BEGIN msg( MSG_PVS, NetworkMessages::TE_CUSTOM, pev.origin );
            msg.WRITE_BYTE( 2 /*TE_C_XEN_PORTAL*/ );
            msg.WriteVector( pev.origin );
            // for the beams
            msg.WRITE_BYTE( iBeamCount );
            msg.WriteVector( vBeamColor );
            msg.WRITE_BYTE( iBeamAlpha );
            msg.WRITE_COORD( flBeamRadius );
            // for the dlight
            msg.WriteVector( vLightColor );
            msg.WRITE_COORD( flLightRadius );
            // for the sprites
            msg.WriteVector( vStartSpriteColor );
            msg.WRITE_BYTE( int( flStartSpriteScale*10 ) );
            msg.WRITE_BYTE( int( flStartSpriteFramerate ) );
            msg.WRITE_BYTE( iStartSpriteAlpha );
            
            msg.WriteVector( vEndSpriteColor );
            msg.WRITE_BYTE( int( flEndSpriteScale*10 ) );
            msg.WRITE_BYTE( int( flEndSpriteFramerate ) );
            msg.WRITE_BYTE( iEndSpriteAlpha );
        msg.MESSAGE_END();
    }
    
    void Touch( CBaseEntity* pOther )
    {
        if( !IsEnabled() || IsActivated() || !pOther.IsPlayer() )
            return;
        
        // Set activated
        pev->frags = 1.0f;
        
        g_SoundSystem.EmitSound( edict(), CHAN_STATIC, "../sound/svencontra2/1up.wav", 1.0f, ATTN_NONE );

        pev->rendermode        = kRenderTransTexture;
        pev->renderamt        = 255;
        
        SetThink( ThinkFunction( this->FadeThink ) );
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
            SetThink( ThinkFunction( this->RespawnStartThink ) );
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
        
        *m_pSprite = g_EntityFuncs.CreateSprite( "sprites/exit1.spr", pev->origin, true, 10 );
        m_pSprite.TurnOn();
        m_pSprite->pev->rendermode = kRenderTransAdd;
        m_pSprite->pev->renderamt = 128;
    
        g_SoundSystem.EmitSound( edict(), CHAN_STATIC, "debris/beamstart8.wav", 1.0f, ATTN_NORM );
        
        SetThink( ThinkFunction( this->RespawnThink ) );
        pev->nextthink = gpGlobals->time + 0.1f;
    }
    
    //Revives 1 player every m_flDelayBetweenRevive seconds, if any players need reviving.
    void RespawnThink()
    {
        CBasePlayer* pPlayer;
        
        for( ; m_iNextPlayerToRevive <= gpGlobals->maxClients; ++m_iNextPlayerToRevive )
        {
            *pPlayer = g_PlayerFuncs.FindPlayerByIndex( m_iNextPlayerToRevive );
            
            //Only respawn if the player died before this checkpoint was activated
            //Prevents exploitation
            if( pPlayer  && !pPlayer->IsAlive() && pPlayer.m_fDeadTime < m_flRespawnStartTime )
            {
                //Revive player && move to this checkpoint
                pPlayer.GetObserver().RemoveDeadBody();
                pPlayer.SetOrigin( pev->origin );
                pPlayer.Revive();
                
                //Call player equip
                //Only disable default giving if there are game_player_equip entities in give mode
                CBaseEntity *pEquipEntity = NULL;
                while ( ( *pEquipEntity = g_EntityFuncs.FindEntityByClassname( pEquipEntity, "game_player_equip" ) )   )
                    pEquipEntity.Touch( pPlayer );
                    
                //Congratulations, && celebrations, YOU'RE ALIVE!
                g_SoundSystem.EmitSound( pPlayer->edict(), CHAN_ITEM, pev->message, 1.0f, ATTN_NORM );
                
                ++m_iNextPlayerToRevive; //Make sure to increment this to avoid unneeded loop
                break;
            }
        }
        
        //All players have been checked, close portal after 5 seconds.
        if( m_iNextPlayerToRevive > gpGlobals->maxClients )
        {
            SetThink( ThinkFunction( this->StartKillSpriteThink ) );
            
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
        g_SoundSystem.EmitSound( edict(), CHAN_STATIC, "common/null.wav", 1.0f, ATTN_NORM );
        
        SetThink( ThinkFunction( this->KillSpriteThink ) );
        pev->nextthink = gpGlobals->time + 3.0f;
    }
    
    void CheckReusable()
    {
        if( pev->SpawnFlagBitSet( SF_CHECKPOINT_REUSABLE ) )
        {
            SetThink( ThinkFunction( this->ReenableThink ) );
            pev->nextthink = gpGlobals->time + m_flDelayBeforeReactivation;
        }
        else
            SetThink( null );
    }
    
    void KillSpriteThink()
    {
        if( m_pSprite  )
        {
            UTIL_Remove( m_pSprite );
            *m_pSprite = NULL;
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
        
        SetThink( ThinkFunction( this->RespawnThink ) );
        pev->nextthink = gpGlobals->time + 0.1f;
    }
}

void RegisterPointCheckPointEntity()
{
    g_CustomEntityFuncs.RegisterCustomEntity( "point_checkpoint", "point_checkpoint" );
}