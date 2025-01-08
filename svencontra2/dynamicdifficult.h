#pragma once
#include "extdll.h"
#include "util.h"

using namespace std;

class CWeaponDMGBase {
public:
    float AR = 15;
    float SG = 18;
    float MG = 25;
    float FG = 55;
    float FGE = 100;
    float KN = 15;
    float LG = 20;
    //默认值
    float ARD = AR;
    float SGD = SG;
    float MGD = MG;
    float FGD = FG;
    float FGED = FGE;
    float KND = KN;
    float LGD = LG;

    void Tweak(float factor);

    vector<float> aryTweakFactors = {
        1.0000000f,1.0000000f,1.0000000f,0.8143534f,0.7705802f,0.7348150f,
        0.7045759f,0.6783816f,0.6552766f,0.6346085f,0.6159119f,0.5988432f,
        0.5831416f,0.5686041f,0.5550701f,0.5424098f,0.5305173f,0.5193048f,
        0.5086987f,0.4986367f,0.4890657f,0.4799401f,0.4712202f,0.4628714f,
        0.4548636f,0.4471698f,0.4397664f,0.4326323f,0.4257486f,0.4190983f,
        0.4126661f,0.4064380f
    };
};

extern CWeaponDMGBase g_WeaponDMG;

void PlayerDMGTweak();