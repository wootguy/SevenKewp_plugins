#include "extdll.h"
#include "util.h"
#include "CBaseAnimating.h"

const char* WEAPONFLAG_REGISTERNAME = "info_weaponflag";

class CWeaponFlag : public CBaseAnimating {
public:

    EHANDLE pOwner = NULL;

    void Spawn() {
        if (pev->owner == NULL)
            return;
        pOwner = EHANDLE(pev->owner);
        Precache();
        pev->movetype = MOVETYPE_NOCLIP;
        pev->solid = SOLID_NOT;
        pev->framerate = 1.0f;
        if (pev->fov <= 0)
            pev->fov = 24;
        pev->scale = 0.25;
        pev->rendermode = kRenderTransAlpha;
        pev->renderamt = 255;
        pev->rendercolor = Vector(255, 255, 255);
        SET_MODEL(edict(), STRING(pev->model));
        UTIL_SetOrigin(pev, pev->origin);
        pev->nextthink = gpGlobals->time + 0.05f;
    }
    void Think() {
        if (!pOwner) {
            SetThink(&CBaseEntity::SUB_Remove);
            pev->nextthink = gpGlobals->time;
            return;
        }
        Vector vecOrigin = pOwner.GetEntity()->pev->origin;
        vecOrigin.z += pev->fov;
        UTIL_SetOrigin(pev, vecOrigin);
        pev->nextthink = gpGlobals->time + 0.05f;
    }
    void Precache() {
        PRECACHE_MODEL(STRING(pev->model));
    }
};

LINK_ENTITY_TO_CLASS(info_weaponflag, CWeaponFlag)