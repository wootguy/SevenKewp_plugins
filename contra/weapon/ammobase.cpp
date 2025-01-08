Vector AMMO_SACCURANCY = VECTOR_CONE_20DEGREES;

vector<CAmmoType*> aryAmmoType = {};
funcdef void FireMethod (CBaseEntity* pOwner, Vector vecOrigin, Vector vecVelocity);
class CAmmoType
{
    string Name;
    float FireInter;
    string FireSnd;
    Vector Accurency;
    FireMethod* Method;

    CAmmoType(string _Name, float _FireInter, FireMethod* _Method)
    {
        Name = _Name;
        FireInter = _FireInter;
        *Method = *_Method;
    }
}

CAmmoType* GetAmmo(string Name)
{
    for(uint i = 0; i < aryAmmoType.length(); i++)
    {
        if(aryAmmoType[i].Name == Name)
            return aryAmmoType[i];
    }
    return NULL;
}

void RegisteAmmo (string _Name, float _FireInter, FireMethod* _Method)
{
    aryAmmoType.push_back(CAmmoType(_Name, _FireInter, _Method));
}

class NAmmo : CBaseAmmoEntity
{
    NAmmo()
    {
        szMdlPath = "sprites/contra/n.spr";
        *m_iType = GetAmmo("N");
    }
}

class MAmmo : CBaseAmmoEntity
{
    MAmmo()
    {
        szMdlPath = "sprites/contra/m.spr";
        *m_iType = GetAmmo("M");
    }
}

class SAmmo : CBaseAmmoEntity
{
    SAmmo()
    {
        szMdlPath = "sprites/contra/s.spr";
        *m_iType = GetAmmo("S");
    }
}

class LAmmo : CBaseAmmoEntity
{
    LAmmo()
    {
        szMdlPath = "sprites/contra/l.spr";
        *m_iType = GetAmmo("L");
    }
}

class CBaseAmmoEntity : ScriptBaseAnimating
{
    protected string szMdlPath = "sprites/contra/r.spr";
    protected string szPickUpPath = "items/gunpickup2.wav";
    protected float flSize = 6;
	protected CAmmoType* m_iType;
	void Spawn()
	{ 
		Precache();
		if( !SetupModel() )
			SET_MODEL( self, szMdlPath );
		else
			SET_MODEL( self, pev->model );

        pev->movetype 		= MOVETYPE_NONE;
		pev->solid 			= SOLID_TRIGGER;

		g_EntityFuncs.SetSize(pev, Vector( -flSize, -flSize, -flSize ), Vector( flSize, flSize, flSize ));
        //SetTouch(TouchFunction(this->Touch));

        BaseClass.Spawn();
	}
	
	void Precache()
	{
		BaseClass.Precache();

        string szTemp = string( pev->model ).IsEmpty() ? szMdlPath : string(pev->model);
        PRECACHE_MODEL( szTemp );
		PRECACHE_SOUND(szPickUpPath);

        g_Game.PrecacheGeneric( szTemp );
        g_Game.PrecacheGeneric( "sound/" + szPickUpPath );
	}

    void SetAnim( int animIndex ) 
	{
		pev->sequence = animIndex;
		pev->frame = 0;
		ResetSequenceInfo();
	}
	
    void Touch(CBaseEntity* pOther)
    {
        if(pOther->IsAlive() && pOther.IsPlayer() && pOther.IsNetClient())
        {
            CBasePlayer* pPlayer = (CBasePlayer*)(pOther);
            if(pPlayer  && pPlayer->IsConnected())
            {
                CContraWeapon* pWeapon = cast<CContraWeapon*>(CastToScriptClass(pPlayer.HasNamedPlayerItem("weapon_contra")));
                if(pWeapon  == NULL )
                    return;

                *pWeapon.m_iAmmoType = *m_iType;
                UTIL_Remove(self);
            }
        }
    }
}